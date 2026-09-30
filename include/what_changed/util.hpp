#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
namespace wc {
std::string read_file_bounded(const std::filesystem::path&, std::size_t max_bytes);
void atomic_write_private(const std::filesystem::path&, const std::string&);
std::string sha256_file_nofollow(const std::filesystem::path&, std::uint64_t max_bytes, std::uint64_t& size);
std::string utc_now();
std::string sanitize_terminal(const std::string&);
std::string sanitize_repo_url(const std::string&);
std::vector<std::string> split(const std::string&, char);
std::string trim(std::string);
std::optional<std::string> executable_path(const std::vector<std::string>& fixed);
struct CommandResult { int exit_code{-1}; bool timed_out{false}; std::string out, err; };
CommandResult run_command(const std::string&, const std::vector<std::string>&, unsigned timeout_ms=5000, std::size_t max_output=8U*1024U*1024U);
}
