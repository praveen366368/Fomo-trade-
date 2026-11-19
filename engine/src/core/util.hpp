#pragma once
// Small helpers shared by every agent.
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace fomo {

inline int64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

inline std::string trim(const std::string& s) {
  const auto b = s.find_first_not_of(" \t\r\n");
  if (b == std::string::npos) return "";
  const auto e = s.find_last_not_of(" \t\r\n");
  return s.substr(b, e - b + 1);
}

inline std::vector<std::string> split(const std::string& s, char sep) {
  std::vector<std::string> out;
  std::string cur;
  for (char c : s) {
    if (c == sep) {
      out.push_back(cur);
      cur.clear();
    } else {
      cur.push_back(c);
    }
  }
  out.push_back(cur);
  return out;
}

// Solana addresses are base58 strings of 32..44 characters.
inline bool is_solana_address(const std::string& s) {
  if (s.size() < 32 || s.size() > 44) return false;
  static const std::string alphabet =
      "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
  for (char c : s)
    if (alphabet.find(c) == std::string::npos) return false;
  return true;
}

// Launchpad bonding curves (no LP; Dexscreener reports no liquidity).
inline bool is_bonding_curve_dex(const std::string& dex) {
  return dex == "pumpfun" || dex == "pump-fun" || dex == "moonshot" || dex == "launchlab" ||
         dex == "raydium-launchlab" || dex == "letsbonk" || dex == "bonk" || dex == "meteora-dbc";
}

inline std::string fmt_usd(double v) {
  char buf[64];
  std::snprintf(buf, sizeof buf, "%s$%.4f", v < 0 ? "-" : "", std::fabs(v));
  return buf;
}

inline std::string fmt_pct(double frac) {
  char buf[64];
  std::snprintf(buf, sizeof buf, "%+.1f%%", frac * 100.0);
  return buf;
}

constexpr const char* kWsolMint = "So11111111111111111111111111111111111111112";
constexpr const char* kUsdcMint = "EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v";
constexpr const char* kUsdtMint = "Es9vMFrzaCERmJfrF4H2FYD4KCoNkY11McCe8BenwNYB";

}  // namespace fomo
