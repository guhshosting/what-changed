# what-changed 1.0.0 initial validation report

> Historical report for the original candidate. See `REPAIR-VALIDATION.md` for
> the current repaired source and its updated 40-test validation.

Validation date: 2026-09-16 UTC. Environment: Linux container, GCC 13.3.0 (Ubuntu toolchain). All environment-dependent commands were bounded.

## Results actually performed

| Stage | Result |
|---|---|
| GCC Debug, `-std=c++17 -Wall -Wextra -Wpedantic -Werror -O0 -g` | PASS; executable and tests compiled |
| Debug tests | PASS; 34/34 |
| GCC Release, same warnings plus `-O3 -DNDEBUG` | PASS; executable and tests compiled |
| Release tests | PASS; 34/34 |
| ASan + UBSan (`-O1`, halt on error, leak detection off) | PASS; 34/34; no findings |
| LeakSanitizer | Environment-limited; failed while opening container `/proc/.../task`, with the tool reporting its ptrace/proc limitation |
| Independent JSON parsing | PASS; Python `json` parsed real snapshot, metadata JSON, and comparison JSON |
| Real environment capture | PASS; all 8 categories `ok`; 693 packages and 125 service units observed |
| Closely spaced real snapshots | PASS; 0 semantic changes, 0 unavailable comparisons; both files mode 0600 |
| Direct identifier audit | PASS; current hostname, machine ID, local non-system username, and MAC patterns absent. IPv4-shaped matches were confined to package-version strings. |
| Man page | PASS; rendered successfully with GNU groff |
| Debian binary package | PASS; built and inspected with `dpkg-deb`; contains executable, README, LICENSE, and man page |
| Runtime linkage | PASS; normal libc, libstdc++, libgcc_s, and libm only |
| CLI smoke tests | PASS; version, categories, validate, show, compare; malformed input returned exit 3 |
| Read-only/network review | PASS; no shell, network calls, or system-mutating collector calls found; only explicit snapshot output uses permission changes on its own temporary file |

## Automated test scope (34 cases)

Fixture/simulated coverage includes Debian/dpkg, Fedora RPM query output, minimal/missing package manager, missing systemd, missing data, package install/removal/upgrade/downgrade/architecture, service add/remove/enable/disable and transient-noise suppression, config content/mode/owner/disappearance, symlink non-following, oversized files, repository credentials, privacy sentinels, SHA-256, atomic no-overwrite, deterministic collection/comparison, JSON escaping, terminal sanitization, duplicate keys, truncation, invalid schema, unsupported future schema, unknown fields, invalid UTF-8, stable ordering, round trip, and unavailability-versus-removal.

Controlled fixture changes verified meaningful package, service, configuration, and repository comparisons. Reversed package comparisons produced upgrade/downgrade and enable/disable inversions. Capture timestamp changes produced no semantic difference.

## Static analysis

`cppcheck` was not installed, so no cppcheck result is claimed. GCC `-fanalyzer` whole-program analysis reached its 60-second bound. Bounded per-file analysis completed without findings for `json.cpp`, `util.cpp`, and `main.cpp`; `snapshot.cpp`, `collectors.cpp`, `diff.cpp`, and `output.cpp` each exceeded their 30-second analysis bound. Normal GCC compilation with the full warning policy passed for every file.

## Packaging and unavailable tooling

CMake was not installed in this container, so CMake configure/build/CTest/CPack could not be executed here. The project contains CMake build, test, install, and CPack metadata. A Debian package was nevertheless built from the final Release binary and inspected. `rpmbuild` was not installed, so the included RPM spec was reviewed but no RPM binary package is claimed. GNU groff was available; man-page rendering was performed.

## Focused final review

- False changes/noise: timestamps, PIDs, uptime, counters, runtime mounts, and service activity are omitted or opt-in; real back-to-back captures were clean.
- False removals: unavailable categories are reported separately and never interpreted as entry removal.
- Privacy: sentinel fixtures and a real identifier audit passed; configuration contents are never stored; repository credentials are stripped/omitted.
- Snapshot security: bounded parser, depth/count/string limits, duplicate-key rejection, UTF-8 validation, no-follow config hashing, 64 MiB config cap, same-directory private temporary output, and atomic no-replace publication were reviewed and tested where applicable.
- Determinism: ordered maps, normalized mount options, sorted repository files, repeat comparisons, and deterministic serialization passed.
- Read-only behavior: collectors read files/directories and use only fixed read-only `rpm -qa` and `systemctl list-units`; no shell is used.
- Causal overreach: CLI, reports, and documentation describe observations and explicitly disclaim causal conclusions.

## Known limitations

- Package pin/hold state is not collected in 1.0.0.
- RPM collection requires `/usr/bin/rpm` or `/bin/rpm`; active service state requires a working systemd manager and explicit opt-in.
- Configuration hashing is capped at 64 MiB and does not follow symlinks.
- Repository sanitization is deliberately conservative but cannot prove arbitrary third-party formats contain no secret; snapshots should be reviewed before sharing.
- Package-version direction uses a deterministic natural comparison, not package-manager-specific Debian/RPM version algorithms in every edge case; ambiguous equal-order changes are labeled `version-changed`.
- Physical Debian 13 validation remains for the user's independent publication test; this container was not a physical Debian 13 host.

## Clean-room result

The source-only archive was inspected to exclude build directories, binaries, objects, packages, snapshots, logs, caches, and generated/compressed man pages. It was extracted into a fresh directory, compiled in Release mode using the exact warning policy, and the complete 34-test suite passed. CMake clean-room validation was unavailable only because CMake itself was absent.
