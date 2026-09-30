# Collectors and normalization

| Category | Source | Noise/privacy handling |
|---|---|---|
| operating-system | `/etc/os-release`, `uname` | selected keys only; no hostname |
| boot-kernel | `uname`, lockdown, EFI presence | no boot ID, command line, timestamps |
| packages | dpkg status or bounded `rpm -qa` | sorted name keys; versions and architecture |
| services | systemd unit directories and enablement symlinks | active/sub-state is opt-in and contextual |
| configuration | five defaults plus repeated `--track` | no contents; no symlink following; 64 MiB hash cap |
| mounts | `/proc/self/mountinfo` | omits device IDs/sources and common runtime submounts; selected options |
| hardware | `/sys/class/block`, `/sys/class/net` | no serials, MACs, PCI IDs; network count only |
| repositories | APT/YUM config | comments removed; credentials sanitized; 1 MiB/file cap |

Normal runtime counters, PIDs, uptime, temporary files, and timestamps are omitted. Directory traversal is sorted through map storage. An unreadable or failed category retains status so comparison reports uncertainty rather than disappearance.
