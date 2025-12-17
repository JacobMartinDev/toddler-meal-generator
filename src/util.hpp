#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

inline std::string to_lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}

inline bool contains_ci(const std::vector<std::string>& v, const std::string& needle) {
  const std::string n = to_lower(needle);
  for (const auto& x : v) {
    if (to_lower(x) == n) return true;
  }
  return false;
}

inline void trim_inplace(std::string& s) {
  auto not_space = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
  s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}
