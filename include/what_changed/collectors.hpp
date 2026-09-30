#pragma once
#include "what_changed/model.hpp"
#include <filesystem>
#include <set>
#include <string>
#include <vector>
namespace wc {
struct CaptureOptions {
  std::set<std::string> include; std::set<std::string> exclude;
  std::vector<std::filesystem::path> config_paths;
  std::filesystem::path root{"/"}; bool include_transient{false};
};
const std::vector<std::string>& supported_categories();
Category parse_rpm_query_output(const std::string& output);
Snapshot capture(const CaptureOptions&);
}
