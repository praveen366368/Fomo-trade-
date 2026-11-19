// Agents 1-4: LaunchScout, TrendScout, SmartMoney, TraderScout.
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

#include "agents/agent.hpp"
#include "core/log.hpp"
#include "core/parse.hpp"
#include "core/util.hpp"

namespace fomo {

using nlohmann::json;

// ------------------------------------------------------------ 1 LaunchScout
LaunchScout::LaunchScout(Http& http, const Config& c)
    : Agent(1, "LaunchScout", "finds brand-new pools and launches"),
      http_(http),
      gecko_every_ms_(c.integer("launch_gecko_every_s", 30) * 1000),
      profiles_every_ms_(c.integer("launch_profiles_every_s", 30) * 1000) {}

std::vector<Lead> LaunchScout::discover(int64_t now) {
  std::vector<Lead> out;
  if (now >= next_gecko_) {
    next_gecko_ = now + gecko_every_ms_;
    auto r = http_.get("https://api.geckoterminal.com/api/v2/networks/solana/new_pools?page=1");
    if (r.ok()) {
      auto v = parse::geckoterminal_new_pools(r.body);
      out.insert(out.end(), v.begin(), v.end());
    }
    count_run();
  }
  if (now >= next_profiles_) {
    next_profiles_ = now + profiles_every_ms_;
    auto r = http_.get("https://api.dexscreener.com/token-profiles/latest/v1");
    if (r.ok()) {
      auto v = parse::dexscreener_leads(r.body, "launch:dex-profile");
      out.insert(out.end(), v.begin(), v.end());
    }
    count_run();
  }
  return out;
}

// ------------------------------------------------------------- 2 TrendScout
TrendScout::TrendScout(Http& http, const Config& c)
    : Agent(2, "TrendScout", "finds coins trending or being promoted (boosts)"),
      http_(http),
      every_ms_(c.integer("trend_every_s", 60) * 1000) {}

std::vector<Lead> TrendScout::discover(int64_t now) {
  std::vector<Lead> out;
  if (now < next_) return out;
  next_ = now + every_ms_;
  for (const char* path : {"token-boosts/latest/v1", "token-boosts/top/v1"}) {
    auto r = http_.get(std::string("https://api.dexscreener.com/") + path);
    if (r.ok()) {
      auto v = parse::dexscreener_leads(r.body, "trend:boost");
      out.insert(out.end(), v.begin(), v.end());
    }
  }
  count_run();
  return out;
}

// ------------------------------------------------------------ 4 TraderScout
TraderScout::TraderScout(const Config& c)
    : Agent(4, "TraderScout", "curates copy-trade wallets and scores them by results"),
      mute_after_signals_(c.integer("trader_mute_after_signals", 8)),
      mute_below_hit_rate_(c.num("trader_mute_below_hit_rate", 0.15)) {}

// CSV: label,address,origin   (lines starting with # are comments)
std::vector<TrackedWallet> TraderScout::snapshot() const {
  std::lock_guard<std::mutex> lock(mu_);
  return wallets_;
}

void TraderScout::load(const std::string& csv_path) {
  std::lock_guard<std::mutex> lock(mu_);
  std::ifstream f(csv_path);
  if (!f) return;
  std::string line;
  while (std::getline(f, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#') continue;
    auto cols = split(line, ',');
    if (cols.size() < 2) continue;
    TrackedWallet w;
    w.label = trim(cols[0]);
    w.address = trim(cols[1]);
    w.origin = cols.size() > 2 ? trim(cols[2]) : "manual";
    if (!is_solana_address(w.address)) {
      log::warn(name(), "skipping invalid wallet address for " + w.label);
      continue;
    }
    wallets_.push_back(w);
  }
  log::info(name(), "tracking " + std::to_string(wallets_.size()) + " wallets from " + csv_path);
}

void TraderScout::record_signal(const std::string& wallet) {
  std::lock_guard<std::mutex> lock(mu_);
  for (auto& w : wallets_)
    if (w.address == wallet) w.signals++;
}

void TraderScout::record_outcome(const std::string& wallet, double peak_multiple) {
  std::lock_guard<std::mutex> lock(mu_);
  for (auto& w : wallets_) {
    if (w.address != wallet) continue;
    if (peak_multiple >= 2.0) w.winners++;
    else w.losers++;
    const int decided = w.winners + w.losers;
    if (decided >= mute_after_signals_ &&
        static_cast<double>(w.winners) / decided < mute_below_hit_rate_ && !w.muted) {
      w.muted = true;
      log::warn(name(), "muting wallet " + w.label + " (hit rate too low)");
    }
  }
}

std::string TraderScout::leaderboard_json() const {
  std::lock_guard<std::mutex> lock(mu_);
  json arr = json::array();
  for (const auto& w : wallets_) {
    const int decided = w.winners + w.losers;
    arr.push_back({{"label", w.label}, {"address", w.address}, {"origin", w.origin},
                   {"signals", w.signals}, {"winners", w.winners}, {"losers", w.losers},
                   {"hit_rate", decided ? static_cast<double>(w.winners) / decided : 0.0},
                   {"muted", w.muted}});
  }
  return arr.dump(2);
}

// ------------------------------------------------------------- 3 SmartMoney
SmartMoney::SmartMoney(Http& http, const Config& c, TraderScout& scout)
    : Agent(3, "SmartMoney", "turns tracked wallets' fresh buys into leads"),
      http_(http),
      scout_(scout),
      rpc_(c.str("solana_rpc_url", "https://api.mainnet-beta.solana.com")),
      every_ms_(c.integer("smart_every_s", 30) * 1000) {}

std::vector<std::string> SmartMoney::new_buys(const std::string& wallet) {
  std::vector<std::string> mints;
  json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "getSignaturesForAddress"},
              {"params", {wallet, {{"limit", 10}}}}};
  auto r = http_.post_json(rpc_, req.dump());
  if (!r.ok()) return mints;
  json j = json::parse(r.body, nullptr, false);
  if (!j.is_object() || !j["result"].is_array() || j["result"].empty()) return mints;
  const std::string newest = j["result"][0].value("signature", "");
  const bool first_time = !last_sig_.count(wallet);
  const std::string last = first_time ? "" : last_sig_[wallet];
  last_sig_[wallet] = newest;
  if (first_time) return mints;  // don't replay old history on startup
  int fetched = 0;
  for (const auto& s : j["result"]) {
    const std::string sig = s.value("signature", "");
    if (sig == last || fetched >= 5) break;
    if (!s["err"].is_null()) continue;
    fetched++;
    json tq = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "getTransaction"},
               {"params", {sig, {{"encoding", "jsonParsed"}, {"maxSupportedTransactionVersion", 0},
                                 {"commitment", "confirmed"}}}}};
    auto tr = http_.post_json(rpc_, tq.dump());
    if (!tr.ok()) continue;
    json t = json::parse(tr.body, nullptr, false);
    if (!t.is_object() || !t["result"].is_object()) continue;
    const json& meta = t["result"]["meta"];
    std::map<std::string, double> delta;
    auto add = [&](const json& arr, double sign) {
      if (!arr.is_array()) return;
      for (const auto& b : arr) {
        if (b.value("owner", "") != wallet) continue;
        const json& ui = b["uiTokenAmount"];
        double amt = ui.contains("uiAmount") && ui["uiAmount"].is_number() ? ui["uiAmount"].get<double>() : 0;
        delta[b.value("mint", "")] += sign * amt;
      }
    };
    add(meta["preTokenBalances"], -1);
    add(meta["postTokenBalances"], +1);
    for (const auto& [mint, d] : delta) {
      if (d <= 0 || mint == kWsolMint || mint == kUsdcMint || mint == kUsdtMint) continue;
      if (is_solana_address(mint)) mints.push_back(mint);
    }
  }
  return mints;
}

std::vector<Lead> SmartMoney::discover(int64_t now) {
  std::vector<Lead> out;
  const auto wallets = scout_.snapshot();
  if (now < next_ || wallets.empty()) return out;
  next_ = now + every_ms_;
  for (const auto& w : wallets) {
    if (w.muted) continue;
    for (const auto& m : new_buys(w.address)) {
      out.push_back(Lead{m, "smart:" + w.label, 0, w.address});
      scout_.record_signal(w.address);
      log::info(name(), w.label + " bought " + m);
    }
  }
  count_run();
  return out;
}

}  // namespace fomo
