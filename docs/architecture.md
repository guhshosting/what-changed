# Architecture

The executable is a thin CLI over six modules:

- `collectors`: bounded, read-only observation using fixture-rootable filesystems and two fixed-path subprocesses.
- `model`/`snapshot`: typed category status and schema validation.
- `json`: dependency-free bounded parser with duplicate-key and depth rejection.
- `diff`: deterministic, directional semantic classification; collection failure is not removal.
- `output`: terminal-sanitized text/Markdown and valid JSON.
- `util`: bounded reads, SHA-256, private atomic output, URL sanitization, and timeout-controlled `fork`/`execv` without a shell.

Collectors return `ok`, `partial`, or `unavailable`. Comparison only interprets entry absence when both category collections are usable. `std::map` provides stable key ordering. Persistent service state is compared independently of opt-in active state. Capture metadata is intentionally outside semantic comparison.

The program never calls package mutation, service mutation, mount, sysctl, firewall, ownership, or permission-changing tools. The only writes are an explicitly named snapshot and its same-directory temporary file during atomic creation.
