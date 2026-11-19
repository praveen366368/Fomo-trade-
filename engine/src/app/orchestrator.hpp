#pragma once
// Wires the 20 agents into one loop. See agents/agent.hpp for the roster.
#include <map>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "agents/agent.hpp"

namespace fomo {

class Orchestrator {
 public:
  Orchestrator(const Config& cfg, Http& http, std::string data_dir);
  // Runs until `minutes` elapse (0 = forever) or data/STOP appears.
  void run(double minutes);
  // One pass of the discovery/filter loop (public for tests).
  void cycle(int64_t now);
  // One pass of the position monitor (runs on its own thread in run()).
  void monitor(int64_t now);
  void close_all(const std::string& why);
  std::string roster() const;
  std::string summary_json() const;
  std::string summary_text() const;
  const std::vector<Position>& positions() const { return positions_; }

 private:
  void add_lead(const Lead& l, int64_t now);
  void evaluate_candidates(int64_t now, double sol_usd, bool entries_ok);
  void manage_positions(int64_t now, double sol_usd);
  void write_reports() const;
  void reject(Candidate& c, const Verdict& v, int64_t now);
  int open_count() const;
  double deployed() const;

  Config cfg_;
  Http& http_;
  std::string dir_;
  FeeModel fees_;

  LaunchScout launch_;
  TrendScout trend_;
  TraderScout traders_;
  SmartMoney smart_;
  MarketData market_;
  ApiHealth health_;
  Volatility vol_;
  LiquidityGuard liq_;
  SocialCheck social_;
  Momentum momentum_;
  RugGuard rug_;
  HolderAnalyst holders_;
  HoneypotTester honeypot_;
  OrganicFlow organic_;
  NarrativeAI ai_;
  Strategist strategist_;
  RiskGovernor risk_;
  Executor exec_;
  ExitManager exits_;
  Accountant books_;

  std::map<std::string, Candidate> pool_;   // coins under watch (eval thread only)
  mutable std::mutex mu_;                    // guards positions_, books_, traders_
  std::vector<Position> positions_;
  std::map<std::string, int> missing_;       // consecutive snapshots without the pair
  double stake_usd_;
  int watch_minutes_, deep_per_cycle_, md_calls_per_cycle_, max_pool_;
  int cycle_seconds_, monitor_seconds_;
  bool close_at_end_;
  std::atomic<bool> stop_{false};
  long cycles_ = 0, leads_ = 0, deep_checks_ = 0, buys_ = 0;
  std::map<std::string, long> reject_counts_;
  int64_t started_ms_ = 0;
  int64_t next_report_ = 0;
};

}  // namespace fomo
