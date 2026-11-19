// Agents 8-14: the filters. Cheap ones (8-10) use the market snapshot only;
// 11-14 call RugCheck / Jupiter and run only on coins that survived 8-10.
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "agents/agent.hpp"
#include "core/log.hpp"
#include "core/parse.hpp"
#include "core/util.hpp"

namespace fomo {

using nlohmann::json;

namespace {
std::string f1(const char* fmt, double a) {
  char b[128];
  std::snprintf(b, sizeof b, fmt, a);
  return b;
}
std::string f2(const char* fmt, double a, double b2) {
  char b[160];
  std::snprintf(b, sizeof b, fmt, a, b2);
  return b;
}
}  // namespace

// --------------------------------------------------------- 8 LiquidityGuard
LiquidityGuard::LiquidityGuard(const Config& c)
    : Agent(8, "LiquidityGuard", "enough liquidity, right age and size window"),
      min_liq_(c.num("liq_min_usd", 8000)),
      dead_liq_(c.num("liq_dead_usd", 1000)),
      max_fdv_liq_(c.num("liq_max_fdv_ratio", 40)),
      min_mcap_(c.num("mcap_min_usd", 10000)),
      max_mcap_(c.num("mcap_max_usd", 5000000)),
      min_age_min_(c.num("age_min_minutes", 3)),
      max_age_min_(c.num("age_max_minutes", 1440)),
      curve_ok_(c.flag("bonding_curve_ok", false)) {}

Verdict LiquidityGuard::check(const Candidate& c, int64_t now) {
  if (!c.snap) return fail("no market data", false);
  const auto& s = *c.snap;
  const double age_min = s.pair_created_ms > 0 ? static_cast<double>(now - s.pair_created_ms) / 60000.0 : -1;
  if (age_min > max_age_min_) return fail(f1("too old (%.0f min)", age_min), true);
  if (is_bonding_curve_dex(s.dex) && s.liquidity_usd <= 0) {
    if (!curve_ok_) return fail("still on bonding curve (waiting for graduation)", false);
    return pass(0, "bonding curve; HoneypotTester checks it can be sold");
  }
  if (s.liquidity_usd < dead_liq_) return fail(f1("dead pool ($%.0f liquidity)", s.liquidity_usd), true);
  if (age_min >= 0 && age_min < min_age_min_) return fail(f1("too new (%.1f min), wait for bundles to clear", age_min), false);
  if (s.liquidity_usd < min_liq_) return fail(f1("liquidity $%.0f below min", s.liquidity_usd), false);
  if (s.mcap < min_mcap_) return fail(f1("market cap $%.0f below min", s.mcap), false);
  if (s.mcap > max_mcap_) return fail(f1("market cap $%.0f above max", s.mcap), false);
  const double ratio = s.liquidity_usd > 0 ? s.fdv / s.liquidity_usd : 1e9;
  if (ratio > max_fdv_liq_) return fail(f1("FDV/liquidity %.0fx too thin", ratio), false);
  const double score = std::min(10.0, std::log10(std::max(1.0, s.liquidity_usd / min_liq_)) * 10.0);
  return pass(score, f2("liq $%.0f, mcap $%.0f", s.liquidity_usd, s.mcap));
}

// ----------------------------------------------------------- 9 SocialCheck
SocialCheck::SocialCheck(const Config& c)
    : Agent(9, "SocialCheck", "socials present, not a copy-cat ticker"),
      require_socials_(c.flag("social_required", true)),
      copycat_limit_(c.integer("social_copycat_limit", 3)) {}

void SocialCheck::observe_symbol(const std::string& symbol, const std::string& mint, int64_t now) {
  if (symbol.empty()) return;
  std::string key = symbol;
  std::transform(key.begin(), key.end(), key.begin(), [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
  auto& m = symbol_mints_[key];
  m[mint] = now;
  for (auto it = m.begin(); it != m.end();) {
    if (now - it->second > 3600000LL) it = m.erase(it);
    else ++it;
  }
}

Verdict SocialCheck::check(const Candidate& c) {
  if (!c.snap) return fail("no market data", false);
  const auto& s = *c.snap;
  const int links = (s.has_website ? 1 : 0) + (s.has_twitter ? 1 : 0) + (s.has_telegram ? 1 : 0);
  if (require_socials_ && links == 0) return fail("no website/X/Telegram", false);
  std::string key = s.symbol;
  std::transform(key.begin(), key.end(), key.begin(), [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
  auto it = symbol_mints_.find(key);
  const int clones = it == symbol_mints_.end() ? 0 : static_cast<int>(it->second.size());
  if (clones > copycat_limit_) return fail("ticker $" + s.symbol + " launched " + std::to_string(clones) + "x in 1h (copy-cats)", false, -10);
  return pass(links * 3.0, std::to_string(links) + " social link(s)");
}

// ------------------------------------------------------------- 10 Momentum
Momentum::Momentum(const Config& c)
    : Agent(10, "Momentum", "buyer pressure and volume speed, last 5 min"),
      min_buys_m5_(c.integer("mom_min_buys_m5", 25)),
      min_ratio_(c.num("mom_min_buy_sell_ratio", 1.1)),
      min_chg_m5_(c.num("mom_min_chg_m5_pct", -15)),
      max_chg_m5_(c.num("mom_max_chg_m5_pct", 120)),
      min_turnover_(c.num("mom_min_turnover_m5", 0.05)) {}

Verdict Momentum::check(const Candidate& c) {
  if (!c.snap) return fail("no market data", false);
  const auto& s = *c.snap;
  if (s.buys_m5 < min_buys_m5_) return fail(std::to_string(s.buys_m5) + " buys in 5m (quiet)", false);
  const double ratio = static_cast<double>(s.buys_m5) / std::max(1, s.sells_m5);
  if (ratio < min_ratio_) return fail(f1("buy/sell %.2f (sellers winning)", ratio), false);
  if (s.chg_m5 < min_chg_m5_) return fail(f1("falling %.0f%% in 5m", s.chg_m5), false);
  if (s.chg_m5 > max_chg_m5_) return fail(f1("already +%.0f%% in 5m (don't chase)", s.chg_m5), false);
  const double turnover = s.liquidity_usd > 0 ? s.vol_m5 / s.liquidity_usd : 0;
  if (turnover < min_turnover_) return fail(f1("5m volume only %.2fx liquidity", turnover), false);
  const double score = std::min(12.0, (ratio - 1.0) * 10.0) + std::min(8.0, turnover * 4.0) +
                       (s.chg_m5 > 0 ? std::min(5.0, s.chg_m5 / 10.0) : 0.0);
  return pass(score, f2("buy/sell %.2f, turnover %.2fx", ratio, turnover));
}

// -------------------------------------------------------------- 11 RugGuard
RugGuard::RugGuard(const Config& c)
    : Agent(11, "RugGuard", "contract traps: authorities, Token-2022, RugCheck dangers"),
      max_rug_score_(c.num("rug_max_score_normalised", 50)),
      min_lp_locked_(c.num("rug_min_lp_locked_pct", 90)),
      max_danger_(c.integer("rug_max_danger_risks", 0)) {}

Verdict RugGuard::check(const SafetyReport& r, const std::string& dex) {
  if (!r.fetched) return fail("no RugCheck report", false);
  if (r.rugged) return fail("RugCheck marks it rugged", true);
  if (r.mint_authority_set) return fail("mint authority active (can print supply)", true);
  if (r.freeze_authority_set) return fail("freeze authority active (can freeze you)", true);
  if (r.permanent_delegate) return fail("Token-2022 permanent delegate (can take tokens)", true);
  if (r.non_transferable || r.default_frozen) return fail("Token-2022 transfer lock", true);
  if (r.transfer_hook) return fail("Token-2022 transfer hook", true);
  if (r.transfer_fee) return fail(f1("transfer fee %.1f%%", r.transfer_fee_pct), true);
  if (r.danger_risks > max_danger_) {
    std::string names;
    for (const auto& n : r.risk_names) names += (names.empty() ? "" : "; ") + n;
    return fail("RugCheck danger: " + names, true);
  }
  if (r.score_normalised > max_rug_score_) return fail(f1("RugCheck risk score %.0f", r.score_normalised), true);
  if (!is_bonding_curve_dex(dex) && r.lp_locked_pct < min_lp_locked_)
    return fail(f1("only %.0f%% of LP locked/burned", r.lp_locked_pct), true);
  double score = 6.0 - r.warn_risks * 2.0 - (r.mutable_metadata ? 3.0 : 0.0);
  return pass(score, std::to_string(r.warn_risks) + " warning(s), LP " + f1("%.0f%%", r.lp_locked_pct));
}

// --------------------------------------------------------- 12 HolderAnalyst
HolderAnalyst::HolderAnalyst(const Config& c)
    : Agent(12, "HolderAnalyst", "holder concentration, insiders, serial launchers"),
      max_top10_(c.num("hold_max_top10_pct", 30)),
      max_single_(c.num("hold_max_single_pct", 10)),
      max_creator_pct_(c.num("hold_max_creator_pct", 5)),
      max_insiders_(c.integer("hold_max_insiders", 2)),
      max_creator_tokens_(c.integer("hold_max_creator_tokens", 3)),
      min_holders_(c.integer("hold_min_holders", 50)) {}

Verdict HolderAnalyst::check(const SafetyReport& r) {
  if (!r.fetched) return fail("no holder data", false);
  if (r.max_holder_pct_ex_pools > max_single_) return fail(f1("one wallet holds %.1f%%", r.max_holder_pct_ex_pools), true);
  if (r.top10_pct_ex_pools > max_top10_) return fail(f1("top-10 wallets hold %.1f%%", r.top10_pct_ex_pools), true);
  if (r.creator_pct > max_creator_pct_) return fail(f1("creator still holds %.1f%%", r.creator_pct), true);
  if (r.insider_holders > max_insiders_ || r.graph_insiders > max_insiders_ * 5)
    return fail("insider wallets: " + std::to_string(r.insider_holders) + " top / " +
                    std::to_string(r.graph_insiders) + " network", true);
  if (r.creator_tokens > max_creator_tokens_)
    return fail("creator launched " + std::to_string(r.creator_tokens) + " other coins (serial launcher)", true);
  if (r.total_holders < min_holders_) return fail(std::to_string(r.total_holders) + " holders (too few)", false);
  const double score = std::max(0.0, 10.0 - r.top10_pct_ex_pools / 3.0);
  return pass(score, f2("top10 %.1f%%, %.0f holders", r.top10_pct_ex_pools, r.total_holders));
}

// -------------------------------------------------------- 13 HoneypotTester
HoneypotTester::HoneypotTester(Http& http, const Config& c)
    : Agent(13, "HoneypotTester", "proves we can sell: Shield + round-trip quote"),
      http_(http),
      slippage_bps_(c.integer("slippage_bps", 1500)),
      max_roundtrip_loss_pct_(c.num("honeypot_max_roundtrip_loss_pct", 12)),
      max_impact_pct_(c.num("honeypot_max_price_impact_pct", 5)) {}

std::map<std::string, std::vector<std::string>> HoneypotTester::shield(const std::vector<std::string>& mints) {
  std::map<std::string, std::vector<std::string>> out;
  if (mints.empty()) return out;
  std::string csv;
  for (const auto& m : mints) csv += (csv.empty() ? "" : ",") + m;
  auto r = http_.get("https://lite-api.jup.ag/ultra/v1/shield?mints=" + csv);
  if (!r.ok()) return out;
  json j = json::parse(r.body, nullptr, false);
  if (!j.is_object() || !j["warnings"].is_object()) return out;
  for (auto it = j["warnings"].begin(); it != j["warnings"].end(); ++it) {
    auto& v = out[it.key()];
    if (!it.value().is_array()) continue;
    for (const auto& w : it.value()) v.push_back(w.value("type", "") + ":" + w.value("severity", ""));
  }
  return out;
}

Verdict HoneypotTester::check_shield(const std::vector<std::string>& warnings) {
  for (const auto& w : warnings) {
    if (w.rfind("NOT_SELLABLE", 0) == 0) return fail("Jupiter Shield: NOT_SELLABLE", true);
    if (w.size() > 9 && w.compare(w.size() - 9, 9, ":critical") == 0) return fail("Jupiter Shield critical: " + w, true);
  }
  return pass(0, std::to_string(warnings.size()) + " info flag(s)");
}

Verdict HoneypotTester::test(Candidate& c, double sol_usd, double size_usd) {
  if (sol_usd <= 0) return fail("no SOL price", false);
  const long long lamports = static_cast<long long>(size_usd / sol_usd * 1e9);
  const std::string base = "https://lite-api.jup.ag/swap/v1/quote?slippageBps=" + std::to_string(slippage_bps_);
  auto rb = http_.get(base + "&inputMint=" + kWsolMint + "&outputMint=" + c.mint + "&amount=" + std::to_string(lamports));
  Quote buy = parse::jupiter_quote(rb.body);
  c.buy_quote = buy;
  if (!buy.ok) return fail("no buy route (" + buy.error + ")", false);
  char amt[64];
  std::snprintf(amt, sizeof amt, "%.0f", buy.out_amount);
  auto rs = http_.get(base + "&inputMint=" + c.mint + "&outputMint=" + kWsolMint + "&amount=" + amt);
  Quote sell = parse::jupiter_quote(rs.body);
  c.sell_quote = sell;
  if (!sell.ok) return fail("NO SELL ROUTE - honeypot? (" + sell.error + ")", true);
  const double loss_pct = (1.0 - sell.out_amount / static_cast<double>(lamports)) * 100.0;
  if (loss_pct > max_roundtrip_loss_pct_) return fail(f1("round trip loses %.1f%%", loss_pct), true);
  if (buy.price_impact_pct > max_impact_pct_) return fail(f1("price impact %.1f%%", buy.price_impact_pct), false);
  return pass(std::max(0.0, 6.0 - loss_pct), f1("round trip -%.1f%% via ", loss_pct) + buy.route);
}

// ----------------------------------------------------------- 14 OrganicFlow
OrganicFlow::OrganicFlow(Http& http, const Config& c)
    : Agent(14, "OrganicFlow", "real buyers or bots? organic volume and trader count"),
      http_(http),
      min_organic_score_(c.num("organic_min_score", 5)),
      min_organic_share_(c.num("organic_min_buy_share", 0.03)),
      min_traders_5m_(c.integer("organic_min_traders_5m", 20)),
      max_dev_mints_(c.integer("organic_max_dev_mints", 20)) {}

std::map<std::string, OrganicFlow::Facts> OrganicFlow::fetch(const std::vector<std::string>& mints) {
  std::map<std::string, Facts> out;
  if (mints.empty()) return out;
  std::string csv;
  for (size_t i = 0; i < mints.size() && i < 50; ++i) csv += (csv.empty() ? "" : ",") + mints[i];
  auto r = http_.get("https://lite-api.jup.ag/tokens/v2/search?query=" + csv);
  if (!r.ok()) return out;
  json j = json::parse(r.body, nullptr, false);
  if (!j.is_array()) return out;
  auto num = [](const json& o, const char* k) {
    return o.is_object() && o.contains(k) && o[k].is_number() ? o[k].get<double>() : 0.0;
  };
  for (const auto& t : j) {
    Facts f;
    f.ok = true;
    f.organic_score = num(t, "organicScore");
    const json& s5 = t.contains("stats5m") ? t["stats5m"] : json::object();
    const double buy_vol = num(s5, "buyVolume");
    f.organic_buy_share = buy_vol > 0 ? num(s5, "buyOrganicVolume") / buy_vol : 0;
    f.traders_5m = static_cast<int>(num(s5, "numTraders"));
    f.net_buyers_5m = static_cast<int>(num(s5, "numNetBuyers"));
    const json& audit = t.contains("audit") ? t["audit"] : json::object();
    f.dev_mints = static_cast<int>(num(audit, "devMints"));
    f.top_holders_pct = num(audit, "topHoldersPercentage");
    out[t.value("id", "")] = f;
  }
  count_run();
  return out;
}

Verdict OrganicFlow::check(const Facts& f) {
  if (!f.ok) return pass(0, "no Jupiter stats (neutral)");
  if (f.traders_5m < min_traders_5m_) return fail(std::to_string(f.traders_5m) + " traders in 5m", false);
  if (f.organic_score < min_organic_score_ && f.organic_buy_share < min_organic_share_)
    return fail(f2("looks botted: organic score %.0f, organic buys %.1f%%", f.organic_score, f.organic_buy_share * 100), false);
  double score = std::min(10.0, f.organic_score / 8.0) + std::min(5.0, f.organic_buy_share * 20.0);
  if (f.dev_mints > max_dev_mints_) score -= 5;
  return pass(score, f2("organic score %.0f, organic buys %.1f%%", f.organic_score, f.organic_buy_share * 100));
}

}  // namespace fomo
