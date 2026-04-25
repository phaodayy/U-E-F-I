# Crash Hardening

Goal: make the detector fail closed, keep logging, and avoid easy forced termination by malformed state or hostile runtime conditions.

Threats to expect:

- invalid pointers or corrupted buffers returned by hooked APIs
- partial tampering that causes null dereference or access violation
- hostile suspension or starvation of critical worker threads
- malformed files or IPC payloads
- loader races and missing dependencies

Design rules:

- Keep the startup path small and deterministic.
- Put risky parsing and file inspection behind strict bounds checks.
- Treat every external input as hostile, including registry, filesystem, and IPC.
- Prefer explicit error paths over implicit assumptions.
- Do not let telemetry failure crash core detection.

Runtime checklist:

- Enable heap termination on corruption.
- Use hardened DLL loading order.
- Turn off Windows error UI for release builds.
- Put watchdog and detector in separate failure domains.
- Add timeouts around blocking I/O and child-process waits.
- Log every failed mitigation call with `GetLastError()`.
- Rate-limit expensive scans so one bad target cannot freeze the product.

Architecture guidance:

- Separate UI, service, and scanner responsibilities.
- Put risky scanning code in a worker process if possible.
- Use heartbeat between components instead of one-way trust.
- If one component dies, mark the session suspicious instead of silently continuing.

Testing:

- Run with Application Verifier and page heap in internal builds.
- Fuzz parsers for config, logs, and imported files.
- Test startup with missing files, denied permissions, and empty directories.
- Test under heavy process churn while many handles fail to open.

Telemetry to keep:

- process start/stop reason
- last successful integrity check
- mitigation enablement status
- crash point module and exception code
- watchdog timeout count
