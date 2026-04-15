# hyper-reV Documentation

## Contents
1. [Introduction](#1-introduction)
2. [Prior to boot](#2-prior-to-boot)
3. [Boot process](#3-boot-process)
    - 3.1. [Restoration of bootmgfw.efi](#31-restoration-of-bootmgfwefi)
    - 3.2. [Heap](#32-heap)
    - 3.3. [Starting bootmgfw.efi](#33-starting-bootmgfwefi)
    - 3.4. [winload.efi](#34-winloadefi)
    - 3.5. [hvloader.dll & Hyper-V launch hook](#35-hvloaderdll--hyper-v-launch-hook)
        - 3.5.1. [Loading hyperv-attachment](#351-loading-hyperv-attachment)
4. [hyperv-attachment / post Hyper-V launch info](#4-hyperv-attachment--post-hyper-v-launch-info)
    - 4.1. [How does it support different architectures?](#41-how-does-it-support-different-architectures)
    - 4.2. [Entry point (pre Hyper-V launch)](#42-entry-point-pre-hyper-v-launch)
    - 4.3. [First VM exit](#43-first-vm-exit)
    - 4.4. [APIC](#44-apic)
    - 4.5. [SLAT](#45-slat)
        - 4.5.1. [Code hooks on Intel](#451-code-hooks-on-intel)
        - 4.5.2. [Code hooks on AMD](#452-code-hooks-on-amd)
        - 4.5.3. [Common Code Hook Features](#453-code-hook-features-common-to-both-amd-and-intel)
5. [Avoiding detection](#5-avoiding-detection)
6. [Usermode app information](#6-usermode-app-information)
    - 6.1. [Command usages and descriptions](#61-command-usages-and-descriptions-list)
    - 6.2. [Kernel hooks](#62-kernel-hooks)
    - 6.3. [Command aliases](#63-command-aliases)
    - 6.4. [Flushing logs](#64-flushing-logs)
7. [How to compile / use](#7-how-to-compile--use)
8. [Source code](#8-source-code)
9. [Tested Windows versions](#9-tested-windows-versions)
10. [Credits](#10-credits)

---

## 1. Introduction
**hyper-reV** is a [memory introspection](https://hvmi.readthedocs.io/en/latest/chapters/1-overview.html) and reverse engineering [hypervisor](https://www.redhat.com/en/topics/virtualization/what-is-a-hypervisor) powered by leveraging [Hyper-V](https://en.wikipedia.org/wiki/Hyper-V). It includes a usermode component that serves as a [kernel debugger](https://en.wikipedia.org/wiki/Kernel_debugger). 

The project provides the following abilities for the guest (the Windows operating system virtualized by Hyper-V):
- Read and write guest [virtual memory](https://wiki.osdev.org/Memory_management#Virtual_Address_Space) and [physical memory](https://wiki.osdev.org/Memory_management#Physical_Address_Space).
- Translate guest virtual addresses (GVA) to guest physical addresses (GPA).
- **[SLAT](https://en.wikipedia.org/wiki/Second_Level_Address_Translation) code hooks** (EPT/NPT hooks).
- Hiding entire pages of physical memory from the guest.

Leveraging Hyper-V allows the system to work under systems protected by **[HVCI](https://learn.microsoft.com/en-us/windows/security/hardware-security/enable-virtualization-based-protection-of-code-integrity?tabs=security)** (Hypervisor-Protected Code Integrity).

## 2. Prior to boot
The `uefi-boot` module is hyper-reV's UEFI driver.
1. A backup of `bootmgfw.efi` is created.
2. The original `bootmgfw.efi` is replaced with the `uefi-boot` module.
3. The `hyperv-attachment` (the module inserted into Hyper-V) is saved in the same directory as `bootmgfw.efi`.

## 3. Boot process

### 3.1. Restoration of bootmgfw.efi
Once started, the `uefi-boot` module restores the original `bootmgfw.efi` file and its metadata (timestamps) to hide any evidence of tampering.

### 3.2. Heap
A [heap](https://wiki.osdev.org/Heap) is [allocated](https://wiki.osdev.org/Memory_Allocation) using [UEFI boot_services!AllocatePages](https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-allocatepages) (ensuring 4kB alignment, rather than using [boot_services!AllocatePool](https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#id16) which does not guarantee that alignment). This heap is used for:
- The **hyperv-attachment** runtime image buffer.
- **[PML4 and PDPT](https://wiki.osdev.org/Page_Tables#48-bit_virtual_address_space)** for the [identity map](https://wiki.osdev.org/Identity_Paging) which is later used.
- The **hyperv-attachment's internal heap**.
- The deep copy of Hyper-V's SLAT **[CR3](https://wiki.osdev.org/CPU_Registers_x86#CR3)**.

All allocated memory is later hidden from the guest using SLAT dummy mappings.

### 3.3. Starting bootmgfw.efi
The original `bootmgfw.efi` is loaded via **[UEFI boot_services!LoadImage](https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#id37)**. A hook is applied to `bootmgfw.efi!ImgpLoadPEImage` before starting it using **[UEFI boot_services!StartImage](https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#id38)**.

### 3.4. winload.efi
When `winload.efi` is loaded by `bootmgfw.efi`, it is intercepted. The previous hooks are removed, and a new hook is applied to `winload.efi!ImgpLoadPEImage` to intercept `hvloader.dll`.

### 3.5. hvloader.dll & Hyper-V launch hook
Once `hvloader.dll` is intercepted, a hook is placed deep within the Hyper-V launch routine (`hvloader.dll!HvlLaunchHypervisor`).

#### Hooked Hyper-V Launch Routine:
```cpp
void __fastcall hv_launch(
    std::uint64_t hyperv_cr3, 
    std::uint8_t* hyperv_entry_point, 
    std::uint8_t* entry_point_gadget, 
    std::uint64_t guest_kernel_cr3
) {
    __writecr3(guest_kernel_cr3);
    __asm { jmp entry_point_gadget }
}
```

**Parameters:**
- `rcx`: CR3 from which Hyper-V copies PML4 entries.
- `rdx`: Virtual address of the relocated Hyper-V entry point.
- `r8`: Address of the gadget jumping to the entry point.
- `r9`: Guest kernel CR3 (e.g., `0x1AE000` on Windows 11 24H2).

#### 3.5.1. Loading hyperv-attachment
A PML4e containing an identity map of host physical memory is inserted into the CR3 in `rcx`. The `hyperv-attachment` image is [relocated](https://sabotagesec.com/pe-relocation-table/) and mapped into Hyper-V's address space. 

As @Iraq1337 described in **[his post](https://www.unknowncheats.me/forum/4323297-post8.html)**, there are a few copies of Hyper-V in physical memory which are not SLAT protected. A hook is placed on Hyper-V's VM exit handler, detouring to the `hyperv-attachment` VM exit handler using a **[far/long jump instruction](https://www.felixcloutier.com/x86/jmp)**.

## 4. hyperv-attachment / post Hyper-V launch info

### 4.1. How does it support different architectures?
The codebase uses abstraction and `#ifdefs` to support both **Intel** and **AMD** in the same codebase. Compilation details are in Section 7.

### 4.2. Entry point (pre Hyper-V launch)
The entry point performs:
- Heap manager setup.
- Initial SLAT context allocations.
- Processor state logs context setup.
- Intake of metadata from the `uefi-boot` module.

### 4.3. First VM exit
During the first VM exit:
1. Sets up a **[non maskable interrupt](https://wiki.osdev.org/Non_Maskable_Interrupt)** (NMI) handler in Hyper-V's global **[interrupt descriptor table](https://wiki.osdev.org/Interrupt_Descriptor_Table)** (IDT).
2. Initializes the **[APIC](https://wiki.osdev.org/APIC)** for cross-processor NMI synchronization.
3. Wipes the `uefi-boot` image from memory to prevent guest detection.
4. Hides the initial UEFI heap via SLAT after initialization completes. This is achieved by setting the **[page table entries](https://wiki.osdev.org/Page_Tables)** to point to a dummy page.

### 4.4. APIC
The hypervisor fetches APIC information via **[MSR](https://wiki.osdev.org/Model_Specific_Registers)** (Model Specific Register) `0x1B`. It enables the highest available version (xAPIC or x2APIC). For xAPIC, the ICR can be accessed through its **[Local APIC](https://wiki.osdev.org/APIC#Local_APIC_configuration)** physical address. Commands are sent via the **ICR** to fire NMIs for SLAT cache synchronization. The APIC library developed for this project **[can be found here](https://github.com/noahware/APIC)**.

### 4.5. SLAT
**SLAT (Second Level Address Translation)** maps guest physical memory to host physical memory.

#### 4.5.1. Code hooks on Intel
Intel uses **EPT** (Extended Page Tables).
1. The target page permissions are set to `--x` (Execution only).
2. The PFN is pointed to a **shadow page**.
3. Read/Write attempts trigger an **EPT Violation**.
4. The handler restores the PFN and sets permissions to `rw-` (Read/Write only) for that access.
5. Execution attempts revert it back to `--x`.

#### 4.5.2. Code hooks on AMD
AMD uses **NPT** (Nested Page Tables). Since AMD lacks "Execute-only" bits:
1. An **identity map** (Hook Nested CR3) is created as a copy of the Hyper-V Nested CR3.
2. In the original CR3, the target page is marked **non-executable**.
3. In the "Hook" CR3, the page is **executable** and points to the **shadow page**.
4. Execution triggers a **Nested Page Fault**, switching the processor to the "Hook" CR3.
5. Reaching a non-hooked page triggers another fault, reverting to the original CR3.

#### 4.5.3. Common Code Hook Features
##### 4.5.3.1. Page split/merge
If a large page (2MB/1GB) has to be split (**[PDe or PDPTe](https://wiki.osdev.org/Page_Tables#48-bit_virtual_address_space)**) to get a **[PTe](https://wiki.osdev.org/Page_Tables#48-bit_virtual_address_space)** for 4kB hooking, entries are merged back when hooks are removed to save memory and improve performance.

##### 4.5.3.2. Synchronization using NMIs
APIC-based NMIs notify all logical processors to invalidate SLAT caches.
- **Host State:** **[All general purpose and XMM registers](https://wiki.osdev.org/CPU_Registers_x86-64)** are saved on the stack. The NMI processor function clears the SLAT cache if signaled. **[Hyper-V uses NMIs for inter processor communication](https://forum.osdev.org/viewtopic.php?p=345109#p345109)**, so a bitmap is used for synchronization.
- **Guest State:** Triggers a 'physical NMI' VM exit; the cache is flushed within the exit handler.

## 4.6. Returning execution to Hyper-V
Unhandled VM exits are transparently passed to Hyper-V's original handler.

## 4.7. Hypercalls
The hypervisor exposes **[hypercalls](https://wiki.xenproject.org/wiki/Hypercall)** via the **[CPUID instruction](https://wiki.osdev.org/CPUID)**. Unhandled exits are passed to Hyper-V.
- **`guest_physical_memory_operation`**: Read/Write GPA.
- **`guest_virtual_memory_operation`**: Read/Write GVA.
- **`translate_guest_virtual_address`**: GVA -> GPA translation.
- **`read_guest_cr3`**: Get current guest CR3.
- **`add_slat_code_hook`**: Add a SLAT hook.
- **`remove_slat_code_hook`**: Remove a SLAT hook.
- **`hide_guest_physical_page`**: Hide GPA from guest.
- **`log_current_state`**: Capture processor state.
- **`flush_logs`**: Flush logs to guest buffer.
- **`get_heap_free_page_count`**: Check hypervisor heap status.

## 5. Avoiding detection
- **Independent Allocation**: `hyperv-attachment` is allocated separately from the Hyper-V image to avoid shifting detection.
- **Late Hooking**: Hooks are applied only to the final SLAT-protected Hyper-V image.
- **Metadata Restoration**: `bootmgfw.efi` timestamps and contents are restored early.

## 6. Usermode app information
The usermode app serves as a CLI-based kernel debugger using **CLI11**.

### 6.1. Command usages and descriptions
| Command | Usage | Description |
| :--- | :--- | :--- |
| `rgpm` | `rgpm <addr> <size>` | Read guest physical memory |
| `wgpm` | `wgpm <addr> <val> <size>` | Write guest physical memory |
| `cgpm` | `cgpm <dst> <src> <size>` | Copy guest physical memory |
| `gvat` | `gvat <gva> <cr3>` | Translate GVA to GPA |
| `rgvm` | `rgvm <gva> <cr3> <size>` | Read guest virtual memory |
| `akh` | `akh <gva> [opts]` | Add kernel hook (supports `--monitor`, `--asmbytes`) |
| `rkh` | `rkh <gva>` | Remove kernel hook |
| `hgpp` | `hgpp <gpa>` | Hide physical page |
| `fl` | `fl` | Flush trap frame logs |
| `lkm` | `lkm` | List loaded kernel modules |
| `kme` | `kme <module>` | List exports of a module |
| `dkm` | `dkm <module> <path>` | Dump kernel module to disk |

### 6.2. Kernel hooks
The app uses a "detour holder" page within `ntoskrnl.exe` or an unused driver. This avoids new executable allocations. **[Inline hooks](https://www.codereversing.com/archives/592)** are applied on shadow pages, resolving **[rip relative operands](https://wiki.osdev.org/X86-64_Instruction_Encoding#RIP/EIP-relative_addressing)** (jumps, calls, memory accesses) to absolute values to maintain stability.

### 6.4. Flushing logs
The `--monitor` flag captures state snapshots, including **[rip](https://wiki.osdev.org/CPU_Registers_x86#Pointer_Registers)**, CR3, and **[the stack](https://wiki.osdev.org/Stack)**:
```text
0. rip=0xFFFFF8049B0BA5E6 rax=0x3 rcx=0x538
rdx=0xFFFFAF869BF2F260 rbx=0x0 rsp=0xFFFFAF869BF2F208 rbp=0xFFFFAF869BF2F360
rsi=0x80 rdi=0x538 r8=0xFFFFAF869BF2F5A0 r9=0xFFFFE486C68FA800
r10=0xFFFFF8049B0BA5E0 r11=0xFFFFAF869BF2F458 r12=0xFFFFF8042D693000 ...
CR3=0x1B473C000
```

## 7. How to compile / use

### 7.1. 'uefi-boot' compilation
1. Install **NASM** and set `NASM_PREFIX`. Referenced from **[VisualUefi's Installation](https://github.com/ionescu007/VisualUefi/#Installation)**.
2. Clone submodules:
   ```bash
   git clone --branch OpenSSL_1_1_0-stable https://github.com/openssl/openssl.git uefi-boot/ext/openssl
   git clone https://github.com/ionescu007/edk2.git uefi-boot/ext/edk2/src
   ```
3. Build EDK2 libraries via `uefi-boot\ext\edk2\build\EDK-II.sln`.

### 7.2. Architecture-specific compilation
In `arch_config.h` (`hyperv-attachment` src):
- **Intel**: `#define _INTELMACHINE`
- **AMD**: Comment out `#define _INTELMACHINE`

### 7.3. Load script
Run `load-hyper-reV.bat` as Administrator. It places files into the EFI partition for the next boot.

### 7.4. Usage with Secure Boot
To load the project with **[secure boot](https://access.redhat.com/articles/5254641)** enabled, a vulnerable **[bootloader](https://wiki.osdev.org/Bootloader)** could be exploited, as described in **[this post](https://habr.com/articles/446238/)**.

### 7.5. Usage with TPM
If evading an advanced security tool, it is not recommend to run the project with **[TPM](https://en.wikipedia.org/wiki/Trusted_Platform_Module)** enabled if the security tool performs **[boot attestation](https://learn.microsoft.com/en-us/azure/attestation/tpm-attestation-concepts)**. This is because it can see that the uefi-boot binary was loaded (through info stored in the TPM PCRs or through **[measured boot](https://learn.microsoft.com/en-us/azure/security/fundamentals/measured-boot-host-attestation)** logs). This information is from **[Zepta's post](https://www.unknowncheats.me/forum/anti-cheat-bypass/623028-measuredboot-tpm.html)**.

## 8. Source code
- GitHub: [https://github.com/noahware/hyper-reV](https://github.com/noahware/hyper-reV)

## 9. Tested Windows versions
Supported (Intel/AMD):
- Windows 10: 21H2, 22H2
- Windows 11: 22H2, 23H2, 24H2

## 10. Credits
- **[John / @Iraq1337](https://github.com/vmp38)**: AMD theory, Nested CR3 mapping, VMCB access.
- **[@papstuc](https://github.com/papstuc)**: EPT/NPT synchronization, AMD page splitting, **[his Windows file format parsing library](https://github.com/papstuc/portable_executable)**.
- **mylostchristmas**: Research on Windows 11 24H2 SLAT protection changes.
