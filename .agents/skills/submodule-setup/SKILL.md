---
name: submodule-setup
description: Managing and initializing external dependencies for the project.
---

# Submodule Setup Skill

This skill ensures that all external libraries required for `uefi-boot` are correctly pulled and configured.

## 1. Required Submodules
- **OpenSSL**: `https://github.com/openssl/openssl.git` (Branch: `OpenSSL_1_1_0-stable`)
- **EDK2**: `https://github.com/ionescu007/edk2.git` (VisualUefi fork)

## 2. Initialization Commands
If clones were not done recursively, use these:
```bash
git clone --branch OpenSSL_1_1_0-stable https://github.com/openssl/openssl.git uefi-boot/ext/openssl
git clone https://github.com/ionescu007/edk2.git uefi-boot/ext/edk2/src
```

## 3. Library Preparation
Before the main project build, you must build the EDK2 libraries:
1. Navigate to `uefi-boot/ext/edk2/build/`.
2. Open `EDK-II.sln`.
3. Build the solution in **Release** or **Debug** mode as needed for your target.
