// Agents 18-20: Executor, ExitManager, Accountant (books + filter audit).
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

#include "agents/agent.hpp"
#include "core/log.hpp"
#include "core/parse.hpp"
#include "core/util.hpp"

namespace fomo {

using nlohmann::json;

// --------------------------------------------------------------- 18 Executor
Executor::Executor(Http& http, const Config& c, FeeModel fees)
    : Agent(18, "Executor", "places $1 buys and the sells; paper mode by default"),
      http_(http),
      fees_(fees),
      paper_(c.str("mode", "paper") != "live"),
      slippage_bps_(c.integer("slippage_bps", 1500)) {}

std::optional<Position> Executor::buy(const Candidate& c, double size_usd, double sol_usd) {
  if (!paper_) {
    // Live trading needs a funded burner wallet + signer service (phase 2).
    fail("live mode not enabled in this build", false);
    return std::nullopt;
  }
  if (!c.snap || !c.buy_quote || !c.buy_quote->ok || sol_usd <= 0) {
    fail("missing quote", false);
    return std::nullopt;
  }
  const int decimals = c.safety ? c.safety->decimals : 6;
  // Paper fill = the real Jupiter quote, minus drift between quote and landing.
  const double tokens = c.buy_quote->out_amount / std::pow(10.0, decimals) * (1.0 - fees_.extra_slippage_pct / 100.0);
  if (tokens <= 0) return std::nullopt;
  const double tx = fees_.tx_cost_usd(sol_usd);
  const double platform = fees_.platform_fee_usd(size_usd);
  Position p;
  p.mint = c.mint;
  p.symbol = c.snap->symbol;
  p.pair = c.snap->pair;
  p.opened_ms = now_ms();
  p.size_usd = size_usd;
  p.fees_usd = tx + platform;
  p.cost_usd = size_usd + tx + platform;
  p.rent_usd = fees_.rent_usd(sol_usd);
  p.tokens_initial = p.tokens_left = tokens;
  p.decimals = decimals;
  p.entry_price = size_usd / tokens;  // effective price we paid per token (ex fees)
  p.screen_price = c.snap->price_usd;
  p.last_price = p.entry_price;
  // Mark at the honeypot test's real sell quote, so the position starts at
  // what we could actually get back (usually a few % below entry).
  if (c.sell_quote && c.sell_quote->ok && c.buy_quote->out_amount > 0) {
    const double sell_px = c.sell_quote->out_amount / 1e9 * sol_usd / (c.buy_quote->out_amount / std::pow(10.0, decimals));
    if (sell_px > 0) {
      p.last_price = sell_px;
      if (p.screen_price > 0) p.quote_ratio = sell_px / p.screen_price;
      p.quote_ms = now_ms();
    }
  }
  p.peak_price = p.last_price;
  p.entry_liquidity = p.last_liquidity = c.snap->liquidity_usd;
  p.entry_score = c.score;
  p.source = c.sources.empty() ? "" : c.sources.front();
  p.smart_wallet = c.smart_wallet;
  pass(0, "bought");
  return p;
}

Quote Executor::sell_quote(const Position& p, double tokens) {
  char amt[64];
  std::snprintf(amt, sizeof amt, "%.0f", tokens * std::pow(10.0, p.decimals));
  auto r = http_.get("https://lite-api.jup.ag/swap/v1/quote?slippageBps=" + std::to_string(slippage_bps_) +
                     "&inputMint=" + p.mint + "&outputMint=" + kWsolMint + "&amount=" + amt);
  return parse::jupiter_quote(r.body);
}

bool Executor::mark(Position& p, double sol_usd) {
  if (p.closed || p.tokens_left <= 0 || sol_usd <= 0) return false;
  const Quote q = sell_quote(p, p.tokens_left);
  if (!q.ok) return false;
  p.last_price = q.out_amount / 1e9 * sol_usd / p.tokens_left;
  if (p.screen_price > 0) p.quote_ratio = p.last_price / p.screen_price;
  p.quote_ms = now_ms();
  return true;
}

double Executor::sell(Position& p, double fraction_of_initial, double sol_usd, const std::string& why) {
  double tokens = std::min(p.tokens_left, p.tokens_initial * fraction_of_initial);
  if (p.tokens_left - tokens < p.tokens_initial * 1e-6) tokens = p.tokens_left;  // no dust
  if (tokens <= 0) return 0;
  double gross = 0;
  // Real exit price: ask Jupiter what this many tokens fetch right now.
  const Quote q = sell_quote(p, tokens);
  if (q.ok && sol_usd > 0) {
    gross = q.out_amount / 1e9 * sol_usd;
  } else {
    // No route: a pulled pool is worth ~0; otherwise assume a bad fill.
    gross = p.last_liquidity < 1000 ? 0.0 : tokens * p.last_price * 0.5;
    log::warn(name(), "no sell quote for " + p.symbol + (gross > 0 ? ", assuming 50% haircut" : ", pool gone: $0"));
  }
  gross *= (1.0 - fees_.extra_slippage_pct / 100.0);
  const double tx = fees_.tx_cost_usd(sol_usd);
  const double platform = fees_.platform_fee_usd(gross);
  double net = std::max(0.0, gross - tx - platform);
  p.tokens_left -= tokens;
  p.fees_usd += tx + platform;
  const bool closing = p.tokens_left <= 0;
  if (closing) {
    p.tokens_left = 0;
    // Closing the token account refunds its rent. That is our own deposit
    // coming back, not profit: the Accountant credits it on close.
    p.closed = true;
    p.closed_ms = now_ms();
    p.close_reason = why;
  }
  p.realized_usd += net;
  pass(0, why);
  return net;
}

// ------------------------------------------------------------ 19 ExitManager
ExitManager::ExitManager(const Config& c)
    : Agent(19, "ExitManager", "take-profit ladder, trailing stop, stop-loss, time stop"),
      stop_loss_mult_(c.num("stop_loss_multiple", 0.5)),
      trail_from_peak_(c.num("trail_from_peak_pct", 30) / 100.0),
      trail_arm_mult_(c.num("trail_arm_multiple", 1.5)),
      time_stop_min_(c.num("time_stop_minutes", 5)),
      time_stop_min_mult_(c.num("time_stop_min_multiple", 1.15)),
      max_hold_min_(c.num("max_hold_minutes", 360)),
      rug_liq_drop_(c.num("rug_liquidity_drop_pct", 50) / 100.0) {
  // exit_mode:
  //   trail  (default) no fixed targets; trailing stop armed at trail_arm_multiple.
  //          Best expected value in the research simulation.
  //   fixed  sell 100% at +take_profit_pct  ("jitne % pe profit, utne pe nikalo")
  //   ladder sell slices at tp_ladder = multiple:fraction,... then trail the rest
  const std::string mode = c.str("exit_mode", "trail");
  if (mode == "fixed") {
    ladder_ = {{1.0 + c.num("take_profit_pct", 100) / 100.0, 1.0}};
  } else if (mode == "ladder") {
    ladder_ = c.pairs("tp_ladder");
    if (ladder_.empty()) ladder_ = {{2.0, 0.5}, {5.0, 0.25}, {10.0, 0.15}};
  }
  std::sort(ladder_.begin(), ladder_.end());
}

ExitManager::Action ExitManager::evaluate(const Position& p, int64_t now) const {
  if (p.closed || p.tokens_left <= 0) return {};
  count_run();
  const double m = p.multiple();
  const double held_min = static_cast<double>(now - p.opened_ms) / 60000.0;
  const double all = 1.0;  // fraction_of_initial >= remaining closes the position
  // 1. Rug: liquidity pulled.
  if (p.entry_liquidity > 0 && p.last_liquidity < p.entry_liquidity * (1.0 - rug_liq_drop_))
    return {all, "rug exit: liquidity -" + std::to_string(static_cast<int>(rug_liq_drop_ * 100)) + "%"};
  // 2. Hard stop-loss.
  if (m > 0 && m <= stop_loss_mult_) return {all, "stop-loss"};
  // 3. Take-profit ladder, one rung per evaluation.
  if (p.ladder_step < ladder_.size() && m >= ladder_[p.ladder_step].first) {
    char why[64];
    std::snprintf(why, sizeof why, "take-profit %.0fx", ladder_[p.ladder_step].first);
    return {ladder_[p.ladder_step].second, why};
  }
  // 4. Trailing stop once armed (protects the moon-bag).
  if (p.peak_multiple() >= trail_arm_mult_ && p.last_price <= p.peak_price * (1.0 - trail_from_peak_))
    return {all, "trailing stop"};
  // 5. Time stop: dead money.
  if (held_min >= time_stop_min_ && p.peak_multiple() < time_stop_min_mult_) return {all, "time stop"};
  // 6. Max hold.
  if (held_min >= max_hold_min_) return {all, "max hold time"};
  return {};
}

// ------------------------------------------------------------- 20 Accountant
Accountant::Accountant(const Config& c, std::string data_dir)
    : Agent(20, "Auditor", "books every trade and tests the filters on rejected coins"),
      dir_(std::move(data_dir)),
      balance_(c.num("paper_balance_usd", 100)),
      max_audit_(static_cast<size_t>(c.integer("audit_max_tracked", 400))),
      audit_every_ms_(c.integer("audit_every_s", 300) * 1000) {}

void Accountant::write_ledger(const std::string& json_line) {
  std::ofstream f(dir_ + "/ledger.jsonl", std::ios::app);
  f << json_line << "\n";
}

void Accountant::on_buy(const Position& p) {
  balance_ -= p.cost_usd + p.rent_usd;
  buy_times_.push_back(p.opened_ms);
  write_ledger(json{{"t", p.opened_ms}, {"event", "buy"}, {"mint", p.mint}, {"symbol", p.symbol},
                    {"usd", p.size_usd}, {"fees", p.fees_usd}, {"rent", p.rent_usd}, {"price", p.entry_price},
                    {"tokens", p.tokens_initial}, {"score", p.entry_score}, {"source", p.source}}
                   .dump());
  count_run();
}

void Accountant::on_sell(const Position& p, double usd, const std::string& why, double fraction) {
  balance_ += usd;
  write_ledger(json{{"t", now_ms()}, {"event", "sell"}, {"mint", p.mint}, {"symbol", p.symbol},
                    {"usd", usd}, {"fraction", fraction}, {"why", why}, {"multiple", p.multiple()},
                    {"peak_multiple", p.peak_multiple()}}
                   .dump());
}

void Accountant::on_close(const Position& p) {
  balance_ += p.rent_usd;  // token-account rent refunded
  day_pnl_ += p.pnl_usd();
  write_ledger(json{{"t", p.closed_ms}, {"event", "close"}, {"mint", p.mint}, {"symbol", p.symbol},
                    {"pnl", p.pnl_usd()}, {"realized", p.realized_usd}, {"cost", p.cost_usd},
                    {"peak_multiple", p.peak_multiple()}, {"why", p.close_reason},
                    {"held_min", static_cast<double>(p.closed_ms - p.opened_ms) / 60000.0}}
                   .dump());
}

int Accountant::trades_last_hour(int64_t now) const {
  return static_cast<int>(std::count_if(buy_times_.begin(), buy_times_.end(),
                                        [&](int64_t t) { return now - t < 3600000LL; }));
}

void Accountant::track_reject(const Candidate& c, int64_t now) {
  if (!c.snap || c.snap->price_usd <= 0 || rejected_.count(c.mint)) return;
  if (rejected_.size() >= max_audit_) return;
  Rejected r;
  r.mint = c.mint;
  r.symbol = c.snap->symbol;
  r.agent = c.reject_agent;
  r.reason = c.reject_reason;
  r.price0 = c.snap->price_usd;
  r.max_mult = r.last_mult = 1.0;
  r.last_liq = c.snap->liquidity_usd;
  r.t0 = now;
  r.next_check = now + audit_every_ms_;
  rejected_[c.mint] = r;
}

std::vector<std::string> Accountant::audit_mints_due(int64_t now) const {
  std::vector<std::string> out;
  for (const auto& [m, r] : rejected_)
    if (now >= r.next_check && r.checks < 12) out.push_back(m);
  return out;
}

void Accountant::audit_update(const std::map<std::string, PairSnapshot>& snaps,
                              const std::vector<std::string>& checked, int64_t now) {
  for (const auto& m : checked) {
    auto rit = rejected_.find(m);
    if (rit == rejected_.end()) continue;
    auto& r = rit->second;
    if (now < r.next_check || r.checks >= 12) continue;
    r.next_check = now + audit_every_ms_;
    r.checks++;
    auto it = snaps.find(m);
    if (it == snaps.end()) {
      r.last_mult = 0;  // pair gone = dead
      r.last_liq = 0;
      continue;
    }
    r.last_mult = it->second.price_usd / r.price0;
    r.last_liq = it->second.liquidity_usd;
    r.max_mult = std::max(r.max_mult, r.last_mult);
  }
}

std::string Accountant::audit_json() const {
  json by_agent = json::object();
  json rows = json::array();
  for (const auto& [m, r] : rejected_) {
    if (r.checks == 0) continue;
    auto& a = by_agent[r.agent];
    if (a.is_null()) a = {{"rejected", 0}, {"later_2x", 0}, {"later_dead", 0}};
    a["rejected"] = a["rejected"].get<int>() + 1;
    if (r.max_mult >= 2.0) a["later_2x"] = a["later_2x"].get<int>() + 1;
    if (r.last_mult < 0.2 || r.last_liq < 1000) a["later_dead"] = a["later_dead"].get<int>() + 1;
    rows.push_back({{"mint", r.mint}, {"symbol", r.symbol}, {"agent", r.agent}, {"reason", r.reason},
                    {"max_multiple_seen", r.max_mult}, {"last_multiple", r.last_mult}, {"checks", r.checks}});
  }
  return json{{"by_agent", by_agent}, {"coins", rows}}.dump(2);
}

std::string Accountant::audit_table() const {
  json j = json::parse(audit_json());
  std::ostringstream o;
  for (auto it = j["by_agent"].begin(); it != j["by_agent"].end(); ++it) {
    char line[160];
    std::snprintf(line, sizeof line, "  %-16s rejected=%-4d later 2x+=%-3d later dead=%-4d\n", it.key().c_str(),
                  it.value()["rejected"].get<int>(), it.value()["later_2x"].get<int>(),
                  it.value()["later_dead"].get<int>());
    o << line;
  }
  return o.str();
}

}  // namespace fomo
