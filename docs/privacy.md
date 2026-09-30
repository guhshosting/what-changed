# Privacy and snapshot security

Default snapshots intentionally omit direct user, host, network, device, and secret identifiers. Configuration contents are represented only by metadata and SHA-256. Hashes reveal equality and may permit guessing of known content; they are not anonymization. UID/GID and unusual package/unit sets can also be identifying.

Repository URLs lose userinfo, query, and fragment portions. Lines visibly containing `password=` or `token=` are omitted. This is conservative filtering, not a universal secret detector; users should inspect snapshots before sharing.

Output creation uses a same-directory temporary file, `0600`, `fsync`, and atomic rename. Existing paths—including symlinks—are rejected. Tracked configuration is inspected with `lstat` and regular files are opened using `O_NOFOLLOW`. Files over 64 MiB are not hashed. Snapshot loading limits total bytes, nesting, string/key sizes, entry counts, and rejects duplicate keys, truncation, invalid numbers, and unsupported schemas.

No data is transmitted. There is no networking or telemetry. Diagnostics can name an explicitly supplied snapshot path, so avoid placing secrets in filenames.
