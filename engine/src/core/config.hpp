#pragma once
// key = value config file, with FOMO_<KEY> environment overrides.
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fomo {

class Config {
 public:
  Config() = default;
  static Config load_file(const std::string& path);
  static Config from_string(const std::string& text);

  void set(const std::string& key, const std::string& value) { values_[key] = value; }
  bool has(const std::string& key) const;

  std::string str(const std::string& key, const std::string& def = "") const;
  double num(const std::string& key, double def) const;
  int integer(const std::string& key, int def) const;
  bool flag(const std::string& key, bool def) const;

  // "2:0.5,5:0.25" -> {(2,0.5),(5,0.25)}
  std::vector<std::pair<double, double>> pairs(const std::string& key) const;

 private:
  std::string raw(const std::string& key, bool* found) const;
  std::map<std::string, std::string> values_;
};

}  // namespace fomo
