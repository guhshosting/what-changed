# what-changed 1.0.0 repair validation

Validation date: 2026-09-30 UTC. This report covers the repair environment only: Linux with GCC 13.3.0. It does not claim Debian 13 or GCC 14.2 validation.

## Repair scope

The repair preserved the CLI, exit codes, snapshot schema, category names, JSON/text/Markdown formats, privacy rules, package comparison semantics, bounded helpers, and read-only behavior.

Changed files: `include/what_changed/json.hpp`, `src/collectors.cpp`, `src/diff.cpp`, `src/output.cpp`, `src/snapshot.cpp`, `src/util.cpp`, `tests/tests.cpp`, `VALIDATION.md`, and this report.

## Findings and disposition

- `noExplicitConstructor`: the seven converting `Json` constructors are intentionally implicit because `Json::Object` and `Json::Array` initializer lists rely on value conversion. Precise inline cppcheck suppressions document only this intentional API choice.
- `knownConditionTrueFalse` in the dpkg parser: fixed by removing the redundant `!l.empty()` after the empty-line branch.
- `duplicateExpressionTernary`: both identical-output ternaries were simplified without changing bytes.
- `useStlAlgorithm`: the reported loops were converted to `std::replace_if`, `std::transform`, or `std::find_if` where this remained clear.
- `fd>=0 always true`: cppcheck's path conclusion was incomplete because exceptions could occur before or after the original `fd=-1`. Manual FD state was nevertheless difficult to audit and was replaced with scoped ownership.

One genuine exception-safety weakness was fixed: an allocation exception in the original post-fork command-capture path could leave the child running and unreaped until process exit. `ChildOwner` now kills and reaps an owned child during exceptional unwinding.

## FD ownership model

`UniqueFd` exclusively owns each open descriptor and closes it on scope exit. Snapshot output releases ownership immediately before the one explicit checked `close`; it is therefore never double-closed. Any earlier failure closes the descriptor during unwinding and unlinks the temporary path. Publication still uses `renameat2(..., RENAME_NOREPLACE)`, with the existing hard-link/unlink fallback.

`run_command` gives each of the four pipe ends a `UniqueFd`. The parent resets its write ends immediately after `fork`; read ends close automatically on every return and exception. `ChildOwner` owns the child until a successful reap and kills/reaps it if stack unwinding occurs first.

## Results actually performed here

| Gate | Result |
|---|---|
| Strict direct GCC Release (`-Wall -Wextra -Wpedantic -Werror -O3 -DNDEBUG`) | PASS |
| Complete test executable | PASS, 40/40 |
| ASan/UBSan (`detect_leaks=0`) | PASS, 40/40 |
| Original/repaired behavior comparison | PASS |
| Atomic output and subprocess FD cleanup tests | PASS |
| GCC 13 `-fanalyzer`, `util.cpp` | PASS, zero diagnostics |
| GCC 13 per-file `-fanalyzer` | 6/7 PASS; `collectors.cpp` resource-limited |

The behavior comparison covered categories; capture structure after timestamp normalization; snapshot mode 0600; validate; show text/JSON/Markdown; compare text/JSON/Markdown; a zero-change comparison; invalid-input diagnostics and exit code; and no-overwrite diagnostics and exit code. Compared outputs were byte-for-byte identical where timestamps and output paths were not inherently new.

Dedicated tests cover atomic success, mode 0600, no-overwrite, missing-parent failure cleanup, symlink refusal, subprocess success, exec failure, timeout, output limit, and FD-count stability. The suite increased from 34 to 40 cases without changing intended application runtime behavior.

## Environment limitations

- CMake and CTest were not installed, so direct GCC builds and the complete test executable were used. No local CTest result is claimed.
- cppcheck was not installed. The saved GCC 14.2 host report was used to locate each original cppcheck finding, but no repaired cppcheck pass is claimed.
- GCC 14.2 was not installed. No repaired GCC 14 analyzer result is claimed.
- GCC 13 whole-program analyzer exceeded 90 seconds. Six source files passed bounded per-file analysis; `collectors.cpp` timed out at 45 seconds and a later 120-second attempt was killed by the environment without a source diagnostic.
- LeakSanitizer was not rerun because this container previously denied the required `/proc` task inspection. ASan/UBSan ran with leak detection disabled.

The repaired archive should be rerun through CMake/CTest, the exact exhaustive cppcheck command, and GCC 14.2 `-fanalyzer` on the real Debian 13 host.
