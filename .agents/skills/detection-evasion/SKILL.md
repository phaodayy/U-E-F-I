---
name: detection-evasion
description: Methods used by hyper-reV to avoid detection by anti-cheats and security tools.
---

# Detection Evasion Skill

This skill explains the stealth mechanisms implemented in hyper-reV to remain undetected.

## 1. Independent Allocation
The `hyperv-attachment` is allocated independently from the main Hyper-V image. This prevents detection methods that look for memory shifts or unexpected growth of the hypervisor image in physical memory.

## 2. Late Hooking
Hooks are not applied to the initial Hyper-V image loaded from disk. Instead, they are applied only to the final, SLAT-protected runtime image. This bypasses security tools that scan for modifications in static or unprotected memory copies.

## 3. Metadata Restoration
The `uefi-boot` module restores the original `bootmgfw.efi` file and its modified timestamps (MACE attributes) early in the boot process, leaving no trace of a modified bootloader on the filesystem.

## 4. SLAT Dummy Pages
The UEFI heap and other internal structures used by hyper-reV are hidden by mapping their guest physical addresses to free dummy pages in the SLAT, making the real memory inaccessible and invisible to the guest.
