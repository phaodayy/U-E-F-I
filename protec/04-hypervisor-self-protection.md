# Hypervisor Self-Protection

This document describes a defensive, production-oriented design for protecting
hypervisor-owned memory from guest access. It is intentionally a reference
architecture and pseudocode only. It is not wired into any runtime component.

## Goals

- Keep hypervisor code, data, heap, logs, and per-vCPU stacks outside guest
  visibility.
- Treat unclassified physical frames as denied by default.
- Make all host-owned PFN ownership changes explicit, audited, and immutable
  after initialization where possible.
- Use hardware-enforced isolation through SLAT: Intel EPT or AMD NPT.
- Fail closed when guest access targets host-owned frames.
- Record security-relevant violations for lab analysis.

## Non-Goals

- No guest or Ring-3 API should be able to mark host PFNs as guest-accessible.
- No arbitrary guest physical memory read/write interface should bypass this
  policy.
- No hidden recovery path should silently grant wider memory permissions after a
  violation.
- This design does not provide implementation hooks for evasion, stealth, or
  runtime patching.

## Ownership Model

Every physical frame must have a single owner classification.

```text
Unknown      : Not classified yet. Deny all guest access.
HostCode     : Hypervisor executable text. Deny guest read/write/execute.
HostData     : Hypervisor globals, heap, metadata, logs. Deny guest access.
HostStack    : Per-vCPU hypervisor stacks. Deny guest access.
GuestRAM     : Normal guest-owned RAM. Map with guest permissions.
SharedBounce : Explicit short-lived copy buffer. Least privilege only.
MMIO         : Device region. Allowlist only.
Reserved     : Firmware, ACPI, SMM, unavailable, or unsafe. Deny by default.
Guard        : Guard/canary page. Deny all access.
```

Access requests should be normalized before policy evaluation.

```text
Read
Write
Execute
DMARead
DMAWrite
HypercallCopyIn
HypercallCopyOut
DiagnosticRead
```

## Registry Data Model

The registry is the single source of truth for PFN ownership. SLAT builders,
hypercall validators, DMA policy, and violation handlers should consume this
registry rather than inventing separate allow/deny checks.

```pseudo
enum PfnOwner {
    Unknown,
    HostCode,
    HostData,
    HostStack,
    GuestRAM,
    SharedBounce,
    MMIO,
    Reserved,
    Guard
}

struct PfnRecord {
    owner: PfnOwner
    allowed_access: AccessMask
    generation: uint64
    immutable: bool
    tag: string
}

global PFN_REGISTRY: array<PfnRecord>
global ZERO_PAGE_PFN: pfn
```

## Initialization

Initialization should classify all memory before the guest is allowed to run.
Unknown entries remain denied and should be treated as configuration errors in
strict builds.

```pseudo
function initialize_pfn_registry(memory_map, hypervisor_layout):
    for each pfn in physical_address_space:
        PFN_REGISTRY[pfn] = {
            owner: Unknown,
            allowed_access: NONE,
            generation: 0,
            immutable: false,
            tag: "unclassified"
        }

    mark_range(hypervisor_layout.code, HostCode, NONE, immutable=true)
    mark_range(hypervisor_layout.data, HostData, NONE, immutable=true)
    mark_range(hypervisor_layout.heap, HostData, NONE, immutable=true)
    mark_range(hypervisor_layout.logs, HostData, NONE, immutable=true)
    mark_range(hypervisor_layout.per_vcpu_stacks, HostStack, NONE, immutable=true)

    for each range in memory_map.guest_ram:
        mark_range(range, GuestRAM, READ | WRITE | EXECUTE, immutable=false)

    for each range in memory_map.mmio:
        mark_range(range, MMIO, NONE, immutable=true)

    for each range in memory_map.reserved:
        mark_range(range, Reserved, NONE, immutable=true)

    create_guard_pages_around_host_ranges()
```

Range marking must reject ownership overlaps.

```pseudo
function mark_range(range, owner, allowed_access, immutable):
    for pfn in range.to_pfns():
        current = PFN_REGISTRY[pfn]

        if current.immutable:
            fail_closed("attempted immutable PFN reclassification")

        if current.owner != Unknown and current.owner != owner:
            fail_closed("conflicting PFN ownership")

        PFN_REGISTRY[pfn] = {
            owner: owner,
            allowed_access: allowed_access,
            generation: current.generation + 1,
            immutable: immutable,
            tag: range.name
        }
```

## Policy Evaluation

Policy evaluation should be centralized and side-effect free. The caller decides
whether denial means VM termination, injected guest fault, or a read-only zero
page response.

```pseudo
function evaluate_guest_access(pfn, access_type, source_context):
    record = PFN_REGISTRY[pfn]

    if record.owner == Unknown:
        return DENY_AND_AUDIT

    if record.owner in [HostCode, HostData, HostStack, Guard, Reserved]:
        return DENY_AND_AUDIT

    if record.owner == MMIO:
        if is_mmio_access_allowlisted(pfn, access_type, source_context):
            return ALLOW
        return DENY_AND_AUDIT

    if record.owner == SharedBounce:
        if source_context.is_authorized_host_path and access_type in record.allowed_access:
            return ALLOW
        return DENY_AND_AUDIT

    if record.owner == GuestRAM:
        if access_type in record.allowed_access:
            return ALLOW
        return DENY_AND_AUDIT

    return DENY_AND_AUDIT
```

## SLAT Mapping Builder

The SLAT mapping should be generated from the registry. Host-owned PFNs should
not appear as guest-accessible mappings.

```pseudo
function build_slat_policy_for_guest(vm):
    for each pfn in physical_address_space:
        record = PFN_REGISTRY[pfn]

        switch record.owner:
            case GuestRAM:
                slat.map(gpa=pfn_to_gpa(pfn),
                         hpa=pfn_to_hpa(pfn),
                         permissions=record.allowed_access)

            case SharedBounce:
                slat.map(gpa=pfn_to_gpa(pfn),
                         hpa=pfn_to_hpa(pfn),
                         permissions=least_privilege(record.allowed_access))

            case MMIO:
                if mmio_region_is_allowlisted(vm, pfn):
                    slat.map_mmio(pfn, permissions=mmio_permissions(vm, pfn))
                else:
                    slat.unmap(pfn)

            default:
                slat.unmap(pfn)

    slat.flush_all_contexts()
```

Recommended default outcomes:

```text
HostCode/HostData/HostStack : unmapped, no read, no write, no execute
Unknown                     : unmapped
Reserved                    : unmapped
Guard                       : unmapped
GuestRAM                    : mapped using guest policy
SharedBounce                : mapped with minimal temporary permissions
MMIO                        : mapped only when allowlisted
```

## Fail-Closed Violation Handling

The violation handler should not grant access while processing a fault against a
host-owned PFN. It should log first, then apply a deterministic policy action.

```pseudo
function handle_slat_violation(fault_gpa, access_type, guest_context):
    pfn = gpa_to_pfn(fault_gpa)
    record = PFN_REGISTRY[pfn]

    audit_log({
        event: "SLAT_ACCESS_VIOLATION",
        vm_id: guest_context.vm_id,
        vcpu_id: guest_context.vcpu_id,
        guest_cr3: guest_context.cr3,
        guest_rip: guest_context.rip,
        gpa: fault_gpa,
        pfn: pfn,
        owner: record.owner,
        access: access_type,
        action: configured_violation_action(record),
        policy_generation: record.generation
    })

    if record.owner in [HostCode, HostData, HostStack, Guard]:
        increment_security_counter(guest_context.vm_id)

        if policy.host_owned_violation_action == TERMINATE_VM:
            terminate_guest("host-owned PFN access")
            return HANDLED

        if policy.host_owned_violation_action == INJECT_FAULT:
            inject_guest_memory_fault()
            return HANDLED

        if policy.host_owned_violation_action == ZERO_PAGE_FOR_READ_ONLY:
            if access_type == Read:
                map_zero_page_readonly_for_faulting_gpa(fault_gpa)
                return HANDLED

        inject_guest_memory_fault()
        return HANDLED

    if record.owner == Unknown:
        inject_guest_memory_fault()
        return HANDLED

    return NOT_HANDLED
```

For production defense, write and execute attempts against host-owned PFNs
should not be redirected silently. Terminating or quarantining the guest gives
clearer security semantics.

## Hypercall Validation

Any memory-related hypercall should validate every touched page before mapping,
copying, translating, or returning data.

```pseudo
function validate_hypercall_memory_request(request, caller_context):
    if not caller_context.is_authenticated_management_plane:
        return DENY

    if range_overflows(request.address, request.size):
        return DENY

    if request.size > MAX_MANAGEMENT_COPY_SIZE:
        return DENY

    for each page in pages_covered_by(request.address, request.size):
        pfn = gpa_to_pfn(page.gpa)
        decision = evaluate_guest_access(pfn, request.access_type, caller_context)

        if decision != ALLOW:
            audit_log_hypercall_denial(request, caller_context, pfn)
            return DENY

    return ALLOW
```

Guest or Ring-3 paths should not be able to:

- Read, write, or execute HostCode, HostData, HostStack, Guard, or Reserved PFNs.
- Request host physical address translation.
- Map a host-owned PFN into guest-visible memory.
- Disable audit logging.
- Change registry ownership or policy generation.
- Widen SLAT permissions directly.

## Mandatory Audit Logging

Audit buffers must themselves be host-owned and denied to the guest.

```pseudo
struct AuditEvent {
    timestamp_tsc: uint64
    event_type: AuditEventType
    vm_id: uint32
    vcpu_id: uint32
    guest_cr3: uint64
    guest_rip: uint64
    gpa: uint64
    pfn: uint64
    owner: PfnOwner
    access_type: AccessType
    action_taken: PolicyAction
    policy_generation: uint64
}
```

Pipeline:

```text
Per-CPU ring buffer
        |
        v
Rate limiter and duplicate suppressor
        |
        v
Host-only drain path
        |
        v
Signed diagnostic export
```

Requirements:

- Logs must not be guest-accessible.
- Dropped event counts must be tracked.
- Repeated faults should be rate-limited by VM, vCPU, PFN, and event type.
- Audit disable must require a host management-plane action.
- Export should use a host management channel, not a general guest hypercall.

## DMA and Hardware Tampering Controls

SLAT protects CPU memory translation. DMA requires IOMMU policy.

- Enable Intel VT-d or AMD-Vi.
- Use DMA remapping with deny-by-default domains.
- Keep host-owned PFNs outside all device-accessible domains.
- Allowlist device MMIO and DMA ranges explicitly.
- Enable interrupt remapping where supported.
- Lock IOMMU configuration after initialization.
- Prefer measured boot or DRTM for lab integrity measurements.
- Record IOMMU faults in the same audit pipeline.

## Production Hardening Checklist

Memory isolation:

- [ ] PFN registry initializes every PFN as Unknown and denied.
- [ ] Hypervisor code, data, heap, logs, and stacks are immutable host-owned PFNs.
- [ ] Guard pages surround hypervisor heap and stacks.
- [ ] SLAT is generated from the registry, not scattered permission edits.
- [ ] Host-owned PFNs are never guest-accessible.
- [ ] Shared buffers have owner, lifetime, size, and least-privilege permissions.
- [ ] SLAT/TLB caches are flushed after policy changes.
- [ ] Policy generation is tracked and logged.

Hypercall surface:

- [ ] Arbitrary guest physical read/write hypercalls are removed or disabled.
- [ ] Memory hypercalls require authenticated host management context.
- [ ] Address plus size overflow is rejected.
- [ ] Cross-page requests validate every page.
- [ ] Host-owned PFNs are denied before any map/copy operation.
- [ ] Hypercalls never return host physical or virtual addresses.
- [ ] Audit cannot be disabled by guest-controlled input.
- [ ] Hypercalls cannot directly widen SLAT permissions.

Violation handling:

- [ ] Host-owned PFN violations fail closed.
- [ ] Write and execute attempts are not silently redirected.
- [ ] Repeated violations can terminate or quarantine the VM.
- [ ] Violation logs include VM, vCPU, RIP, CR3, GPA, PFN, owner, and action.
- [ ] Unknown PFN faults are denied and audited.

DMA and devices:

- [ ] IOMMU is enabled and locked.
- [ ] Device DMA domains are deny-by-default.
- [ ] No device domain contains host-owned PFNs.
- [ ] MMIO is allowlisted per VM/device.
- [ ] IOMMU faults are audited.

Runtime integrity:

- [ ] Hypervisor code pages are read-only/executable and never writable.
- [ ] Hypervisor data and stacks are non-executable.
- [ ] Per-vCPU stacks have guard pages.
- [ ] Debug-only interfaces are compiled out in production profile.
- [ ] Ownership invariants fail closed.
- [ ] Crash dumps avoid exposing host-owned secrets to the guest.

Testing:

- [ ] Unit tests cover PFN classification and overlap rejection.
- [ ] Range validator tests cover overflow and wraparound.
- [ ] Cross-page hypercall tests validate every touched PFN.
- [ ] SLAT policy tests confirm host-owned PFNs are unmapped.
- [ ] Violation tests cover read, write, and execute faults.
- [ ] Audit tests verify event persistence and dropped-event counters.
- [ ] IOMMU lab tests deny DMA into host-owned PFNs.

## Review Guidance

A production review should reject any change that:

- Adds a new memory mapping path that bypasses the registry.
- Gives guest-originated code a way to classify or reclassify PFNs.
- Maps host-owned memory into guest-visible GPA space.
- Copies from or to guest memory before policy validation.
- Silently downgrades a host-owned violation into a successful guest access.
- Stores audit logs in memory that the guest can read or write.
