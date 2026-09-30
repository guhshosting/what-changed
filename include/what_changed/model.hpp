#pragma once
#include "what_changed/json.hpp"
#include <map>
#include <string>
#include <vector>

namespace wc {
struct Category { std::string status{"unavailable"}; std::string note; std::map<std::string, Json> entries; };
struct Snapshot {
  int schema_version{1}; std::string application_version{WHAT_CHANGED_VERSION};
  std::string captured_at; std::map<std::string, Category> categories;
};
struct Change { std::string category, key, type, explanation; Json before, after; bool contextual{false}; };
struct Diff { std::vector<Change> changes; std::vector<std::string> unavailable; };

Json snapshot_to_json(const Snapshot&); Snapshot snapshot_from_json(const Json&);
std::string serialize_snapshot(const Snapshot&, bool pretty=true);
Snapshot parse_snapshot(const std::string&);
}
