---
name: hypercall-development
description: How to extend or modify the hypercall interface between guest and host.
---

# Hypercall Development Skill

This skill provides instructions on how to add and manage hypercalls in hyper-reV.

## 1. Mechanism
hyper-reV uses the `CPUID` instruction as a hypercall trigger. 
- A VM exit occurs when `CPUID` is executed in the guest.
- The hypervisor checks specific registers for "magic values" to identify a hyper-reV hypercall.

## 2. Adding a New Hypercall
1. **Hypervisor Side**:
   - Locate the `CPUID` handler in the `hyperv-attachment` source code.
   - Add a new case to the dispatcher based on the register values (e.g., `rax`, `rcx`).
   - Implement the logic (e.g., memory access, state logging).
2. **Guest Side**:
   - Update the usermode application to include a wrapper for the new call.
   - Pass the required parameters via registers.

## 3. Existing List
Refer to Section 4.7.1 in `DOCUMENTATION.md` for the current list of supported hypercalls like `add_slat_code_hook` and `translate_guest_virtual_address`.
