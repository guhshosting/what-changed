#pragma once
#include "what_changed/model.hpp"
#include <string>
namespace wc {
enum class Format { Text, Json, Markdown };
Format parse_format(const std::string&);
std::string format_diff(const Diff&, Format);
std::string format_metadata(const Snapshot&, Format);
}
