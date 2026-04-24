# detector

`detector.exe` is a defensive scanner for fast detection of boot-stage and helper-tool anomalies.

The detector is designed to keep working even when the source code changes. It still uses some known IOCs for the current repo, but the stronger signals come from generic heuristics.

What it checks:

- EFI artifacts in `\EFI\Microsoft\Boot` such as `uefi-boot.efi`, `loader.exe`, `load-hyper-reV.txt`, `bootmgfw.original.efi`, and `hyperv-attachment.dll`.
- String IOCs in readable `MEM_IMAGE` and `MEM_PRIVATE` regions of running processes.
- Marker strings inside suspicious EFI files when those files are present.
- Generic executable heuristics:
  - unexpected `.exe` or `.dll` files under `EFI\Microsoft\Boot`
  - non-standard `.efi` names in the Microsoft boot path
  - `bootmgfw.efi` metadata that does not look like Microsoft
  - multiple embedded PE images inside one executable
  - capability clusters related to EFI staging, boot edits, privilege escalation, and memory introspection
  - elevated processes running from user-writable paths with suspicious capability clusters

How to build:

```bat
build-detector.bat Release
```

How to run:

```bat
bin\detector.exe
```

Notes:

- Run as administrator if you want EFI scanning and broader process access.
- The strongest future-proof signals are in `efi_heuristic` and `process_image` findings, not just exact string hits.
- `HIGH_CONFIDENCE_DETECTION` means the machine has strong indicators and should be reviewed immediately.
- `REVIEW` means there are useful signals but not enough to trust as a single-source instant ban.
- `LOW_SIGNAL` means the fast IOC scan did not find enough evidence.
