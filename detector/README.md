# detector

`detector.exe` is a defensive scanner for fast detection of boot-stage and helper-tool anomalies.

The detector is designed to keep working even when the source code changes. It still uses some known IOCs for the current repo, but the stronger signals come from generic heuristics.

What it checks:

- Startup baseline hardening from `..\protec\win_runtime_hardening.*`.
- EFI artifacts in `\EFI\Microsoft\Boot` such as `uefi-boot.efi`, `loader.exe`, `load-hyper-reV.txt`, `bootmgfw.original.efi`, and `hyperv-attachment.dll`.
- Recursive artifacts on `B:\` for the same known filenames plus executable-content heuristics.
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

After each run, the detector also writes the same report to a timestamped file under:

```text
bin\reports\YYYYMMDD-HHMMSS.txt
```

Notes:

- Run as administrator if you want EFI scanning and broader process access.
- `runtime_hardening` in the report shows whether the detector successfully enabled its own baseline mitigations at startup.
- `b_drive_scan` will show `ok` when `B:\` exists and was scanned, or `not_found_or_failed` when the drive is missing or unreadable.
- The strongest future-proof signals are in `efi_heuristic` and `process_image` findings, not just exact string hits.
- `HIGH_CONFIDENCE_DETECTION` means the machine has strong indicators and should be reviewed immediately.
- `REVIEW` means there are useful signals but not enough to trust as a single-source instant ban.
- `LOW_SIGNAL` means the fast IOC scan did not find enough evidence.
- The final console line shows `report_file: ...` so you can see exactly where the saved report went.
