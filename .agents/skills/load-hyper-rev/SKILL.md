---
name: load-hyper-rev
description: Steps to deploy hyper-reV to the EFI partition and manage boot security.
---

# Load hyper-reV Skill

This skill covers the deployment of the hypervisor and handling security features like Secure Boot and TPM.

## 1. Automated Deployment
1. Ensure `uefi-boot.efi` and `hyperv-attachment.dll` are in the same directory as `load-hyper-reV.bat`.
2. Run `load-hyper-reV.bat` as **Administrator**.
3. This script mounts the EFI partition and copies the files, replacing the Windows bootloader temporarily.

## 2. Booting
1. Restart the computer.
2. The `uefi-boot` module will execute first, restore the original bootloader in memory, and then launch the system.

## 3. Security Considerations
- **Secure Boot**: If enabled, use a vulnerable bootloader exploit (e.g., BlackLotus or similar bypasses) to load unsigned code.
- **TPM / Measured Boot**: Avoid running if the system uses boot attestation, as the `uefi-boot` binary will be measured and might trigger detection.
