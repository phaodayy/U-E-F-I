# protec

This folder stores defensive notes and reusable code for protecting the anti-cheat detector from:

- forced crash
- usermode bypass
- offline dumping and string extraction

Files:

- `01-crash-hardening.md`: crash-resilience checklist and design guidance
- `02-bypass-hardening.md`: how to make bypasses more expensive and easier to detect
- `03-dump-hardening.md`: how to reduce dump value and string leakage
- `win_runtime_hardening.h`: reusable Windows hardening declarations
- `win_runtime_hardening.cpp`: reusable Windows hardening implementation

Recommended order:

1. Apply `win_runtime_hardening.*` early in process startup.
2. Move fragile logic behind interfaces that can fail closed without crashing.
3. Split trust so a bypass in one layer does not disable the whole product.
4. Minimize long-lived secrets and obvious strings in release builds.

Scope:

- This folder is for defensive engineering only.
- It is not wired into the current detector automatically.
- It is intended to be reused from `PUBG-2`, the detector, or any future service/helper.
