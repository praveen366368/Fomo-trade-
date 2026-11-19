#pragma once
// Thread-safe, timestamped console log.
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>

namespace fomo::log {

inline std::mutex& mu() {
  static std::mutex m;
  return m;
}

inline bool& quiet() {
  static bool q = false;
  return q;
}

inline void write(const char* level, const std::string& who, const std::string& msg) {
  if (quiet()) return;
  std::time_t t = std::time(nullptr);
  char ts[32];
  std::strftime(ts, sizeof ts, "%H:%M:%S", std::gmtime(&t));
  std::lock_guard<std::mutex> lock(mu());
  std::fprintf(stderr, "%s %-5s [%s] %s\n", ts, level, who.c_str(), msg.c_str());
}

inline void info(const std::string& who, const std::string& msg) { write("INFO", who, msg); }
inline void warn(const std::string& who, const std::string& msg) { write("WARN", who, msg); }
inline void error(const std::string& who, const std::string& msg) { write("ERROR", who, msg); }

}  // namespace fomo::log
