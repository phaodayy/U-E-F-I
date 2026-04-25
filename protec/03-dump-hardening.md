# Dump Hardening

Goal: reduce how much value an attacker gets from static analysis, memory dumps, or strings extraction.

Reality:

- Usermode anti-dump is never absolute.
- If secrets live long enough in one process, a determined attacker can usually recover them.
- The correct target is risk reduction, not magical prevention.

High-value controls:

- Move sensitive logic server-side where possible.
- Avoid long-lived plaintext secrets in process memory.
- Derive ephemeral session material from server challenge/response.
- Wipe temporary buffers after use with `SecureZeroMemory`.
- Keep sensitive lookup tables encrypted or generated just-in-time if they must live client-side.
- Keep release symbols private.

String reduction:

- Remove debug banners and verbose operator-facing messages from release.
- Do not embed obvious filenames, command strings, or feature names when not required.
- Split or transform especially sensitive constants if you must store them locally.

Memory handling:

- Allocate sensitive buffers only when needed.
- Shorten object lifetimes.
- Avoid global writable state for secrets.
- Consider `VirtualLock` only for short-lived critical material and only after measuring impact.

Operational controls:

- Separate internal and release builds.
- Keep private diagnostics off customer machines by default.
- Review binaries with a strings pass before release.
- Review release packages for accidental `.pdb`, logs, configs, and test artifacts.

What to accept:

- A dump can still capture generic code paths.
- Focus on protecting policy, signatures, secrets, and decision logic.
