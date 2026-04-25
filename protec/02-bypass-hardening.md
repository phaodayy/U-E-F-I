# Bypass Hardening

Goal: make simple patching, API hooking, and one-check bypasses insufficient.

Principles:

- Never trust a single process, thread, module, or signal.
- Never let one boolean disable the whole detector.
- Assume usermode hooks can lie.

Hardening strategy:

- Split checks across multiple components.
- Cross-check local state with remote policy when possible.
- Verify both presence and freshness of data.
- Prefer multiple cheap checks over one expensive all-or-nothing check.
- Treat disabled telemetry as a signal, not as absence of evidence.

Patterns that help:

- Mutual watchdog between separate binaries.
- Signed policy/config blobs instead of editable plaintext rules.
- Redundant integrity checks on critical code and config.
- Heartbeat with monotonic counters so replay is obvious.
- Canary functions or decoy checks to detect blanket patching.
- Randomized scan order and timing inside safe operational bounds.

What not to rely on:

- one exported function returning "healthy"
- one registry key
- one mutex
- one anti-debug check
- one usermode inline hook scan

Recommended release posture:

- Release build should remove verbose strings and debug-only paths.
- Symbols should stay on a private server, not next to binaries.
- Feature flags should be server-authenticated, not local-only.
- Version mismatches should be explicit and logged.

Response model:

- If one layer is disabled but others still work, keep collecting evidence.
- If multiple layers drop at once, fail closed or quarantine the session.
- Log neutralization attempts separately from ordinary failures.
