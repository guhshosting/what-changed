#pragma once
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace wc {
class Json {
public:
  using Object = std::map<std::string, Json>;
  using Array = std::vector<Json>;
  using Value = std::variant<std::nullptr_t, bool, std::int64_t, std::string, Array, Object>;
  Json();
  // Implicit conversions are intentional: they make Json::Object and
  // Json::Array initializer lists concise and are part of this internal JSON
  // construction API.
  // cppcheck-suppress noExplicitConstructor
  Json(std::nullptr_t);
  // cppcheck-suppress noExplicitConstructor
  Json(bool);
  // cppcheck-suppress noExplicitConstructor
  Json(std::int64_t);
  // cppcheck-suppress noExplicitConstructor
  Json(const char*);
  // cppcheck-suppress noExplicitConstructor
  Json(std::string);
  // cppcheck-suppress noExplicitConstructor
  Json(Array);
  // cppcheck-suppress noExplicitConstructor
  Json(Object);
  bool is_null() const; bool is_bool() const; bool is_int() const; bool is_string() const; bool is_array() const; bool is_object() const;
  bool as_bool() const; std::int64_t as_int() const; const std::string& as_string() const;
  const Array& as_array() const; Array& as_array(); const Object& as_object() const; Object& as_object();
  const Json& at(const std::string&) const; bool contains(const std::string&) const;
  std::string dump(bool pretty=false, unsigned indent=0) const;
  static Json parse(const std::string& input, std::size_t max_bytes=16U*1024U*1024U, unsigned max_depth=64);
private: Value value_;
};
class JsonError : public std::runtime_error { public: using std::runtime_error::runtime_error; };
}
