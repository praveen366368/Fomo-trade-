#include "core/config.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "core/util.hpp"

namespace fomo {

Config Config::from_string(const std::string& text) {
  Config c;
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    const auto hash = line.find('#');
    if (hash != std::string::npos) line = line.substr(0, hash);
    const auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    const std::string k = trim(line.substr(0, eq));
    const std::string v = trim(line.substr(eq + 1));
    if (!k.empty()) c.values_[k] = v;
  }
  return c;
}

Config Config::load_file(const std::string& path) {
  std::ifstream f(path);
  if (!f) throw std::runtime_error("cannot open config file: " + path);
  std::stringstream ss;
  ss << f.rdbuf();
  return from_string(ss.str());
}

std::string Config::raw(const std::string& key, bool* found) const {
  std::string env = "FOMO_" + key;
  std::transform(env.begin(), env.end(), env.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
  if (const char* e = std::getenv(env.c_str())) {
    *found = true;
    return e;
  }
  auto it = values_.find(key);
  if (it == values_.end()) {
    *found = false;
    return "";
  }
  *found = true;
  return it->second;
}

bool Config::has(const std::string& key) const {
  bool found = false;
  raw(key, &found);
  return found;
}

std::string Config::str(const std::string& key, const std::string& def) const {
  bool found = false;
  std::string v = raw(key, &found);
  return found ? v : def;
}

double Config::num(const std::string& key, double def) const {
  bool found = false;
  std::string v = raw(key, &found);
  if (!found || v.empty()) return def;
  try {
    return std::stod(v);
  } catch (...) {
    throw std::runtime_error("config key '" + key + "' is not a number: " + v);
  }
}

int Config::integer(const std::string& key, int def) const {
  return static_cast<int>(num(key, def));
}

bool Config::flag(const std::string& key, bool def) const {
  bool found = false;
  std::string v = raw(key, &found);
  if (!found || v.empty()) return def;
  return v == "1" || v == "true" || v == "yes" || v == "on";
}

std::vector<std::pair<double, double>> Config::pairs(const std::string& key) const {
  std::vector<std::pair<double, double>> out;
  const std::string v = str(key);
  if (v.empty()) return out;
  for (const auto& part : split(v, ',')) {
    const auto bits = split(trim(part), ':');
    if (bits.size() != 2) {
      std::string msg = "config key '";
      msg += key;
      msg += "' bad pair: ";
      msg += part;
      throw std::runtime_error(msg);
    }
    out.emplace_back(std::stod(bits[0]), std::stod(bits[1]));
  }
  return out;
}

}  // namespace fomo
