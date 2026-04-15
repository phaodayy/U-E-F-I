---
name: compile-hyper-rev
description: Instructions for compiling the UEFI boot module and Hyper-V attachment.
---

# Compile hyper-reV Skill

This skill provides the steps and requirements to compile the two main components of hyper-reV.

## 1. Prerequisites
- **NASM**: Must be installed and `NASM_PREFIX` environment variable set.
- **Visual Studio**: Required for building the `.sln` files.
- **VisualUefi / EDK2**: Submodules must be initialized.

## 2. UEFI Boot Module (`uefi-boot`)
1. Ensure submodules are cloned in `uefi-boot/ext/`:
   - `openssl` (branch `OpenSSL_1_1_0-stable`)
   - `edk2` (src folder)
2. Open `uefi-boot/ext/edk2/build/EDK-II.sln` and build the entire solution to prepare libraries.
3. Open the main `hyper-reV.sln` and build the `uefi-boot` project.

## 3. Hyper-V Attachment (`hyperv-attachment`)
This module is architecture-specific.
1. Open `hyperv-attachment/src/arch_config.h`.
2. For **Intel**: Ensure `#define _INTELMACHINE` is present.
3. For **AMD**: Comment out `#define _INTELMACHINE`.
4. Build the `hyperv-attachment` project in Visual Studio.

## 4. Usermode App
Build the `usermode` project as a standard C++ console application. It works for both Intel and AMD.
