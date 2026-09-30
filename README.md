# what-changed

`what-changed` 1.0.0 is a read-only, privacy-conscious Linux CLI that captures curated system snapshots and compares them. It reports observations, not causes.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

Requires Linux, CMake 3.16+, and a C++17 compiler. There are no runtime libraries beyond the normal C++/C system libraries. Optional RPM collection uses a fixed `/usr/bin/rpm` or `/bin/rpm`; transient service collection uses fixed `systemctl` paths. Both calls have short timeouts, never use a shell, and are read-only.

## Usage

```sh
what-changed capture -o before.json
what-changed capture -o after.json --track /etc/myapp.conf
what-changed compare before.json after.json
what-changed compare before.json after.json --format json
what-changed show before.json --format markdown
what-changed validate before.json
what-changed categories
```

Use repeated `--category NAME` to select categories or `--exclude NAME` to omit them. `--include-transient` opts into systemd active/sub-state. Snapshot files are created atomically with mode `0600`; existing paths are never overwritten.

## Captured by default

- Distribution release, current kernel release, and architecture.
- Kernel lockdown when readable and EFI presence.
- Installed dpkg packages, or RPM packages where `rpm` is available: name, version, architecture, manager.
- Discovered systemd service units and persistent enablement inferred from enablement links.
- Hash, size, Unix mode, UID/GID, type, and readability state for a small predefined configuration list: `/etc/fstab`, `/etc/ssh/sshd_config`, `/etc/sysctl.conf`, `/etc/default/grub`, and `/etc/security/limits.conf`.
- Non-runtime mount points, filesystem type, and selected security/semantic mount options.
- High-level block-device presence/removability and only the count of non-loopback network interfaces.
- Normalized APT/YUM repository configuration with URL userinfo, query strings, fragments, and credential-bearing lines removed.

## Omitted by default

Hostnames, usernames, home paths, IP/MAC addresses, SSIDs, machine IDs, filesystem UUIDs, hardware serials, environment variables, configuration contents, private keys, package repository credentials, PIDs, uptime, counters, and service active state. Snapshot timestamps are retained as capture context but ignored by semantic comparison.

Tracked configuration paths must be absolute. Symlinks are recorded but never followed. Regular files are hashed with SHA-256 and bounded to 64 MiB; contents are not stored. This is privacy-conscious minimization, not anonymity: package lists and selected system structure may still identify unusual installations. Treat snapshots as potentially sensitive.

## Compatibility and semantics

Schema version 1 is documented in [docs/snapshot-format.md](docs/snapshot-format.md). Readers reject unsupported future versions and unknown structural fields rather than guessing. Entry order and JSON object order are canonical. A category marked `unavailable` in either snapshot produces an unavailable comparison, never false removals. Transient service changes are excluded unless requested.

Exit codes: `0` success, `2` CLI usage, `3` invalid/unsupported snapshot, `4` filesystem failure, `5` collection/other runtime failure. Diagnostics go to stderr; requested JSON remains clean on stdout.

## Platform limitations

Linux only. dpkg state is read directly from `/var/lib/dpkg/status`. RPM needs the `rpm` executable. Service discovery works without a running systemd manager, but active state requires it and is opt-in. Permissions may yield partial or unavailable observations. Package pin/hold state is not collected in 1.0.0 because no cross-manager source was reliable enough under the safety and dependency constraints.

See [architecture](docs/architecture.md), [privacy](docs/privacy.md), [collectors](docs/collectors.md), and the installed `what-changed(1)` manual.

## License

MIT.
