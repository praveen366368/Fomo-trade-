// Agents 5-7: MarketData, ApiHealth, Volatility.
#include <algorithm>
#include <cmath>
#include <sstream>

#include "agents/agent.hpp"
#include "core/log.hpp"
#include "core/parse.hpp"
#include "core/util.hpp"

namespace fomo {

// ------------------------------------------------------------- 5 MarketData
MarketData::MarketData(Http& http, const Config& c)
    : Agent(5, "MarketData", "batch price, liquidity and order-flow snapshots"), http_(http) {
  sol_usd_ = c.num("sol_usd_fallback", 0);
}

std::map<std::string, PairSnapshot> MarketData::fetch(const std::vector<std::string>& mints, int max_calls) {
  std::map<std::string, PairSnapshot> out;
  std::vector<double> sol_samples;
  int calls = 0;
  for (size_t i = 0; i < mints.size() && calls < max_calls; i += 30, ++calls) {
    std::string csv;
    for (size_t k = i; k < mints.size() && k < i + 30; ++k) csv += (csv.empty() ? "" : ",") + mints[k];
    auto r = http_.get("https://api.dexscreener.com/tokens/v1/solana/" + csv);
    count_run();
    if (!r.ok()) continue;
    for (auto& [m, s] : parse::dexscreener_tokens(r.body, now_ms())) {
      if (s.quote_mint == kWsolMint && s.price_native > 0) sol_samples.push_back(s.price_usd / s.price_native);
      out[m] = s;
    }
  }
  if (sol_samples.size() >= 3) {
    std::sort(sol_samples.begin(), sol_samples.end());
    sol_usd_ = sol_samples[sol_samples.size() / 2];
  }
  return out;
}

double MarketData::sol_usd(int64_t now) {
  if (sol_usd_ <= 0 || now >= sol_next_) {
    auto r = http_.get(std::string("https://lite-api.jup.ag/price/v3?ids=") + kWsolMint);
    if (r.ok()) {
      double p = parse::jupiter_price(r.body, kWsolMint);
      if (p > 0) sol_usd_ = p;
    }
    sol_next_ = now + 300000LL;
  }
  return sol_usd_;
}

// -------------------------------------------------------------- 6 ApiHealth
ApiHealth::ApiHealth(Http& http, const Config& c)
    : Agent(6, "ApiHealth", "watches API latency, errors and rate limits"),
      http_(http),
      max_error_rate_(c.num("health_max_error_rate", 0.5)) {}

bool ApiHealth::entries_allowed() {
  auto now = http_.stats();
  bool ok = true;
  const auto it = now.find("api.dexscreener.com");
  if (it != now.end()) {
    const auto prev = last_.count(it->first) ? last_[it->first] : HostStats{};
    const long calls = it->second.calls - prev.calls;
    const long errs = it->second.errors - prev.errors;
    if (calls >= 3 && static_cast<double>(errs) / static_cast<double>(calls) > max_error_rate_) {
      ok = false;
      log::warn(name(), "price feed unhealthy: " + std::to_string(errs) + "/" + std::to_string(calls) +
                            " errors; pausing new entries");
    }
  }
  last_ = now;
  count_run();
  return ok;
}

std::string ApiHealth::table() const {
  std::ostringstream o;
  for (const auto& [host, s] : http_.stats()) {
    char line[200];
    std::snprintf(line, sizeof line, "  %-28s calls=%-5ld err=%-4ld 429=%-4ld avg=%.0fms\n", host.c_str(),
                  s.calls, s.errors, s.rate_limited, s.avg_ms());
    o << line;
  }
  return o.str();
}

// ------------------------------------------------------------- 7 Volatility
Volatility::Volatility(const Config& c)
    : Agent(7, "Volatility", "measures how violently a coin is moving"),
      max_vol_(c.num("vol_max_per_min", 0.35)) {}

void Volatility::observe(const PairSnapshot& s) {
  auto& h = hist_[s.mint];
  if (!h.empty() && s.ts_ms - h.back().first < 2000) return;
  h.emplace_back(s.ts_ms, s.price_usd);
  if (h.size() > 60) h.erase(h.begin());
}

double Volatility::vol_per_min(const std::string& mint) const {
  auto it = hist_.find(mint);
  if (it == hist_.end() || it->second.size() < 3) return -1;
  std::vector<double> r;
  const auto& h = it->second;
  for (size_t i = 1; i < h.size(); ++i) {
    const double dt_min = static_cast<double>(h[i].first - h[i - 1].first) / 60000.0;
    if (dt_min <= 0 || h[i - 1].second <= 0 || h[i].second <= 0) continue;
    r.push_back(std::log(h[i].second / h[i - 1].second) / std::sqrt(dt_min));
  }
  if (r.size() < 2) return -1;
  double mean = 0;
  for (double x : r) mean += x;
  mean /= static_cast<double>(r.size());
  double var = 0;
  for (double x : r) var += (x - mean) * (x - mean);
  return std::sqrt(var / static_cast<double>(r.size() - 1));
}

Verdict Volatility::check(const Candidate& c) {
  const double v = vol_per_min(c.mint);
  if (v < 0) return pass(0, "not enough history yet");
  char why[96];
  std::snprintf(why, sizeof why, "vol %.2f/min", v);
  if (v > max_vol_) return fail(std::string(why) + " above max", false, -10);
  return pass(v > max_vol_ / 2 ? -3 : 2, why);
}

}  // namespace fomo
