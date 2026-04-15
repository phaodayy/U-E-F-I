---
name: kernel-debugging
description: How to use the usermode CLI app for memory introspection and kernel hooking.
---

# Kernel Debugging Skill

This skill explains how to use the hyper-reV usermode application to interact with the guest Windows OS.

## 1. Starting the App
Run the compiled `usermode.exe`. It uses a CLI interface for commands.

## 2. Memory Operations
- **Read Physical**: `rgpm <address> <size>`
- **Read Virtual**: `rgvm <address> <cr3> <size>`
- **Translate**: `gvat <virtual_address> <cr3>`

## 3. Kernel Hooking
Use the `akh` (Add Kernel Hook) command to place a stealthy SLAT-based hook.
- **Example**: `akh ntoskrnl.exe!PsLookupProcessByProcessId --monitor --asmbytes 0x90 0x90`
- **RIP-relative support**: The hypervisor automatically resolves RIP-relative operands, making it safe to hook locations with relative jumps or calls.

## 4. Stealth & Logs
- **Flush Logs**: Use `fl` to view processor states captured by monitored hooks.
- **Hide Page**: Use `hgpp <physical_address>` to completely hide a physical page from the guest OS.
