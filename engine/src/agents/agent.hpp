#pragma once
// The 20 FOMO Family agents. Each one has a single job. The orchestrator
// (app/orchestrator.cpp) wires them into one pipeline:
//
//   DISCOVER  1 LaunchScout  2 TrendScout  3 SmartMoney  4 TraderScout
//   OBSERVE   5 MarketData   6 ApiHealth   7 Volatility
//   FILTER    8 LiquidityGuard  9 SocialCheck  10 Momentum
//             11 RugGuard  12 HolderAnalyst  13 HoneypotTester  14 OrganicFlow
//   DECIDE    15 NarrativeAI  16 Strategist  17 RiskGovernor (OCaml)
//   ACT       18 Executor  19 ExitManager
//   LEARN     20 Accountant & FilterAuditor (books + tests the filters)
//
// Cheap checks run first; network-heavy checks only run on survivors.
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "core/config.hpp"
#include "core/fees.hpp"
#include "core/http.hpp"
#include "core/types.hpp"

namespace fomo {

struct AgentStats {
  long runs = 0, passes = 0, fails = 0;
};

class Agent {
 public:
  Agent(int id, std::string name, std::string job) : id_(id), name_(std::move(name)), job_(std::move(job)) {}
  virtual ~Agent() = default;
  int id() const { return id_; }
  const std::string& name() const { return name_; }
  const std::string& job() const { return job_; }
  AgentStats stats() const { return {runs_.load(), passes_.load(), fails_.load()}; }

 protected:
  Verdict pass(double score, std::string why = "") {
    runs_++;
    passes_++;
    return Verdict{name_, true, false, score, std::move(why)};
  }
  Verdict fail(std::string why, bool hard, double score = 0) {
    runs_++;
    fails_++;
    return Verdict{name_, false, hard, score, std::move(why)};
  }
  void count_run() { runs_++; }

 private:
  int id_;
  std::string name_, job_;
  std::atomic<long> runs_{0}, passes_{0}, fails_{0};
};

// ---------------------------------------------------------------- DISCOVER

// 1. New pools/launches (GeckoTerminal new_pools, Dexscreener latest profiles).
class LaunchScout : public Agent {
 public:
  LaunchScout(Http& http, const Config& c);
  std::vector<Lead> discover(int64_t now);

 private:
  Http& http_;
  int gecko_every_ms_, profiles_every_ms_;
  int64_t next_gecko_ = 0, next_profiles_ = 0;
};

// 2. Coins people are paying to promote / trending (Dexscreener boosts).
class TrendScout : public Agent {
 public:
  TrendScout(Http& http, const Config& c);
  std::vector<Lead> discover(int64_t now);

 private:
  Http& http_;
  int every_ms_;
  int64_t next_ = 0;
};

// 4. Curates the copy-trade wallet list and scores wallets by how their
//    signals actually performed (feedback from Accountant/FilterAuditor).
struct TrackedWallet {
  std::string label, address, origin;
  int signals = 0, winners = 0, losers = 0;
  bool muted = false;
};

class TraderScout : public Agent {
 public:
  TraderScout(const Config& c);
  void load(const std::string& csv_path);
  std::vector<TrackedWallet> snapshot() const;  // thread-safe copy
  void record_signal(const std::string& wallet);
  void record_outcome(const std::string& wallet, double peak_multiple);
  std::string leaderboard_json() const;

 private:
  mutable std::mutex mu_;
  std::vector<TrackedWallet> wallets_;
  int mute_after_signals_;
  double mute_below_hit_rate_;
};

// 3. Watches tracked wallets on-chain and turns their fresh buys into leads.
class SmartMoney : public Agent {
 public:
  SmartMoney(Http& http, const Config& c, TraderScout& scout);
  std::vector<Lead> discover(int64_t now);

 private:
  std::vector<std::string> new_buys(const std::string& wallet);
  Http& http_;
  TraderScout& scout_;
  std::string rpc_;
  int every_ms_;
  int64_t next_ = 0;
  std::map<std::string, std::string> last_sig_;
};

// ----------------------------------------------------------------- OBSERVE

// 5. Batch price/liquidity/flow snapshots + SOL price.
class MarketData : public Agent {
 public:
  MarketData(Http& http, const Config& c);
  std::map<std::string, PairSnapshot> fetch(const std::vector<std::string>& mints, int max_calls);
  double sol_usd(int64_t now);

 private:
  Http& http_;
  double sol_usd_ = 0;
  int64_t sol_next_ = 0;
};

// 6. Health of every API: latency, error rate, 429s. Blocks new entries
//    when the data needed to manage open positions is unreliable.
class ApiHealth : public Agent {
 public:
  ApiHealth(Http& http, const Config& c);
  bool entries_allowed();
  std::string table() const;

 private:
  Http& http_;
  double max_error_rate_;
  std::map<std::string, HostStats> last_;
};

// 7. Realised volatility per coin from our own snapshot history.
class Volatility : public Agent {
 public:
  explicit Volatility(const Config& c);
  void observe(const PairSnapshot& s);
  double vol_per_min(const std::string& mint) const;  // stdev of 1-min log returns
  Verdict check(const Candidate& c);

 private:
  std::map<std::string, std::vector<std::pair<int64_t, double>>> hist_;
  double max_vol_;
};

// ------------------------------------------------------------------ FILTER

// 8. Enough liquidity for a $1 round trip, right age window, sane FDV/liquidity.
class LiquidityGuard : public Agent {
 public:
  explicit LiquidityGuard(const Config& c);
  Verdict check(const Candidate& c, int64_t now);

 private:
  double min_liq_, dead_liq_, max_fdv_liq_, min_mcap_, max_mcap_;
  double min_age_min_, max_age_min_;
  bool curve_ok_;
};

// 9. Socials present, and not a copy-cat of a ticker launched many times.
class SocialCheck : public Agent {
 public:
  explicit SocialCheck(const Config& c);
  void observe_symbol(const std::string& symbol, const std::string& mint, int64_t now);
  Verdict check(const Candidate& c);

 private:
  bool require_socials_;
  int copycat_limit_;
  std::map<std::string, std::map<std::string, int64_t>> symbol_mints_;
};

// 10. Buyer pressure and volume velocity in the last 5 minutes.
class Momentum : public Agent {
 public:
  explicit Momentum(const Config& c);
  Verdict check(const Candidate& c);

 private:
  int min_buys_m5_;
  double min_ratio_, min_chg_m5_, max_chg_m5_, min_turnover_;
};

// 11. Contract safety: authorities, Token-2022 traps, RugCheck dangers.
class RugGuard : public Agent {
 public:
  explicit RugGuard(const Config& c);
  // dex = the pair's dexId; bonding-curve coins ("pumpfun") have no LP to lock.
  Verdict check(const SafetyReport& r, const std::string& dex);

 private:
  double max_rug_score_, min_lp_locked_;
  int max_danger_;
};

// 12. Who holds it: concentration, insiders, serial-launcher creators.
class HolderAnalyst : public Agent {
 public:
  explicit HolderAnalyst(const Config& c);
  Verdict check(const SafetyReport& r);

 private:
  double max_top10_, max_single_, max_creator_pct_;
  int max_insiders_, max_creator_tokens_, min_holders_;
};

// 13. Can we actually sell? Jupiter Shield + buy->sell round-trip quote.
class HoneypotTester : public Agent {
 public:
  HoneypotTester(Http& http, const Config& c);
  // Fills c.buy_quote / c.sell_quote; returns verdict.
  Verdict test(Candidate& c, double sol_usd, double size_usd);
  std::map<std::string, std::vector<std::string>> shield(const std::vector<std::string>& mints);
  Verdict check_shield(const std::vector<std::string>& warnings);

 private:
  Http& http_;
  int slippage_bps_;
  double max_roundtrip_loss_pct_, max_impact_pct_;
};

// 14. Real buyers or bots? Distinct-trader growth and Jupiter organic score.
class OrganicFlow : public Agent {
 public:
  OrganicFlow(Http& http, const Config& c);
  struct Facts {
    bool ok = false;
    double organic_score = 0;      // Jupiter 0-100
    double organic_buy_share = 0;  // buyOrganicVolume / buyVolume (5m), 0-1
    int traders_5m = 0, net_buyers_5m = 0;
    int dev_mints = 0;
    double top_holders_pct = 0;
  };
  // One call for up to 50 mints (Jupiter Tokens v2 search).
  std::map<std::string, Facts> fetch(const std::vector<std::string>& mints);
  Verdict check(const Facts& f);

 private:
  Http& http_;
  double min_organic_score_, min_organic_share_;
  int min_traders_5m_, max_dev_mints_;
};

// ------------------------------------------------------------------ DECIDE

// 15. Optional AI judgement (Jev or any OpenAI-compatible model). Off without a key.
class NarrativeAI : public Agent {
 public:
  NarrativeAI(Http& http, const Config& c);
  bool enabled() const { return enabled_; }
  Verdict judge(const Candidate& c);

 private:
  Http& http_;
  bool enabled_ = false;
  std::string url_, model_, key_;
};

// 16. Adds up every verdict into one 0-100 entry score and decides.
class Strategist : public Agent {
 public:
  explicit Strategist(const Config& c);
  double entry_score() const { return entry_score_; }
  // true = buy. Sets c.score.
  bool decide(Candidate& c);

 private:
  double entry_score_, smart_bonus_, boost_bonus_;
};

// 17. OCaml risk daemon (risk/bin/riskd.ml). Fails closed: no answer = no trade.
class RiskGovernor : public Agent {
 public:
  RiskGovernor(const Config& c);
  ~RiskGovernor() override;
  struct Request {
    double size = 1, deployed = 0, day_pnl = 0, balance = 0, score = 0, liq = 0;
    int open = 0, trades_hour = 0;
    bool holding = false;
  };
  // returns "" if approved, else the reason
  std::string approve(const Request& r);
  bool alive() const { return pid_ > 0; }

 private:
  bool start();
  void stop();
  std::string path_, args_;
  int pid_ = -1, to_child_ = -1, from_child_ = -1;
  long next_id_ = 1;
};

// -------------------------------------------------------------------- ACT

// 18. Places orders. Paper mode fills at real Jupiter quotes minus modelled
//     costs. Live mode is not wired yet (needs a funded burner wallet).
class Executor : public Agent {
 public:
  Executor(Http& http, const Config& c, FeeModel fees);
  bool paper() const { return paper_; }
  std::optional<Position> buy(const Candidate& c, double size_usd, double sol_usd);
  // Sells `fraction` of the ORIGINAL token amount (clamped to what is left).
  // Returns net USD received.
  double sell(Position& p, double fraction_of_initial, double sol_usd, const std::string& why);

 private:
  Http& http_;
  FeeModel fees_;
  bool paper_ = true;
  int slippage_bps_;
};

// 19. Exit rules: take-profit ladder, trailing stop, stop-loss, time stop, rug exit.
class ExitManager : public Agent {
 public:
  explicit ExitManager(const Config& c);
  struct Action {
    double fraction = 0;   // of initial tokens; 0 = nothing to do
    std::string why;
  };
  Action evaluate(const Position& p, int64_t now) const;
  const std::vector<std::pair<double, double>>& ladder() const { return ladder_; }

 private:
  std::vector<std::pair<double, double>> ladder_;  // (multiple, fraction of initial)
  double stop_loss_mult_, trail_from_peak_, trail_arm_mult_;
  double time_stop_min_, time_stop_min_mult_, max_hold_min_, rug_liq_drop_;
};

// 20. Books every fill (ledger.jsonl), tracks equity and daily PnL, and
//     audits the filters: it follows coins we REJECTED and records what
//     happened, so we learn which filters cost us winners or saved us.
class Accountant : public Agent {
 public:
  Accountant(const Config& c, std::string data_dir);
  void on_buy(const Position& p);
  void on_sell(const Position& p, double usd, const std::string& why, double fraction);
  void on_close(const Position& p);
  double balance() const { return balance_; }
  double day_pnl() const { return day_pnl_; }
  int trades_last_hour(int64_t now) const;
  void add_balance(double usd) { balance_ += usd; }

  // filter audit
  void track_reject(const Candidate& c, int64_t now);
  std::vector<std::string> audit_mints_due(int64_t now) const;
  // `checked` = the mints we asked MarketData about; missing from snaps = pair gone.
  void audit_update(const std::map<std::string, PairSnapshot>& snaps, const std::vector<std::string>& checked,
                    int64_t now);
  std::string audit_table() const;
  std::string audit_json() const;

  void write_ledger(const std::string& json_line);
  std::string data_dir() const { return dir_; }

 private:
  struct Rejected {
    std::string mint, symbol, agent, reason;
    double price0 = 0, max_mult = 0, last_mult = 0, last_liq = 0;
    int64_t t0 = 0, next_check = 0;
    int checks = 0;
  };
  std::string dir_;
  double balance_;
  double day_pnl_ = 0;
  std::vector<int64_t> buy_times_;
  std::map<std::string, Rejected> rejected_;
  size_t max_audit_;
  int audit_every_ms_;
};

}  // namespace fomo
