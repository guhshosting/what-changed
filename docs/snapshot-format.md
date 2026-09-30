# Snapshot format — schema 1

UTF-8 JSON, maximum accepted file size 16 MiB and maximum nesting depth 64. Object keys are unique. Unknown top-level or category wrapper fields are rejected. Strings are bounded by context and collections have hard entry limits.

```json
{
  "application_version": "1.0.0",
  "capture_metadata": {
    "captured_at": "2026-09-16T00:00:00Z",
    "privacy_profile": "default",
    "semantic_comparison_excludes": ["captured_at"]
  },
  "categories": {
    "packages": {
      "entries": {
        "example": {"architecture":"amd64","manager":"dpkg","version":"1.2"}
      },
      "note": "dpkg database",
      "status": "ok"
    }
  },
  "schema_version": 1
}
```

`status` is `ok`, `partial`, or `unavailable`. Entry values are category-specific canonical JSON. Null before/after values in comparison output mean absent, not unavailable. Unavailability is reported separately.

Schema 1 writers sort all object keys. Readers reject schema versions above 1; a future reader may migrate older versions, but 1.0.0 performs no speculative conversion. The capture timestamp supports interpretation and audit but does not create a semantic change.
