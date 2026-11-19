#include "app/orchestrator.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <sstream>
#include <thread>

#include "core/log.hpp"
#include "core/parse.hpp"
#include "core/util.hpp"

namespace fomo {

using nlohmann::json;
namespace fs = std::filesystem;

Orchestrator::Orchestrator(const Config& cfg, Http& http, std::string data_dir)
    : cfg_(cfg),
      http_(http),
      dir_(std::move(data_dir)),
      fees_(FeeModel::from(cfg)),
      launch_(http, cfg),
      trend_(http, cfg),
      traders_(cfg),
      smart_(http, cfg, traders_),
      market_(http, cfg),
      health_(http, cfg),
      vol_(cfg),
      liq_(cfg),
      social_(cfg),
      momentum_(cfg),
      rug_(cfg),
      holders_(cfg),
      honeypot_(http, cfg),
      organic_(http, cfg),
      ai_(http, cfg),
      strategist_(cfg),
      risk_(cfg),
      exec_(http, cfg, fees_),
      exits_(cfg),
      books_(cfg, dir_),
      stake_usd_(cfg.num("stake_usd", 1.0)),
      watch_minutes_(cfg.integer("watch_minutes", 30)),
      deep_per_cycle_(cfg.integer("deep_checks_per_cycle", 4)),
      md_calls_per_cycle_(cfg.integer("market_calls_per_cycle", 6)),
      max_pool_(cfg.integer("max_watch_pool", 400)),
      cycle_seconds_(cfg.integer("cycle_seconds", 10)),
      monitor_seconds_(cfg.integer("monitor_seconds", 4)),
      quotes_per_monitor_(cfg.integer("quotes_per_monitor", 3)),
      quote_fresh_ms_(static_cast<int64_t>(cfg.integer("quote_fresh_seconds", 15)) * 1000),
      close_at_end_(cfg.flag("close_at_end", true)) {
  fs::create_directories(dir_);
  traders_.load(cfg.str("wallets_file", "config/wallets.csv"));
  if (!ai_.enabled()) log::info("NarrativeAI", "AI scoring off (set ai_url + FOMO_AI_KEY to turn on)");
}

std::string Orchestrator::roster() const {
  const Agent* all[] = {&launch_, &trend_,  &smart_,    &traders_, &market_,   &health_,     &vol_,
                        &liq_,    &social_, &momentum_, &rug_,     &holders_,  &honeypot_,   &organic_,
                        &ai_,     &strategist_, &risk_, &exec_,    &exits_,    &books_};
  std::vector<const Agent*> v(std::begin(all), std::end(all));
  std::sort(v.begin(), v.end(), [](const Agent* a, const Agent* b) { return a->id() < b->id(); });
  std::ostringstream o;
  for (const Agent* a : v) {
    char line[200];
    const auto s = a->stats();
    std::snprintf(line, sizeof line, "  %2d %-15s %-58s runs=%-6ld pass=%-6ld fail=%ld\n", a->id(),
                  a->name().c_str(), a->job().c_str(), s.runs, s.passes, s.fails);
    o << line;
  }
  return o.str();
}

void Orchestrator::add_lead(const Lead& l, int64_t now) {
  leads_++;
  if (banned_.count(l.mint)) return;  // hard-rejected once = never again
  auto it = pool_.find(l.mint);
  if (it != pool_.end()) {
    auto& c = it->second;
    if (std::find(c.sources.begin(), c.sources.end(), l.source) == c.sources.end()) c.sources.push_back(l.source);
    c.boost = std::max(c.boost, l.boost);
    if (!l.wallet.empty()) c.smart_wallet = l.wallet;
    return;
  }
  if (static_cast<int>(pool_.size()) >= max_pool_) {
    // Evict the oldest finished (rejected) entry, else the oldest overall.
    auto victim = pool_.end();
    for (auto p = pool_.begin(); p != pool_.end(); ++p) {
      if (p->second.bought) continue;
      if (victim == pool_.end() || (p->second.rejected && !victim->second.rejected) ||
          (p->second.rejected == victim->second.rejected && p->second.first_seen_ms < victim->second.first_seen_ms))
        victim = p;
    }
    if (victim != pool_.end()) pool_.erase(victim);
  }
  Candidate c;
  c.mint = l.mint;
  c.sources = {l.source};
  c.boost = l.boost;
  c.smart_wallet = l.wallet;
  c.first_seen_ms = now;
  pool_[l.mint] = c;
}

void Orchestrator::reject(Candidate& c, const Verdict& v, int64_t now) {
  c.rejected = true;
  c.hard_rejected = v.hard;
  c.reject_agent = v.agent;
  c.reject_reason = v.reason;
  reject_counts_[v.agent]++;
  if (v.hard && banned_.size() < 500000) banned_.insert(c.mint);
  std::lock_guard<std::mutex> lock(mu_);
  books_.track_reject(c, now);
}

int Orchestrator::open_count() const {
  return static_cast<int>(std::count_if(positions_.begin(), positions_.end(), [](const Position& p) { return !p.closed; }));
}

double Orchestrator::deployed() const {
  double d = 0;
  for (const auto& p : positions_)
    if (!p.closed) d += p.size_usd;
  return d;
}

// ------------------------------------------------------------- the loop
void Orchestrator::cycle(int64_t now) {
  cycles_++;
  // DISCOVER
  for (auto* scout : {static_cast<Agent*>(&launch_), static_cast<Agent*>(&trend_), static_cast<Agent*>(&smart_)}) {
    std::vector<Lead> leads;
    if (scout == &launch_) leads = launch_.discover(now);
    else if (scout == &trend_) leads = trend_.discover(now);
    else leads = smart_.discover(now);
    for (const auto& l : leads) add_lead(l, now);
  }
  // OBSERVE: refresh snapshots for live candidates, newest first.
  std::vector<std::pair<int64_t, std::string>> live;
  for (const auto& [m, c] : pool_)
    if (!c.rejected && !c.bought) live.emplace_back(c.first_seen_ms, m);
  std::sort(live.rbegin(), live.rend());
  std::vector<std::string> mints;
  mints.reserve(live.size());
  for (const auto& [t, m] : live) mints.push_back(m);
  auto snaps = market_.fetch(mints, md_calls_per_cycle_);
  for (auto& [m, s] : snaps) {
    auto it = pool_.find(m);
    if (it == pool_.end()) continue;
    it->second.snap = s;
    it->second.symbol = s.symbol;
    vol_.observe(s);
    social_.observe_symbol(s.symbol, m, now);
  }
  const double sol = market_.sol_usd(now);
  bool entries_ok = health_.entries_allowed();
  if (fs::exists(dir_ + "/KILL")) {
    if (entries_ok) log::warn("Orchestrator", "KILL file present: no new buys");
    entries_ok = false;
  }
  evaluate_candidates(now, sol, entries_ok);

  // LEARN: re-price coins we rejected, to grade the filters.
  std::vector<std::string> due;
  {
    std::lock_guard<std::mutex> lock(mu_);
    due = books_.audit_mints_due(now);
  }
  if (due.size() > 120) due.resize(120);
  if (!due.empty()) {
    auto asnaps = market_.fetch(due, 4);
    if (!asnaps.empty()) {
      std::lock_guard<std::mutex> lock(mu_);
      books_.audit_update(asnaps, due, now);
    }
  }
  if (now >= next_report_) {
    next_report_ = now + 60000LL;
    log::info("Orchestrator", summary_text());
    write_reports();
  }
}

void Orchestrator::evaluate_candidates(int64_t now, double sol, bool entries_ok) {
  const int64_t safety_ttl = 600000LL;
  struct Deep {
    Candidate* c;
    double prelim;
  };
  std::vector<Deep> deep;
  for (auto& [m, c] : pool_) {
    if (c.rejected || c.bought) continue;
    if (now - c.first_seen_ms > static_cast<int64_t>(watch_minutes_) * 60000) {
      reject(c, Verdict{"Expired", false, false, 0, c.reject_reason.empty() ? "never qualified" : c.reject_reason}, now);
      continue;
    }
    if (!c.snap || now - c.snap->ts_ms > 90000LL) continue;
    c.verdicts.clear();
    c.last_eval_ms = now;
    bool ok = true;
    // Cheapest first; stop at the first failure.
    const std::function<Verdict()> cheap[] = {[&] { return liq_.check(c, now); }, [&] { return social_.check(c); },
                                              [&] { return momentum_.check(c); }, [&] { return vol_.check(c); }};
    for (const auto& run_check : cheap) {
      const Verdict v = run_check();
      c.verdicts.push_back(v);
      if (!v.pass) {
        ok = false;
        if (v.hard) reject(c, v, now);
        else c.reject_reason = v.agent + ": " + v.reason;  // remembered if it later expires
        break;
      }
    }
    if (!ok) continue;
    double prelim = 50;
    for (const auto& v : c.verdicts) prelim += v.score;
    deep.push_back({&c, prelim});
  }
  if (deep.empty()) return;
  std::sort(deep.begin(), deep.end(), [](const Deep& a, const Deep& b) { return a.prelim > b.prelim; });
  if (static_cast<int>(deep.size()) > deep_per_cycle_) deep.resize(static_cast<size_t>(deep_per_cycle_));

  // Network-heavy checks in parallel: one batch call for Shield + organic
  // stats, and RugCheck + round-trip quotes per coin.
  std::vector<std::string> batch;
  batch.reserve(deep.size());
  for (const auto& d : deep) batch.push_back(d.c->mint);
  auto meta = std::async(std::launch::async, [&] {
    return std::make_pair(honeypot_.shield(batch), organic_.fetch(batch));
  });
  std::vector<std::future<Verdict>> quotes;
  std::vector<std::future<void>> reports;
  for (auto& d : deep) {
    Candidate* c = d.c;
    if (!c->safety || now - c->safety_ms > safety_ttl) {
      reports.push_back(std::async(std::launch::async, [this, c, now] {
        auto r = http_.get("https://api.rugcheck.xyz/v1/tokens/" + c->mint + "/report");
        c->safety = r.ok() ? parse::rugcheck_report(r.body) : SafetyReport{};
        c->safety_ms = r.ok() ? now : 0;
      }));
    }
    quotes.push_back(std::async(std::launch::async, [this, c, sol] { return honeypot_.test(*c, sol, stake_usd_); }));
  }
  for (auto& f : reports) f.get();
  auto [shield, organic] = meta.get();
  deep_checks_ += static_cast<long>(deep.size());

  for (size_t i = 0; i < deep.size(); ++i) {
    Candidate& c = *deep[i].c;
    const Verdict quote_v = quotes[i].get();
    const SafetyReport sr = c.safety ? *c.safety : SafetyReport{};
    std::vector<Verdict> checks = {rug_.check(sr, c.snap->dex), holders_.check(sr),
                                   honeypot_.check_shield(shield.count(c.mint) ? shield[c.mint] : std::vector<std::string>{}),
                                   quote_v,
                                   organic_.check(organic.count(c.mint) ? organic[c.mint] : OrganicFlow::Facts{})};
    bool ok = true;
    for (const auto& v : checks) {
      c.verdicts.push_back(v);
      if (!v.pass) {
        ok = false;
        if (v.hard) reject(c, v, now);
        else c.reject_reason = v.agent + ": " + v.reason;
        break;
      }
    }
    if (!ok) continue;
    if (ai_.enabled()) c.verdicts.push_back(ai_.judge(c));
    if (!strategist_.decide(c)) {
      c.reject_reason = "Strategist: score " + std::to_string(static_cast<int>(c.score));
      continue;
    }
    if (!entries_ok) continue;
    RiskGovernor::Request rq;
    {
      std::lock_guard<std::mutex> lock(mu_);
      rq.size = stake_usd_;
      rq.open = open_count();
      rq.deployed = deployed();
      rq.day_pnl = books_.day_pnl();
      rq.trades_hour = books_.trades_last_hour(now);
      rq.balance = books_.balance();
      rq.holding = std::any_of(positions_.begin(), positions_.end(),
                               [&](const Position& p) { return p.mint == c.mint && !p.closed; });
    }
    rq.score = c.score;
    // Bonding-curve coins have no pool liquidity; their sellability was
    // already proven by HoneypotTester's round-trip quote.
    rq.liq = is_bonding_curve_dex(c.snap->dex) && c.snap->liquidity_usd <= 0 ? cfg_.num("liq_min_usd", 8000)
                                                                              : c.snap->liquidity_usd;
    const std::string denied = risk_.approve(rq);
    if (!denied.empty()) {
      log::info("RiskGovernor", "blocked $" + c.snap->symbol + ": " + denied);
      continue;
    }
    auto pos = exec_.buy(c, stake_usd_, sol);
    if (!pos) continue;
    c.bought = true;
    buys_++;
    {
      std::lock_guard<std::mutex> lock(mu_);
      positions_.push_back(*pos);
      books_.on_buy(*pos);
    }
    std::ostringstream why;
    for (const auto& v : c.verdicts)
      if (!v.reason.empty()) why << v.agent << "{" << v.reason << "} ";
    log::info("Executor", "BUY $" + c.snap->symbol + " " + fmt_usd(stake_usd_) + " score " +
                              std::to_string(static_cast<int>(c.score)) + " | " + why.str());
  }
}

void Orchestrator::monitor(int64_t now) {
  std::vector<std::string> mints;
  {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& p : positions_)
      if (!p.closed) mints.push_back(p.mint);
  }
  if (mints.empty()) return;
  auto snaps = market_.fetch(mints, 3);
  const double sol = market_.sol_usd(now);
  std::lock_guard<std::mutex> lock(mu_);
  for (auto& p : positions_) {
    if (p.closed) continue;
    auto it = snaps.find(p.mint);
    if (it != snaps.end()) {
      missing_[p.mint] = 0;
      p.screen_price = it->second.price_usd;
      p.last_liquidity = it->second.liquidity_usd;
      // Between real quotes, move the sellable mark with the screen price.
      if (now - p.quote_ms > quote_fresh_ms_) p.last_price = p.screen_price * p.quote_ratio;
      p.peak_price = std::max(p.peak_price, p.last_price);
    } else if (!snaps.empty() && ++missing_[p.mint] >= 3) {
      p.last_liquidity = 0;  // pair vanished from the market = treat as rug
    }
  }
  // Real sell quotes for the positions priced longest ago (rate-limit budget).
  std::vector<Position*> open;
  for (auto& p : positions_)
    if (!p.closed) open.push_back(&p);
  std::sort(open.begin(), open.end(), [](const Position* a, const Position* b) { return a->quote_ms < b->quote_ms; });
  for (size_t i = 0; i < open.size() && static_cast<int>(i) < quotes_per_monitor_; ++i)
    if (exec_.mark(*open[i], sol)) open[i]->peak_price = std::max(open[i]->peak_price, open[i]->last_price);
  manage_positions(now, sol);
}

void Orchestrator::manage_positions(int64_t now, double sol) {
  // caller holds mu_
  for (auto& p : positions_) {
    if (p.closed) continue;
    auto a = exits_.evaluate(p, now);
    if (a.fraction <= 0) continue;
    // Never act on a stale screen price: confirm with a real sell quote first.
    // (If no quote comes back, e.g. the pool was pulled, the exit goes ahead.)
    if (now - p.quote_ms > quote_fresh_ms_ && exec_.mark(p, sol)) {
      p.peak_price = std::max(p.peak_price, p.last_price);
      a = exits_.evaluate(p, now);
      if (a.fraction <= 0) continue;
    }
    const double before = p.realized_usd;
    const double got = exec_.sell(p, a.fraction, sol, a.why);
    if (a.why.rfind("take-profit", 0) == 0) p.ladder_step++;
    books_.on_sell(p, got, a.why, a.fraction);
    log::info("ExitManager", "SELL $" + p.symbol + " " + a.why + " @" + fmt_pct(p.multiple() - 1) + " got " +
                                 fmt_usd(p.realized_usd - before));
    if (p.closed) {
      books_.on_close(p);
      if (!p.smart_wallet.empty()) traders_.record_outcome(p.smart_wallet, p.peak_multiple());
      log::info("Accountant", "CLOSED $" + p.symbol + " pnl " + fmt_usd(p.pnl_usd()) + " peak " +
                                  fmt_pct(p.peak_multiple() - 1) + " (" + p.close_reason + ")");
    }
  }
}

void Orchestrator::close_all(const std::string& why) {
  const double sol = market_.sol_usd(now_ms());
  std::lock_guard<std::mutex> lock(mu_);
  for (auto& p : positions_) {
    if (p.closed) continue;
    const double got = exec_.sell(p, 1.0, sol, why);
    books_.on_sell(p, got, why, 1.0);
    books_.on_close(p);
  }
}

void Orchestrator::run(double minutes) {
  started_ms_ = now_ms();
  const int64_t end = minutes > 0 ? started_ms_ + static_cast<int64_t>(minutes * 60000) : INT64_MAX;
  log::info("Orchestrator", std::string("FOMO Family engine starting in ") + (exec_.paper() ? "PAPER" : "LIVE") +
                                " mode, stake " + fmt_usd(stake_usd_) + " per coin\n" + roster());
  std::thread mon([&] {
    while (!stop_) {
      monitor(now_ms());
      for (int i = 0; i < monitor_seconds_ * 10 && !stop_; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });
  while (now_ms() < end && !fs::exists(dir_ + "/STOP")) {
    const int64_t t0 = now_ms();
    try {
      cycle(t0);
    } catch (const std::exception& e) {
      log::error("Orchestrator", std::string("cycle error: ") + e.what());
    }
    const int64_t wait = static_cast<int64_t>(cycle_seconds_) * 1000 - (now_ms() - t0);
    if (wait > 0) std::this_thread::sleep_for(std::chrono::milliseconds(wait));
  }
  stop_ = true;
  mon.join();
  if (close_at_end_) close_all("end of run");
  write_reports();
  log::info("Orchestrator", "finished\n" + summary_text() + "\nAgents:\n" + roster() + "API health:\n" + health_.table() +
                                "Filter audit (coins we rejected, followed afterwards):\n" + books_.audit_table());
}

// ------------------------------------------------------------- reports
std::string Orchestrator::summary_json() const {
  std::lock_guard<std::mutex> lock(mu_);
  int open = 0, closed = 0, wins = 0;
  double pnl_closed = 0, fees = 0, best_peak = 0, open_value = 0;
  json pos = json::array();
  for (const auto& p : positions_) {
    fees += p.fees_usd;
    best_peak = std::max(best_peak, p.peak_multiple());
    if (p.closed) {
      closed++;
      pnl_closed += p.pnl_usd();
      if (p.pnl_usd() > 0) wins++;
    } else {
      open++;
      open_value += p.tokens_left * p.last_price;
    }
    pos.push_back({{"symbol", p.symbol}, {"mint", p.mint}, {"closed", p.closed}, {"why", p.close_reason},
                   {"pnl", p.pnl_usd()}, {"peak_multiple", p.peak_multiple()}, {"multiple", p.multiple()},
                   {"held_min", static_cast<double>((p.closed ? p.closed_ms : now_ms()) - p.opened_ms) / 60000.0},
                   {"score", p.entry_score}, {"source", p.source}, {"fees", p.fees_usd}});
  }
  json agents = json::array();
  for (const Agent* a : std::initializer_list<const Agent*>{&launch_, &trend_, &smart_, &traders_, &market_, &health_, &vol_,
                                                             &liq_, &social_, &momentum_, &rug_, &holders_, &honeypot_,
                                                             &organic_, &ai_, &strategist_, &risk_, &exec_, &exits_, &books_}) {
    const auto s = a->stats();
    agents.push_back({{"id", a->id()}, {"name", a->name()}, {"job", a->job()}, {"runs", s.runs},
                      {"passes", s.passes}, {"fails", s.fails}});
  }
  json rejects = json::object();
  for (const auto& [k, v] : reject_counts_) rejects[k] = v;
  json apis = json::object();
  for (const auto& [h, s] : http_.stats())
    apis[h] = {{"calls", s.calls}, {"errors", s.errors}, {"rate_limited", s.rate_limited}, {"avg_ms", s.avg_ms()}};
  return json{{"mode", exec_.paper() ? "paper" : "live"},
              {"started_ms", started_ms_},
              {"minutes", static_cast<double>(now_ms() - started_ms_) / 60000.0},
              {"stake_usd", stake_usd_},
              {"cycles", cycles_},
              {"leads", leads_},
              {"watch_pool", pool_.size()},
              {"deep_checks", deep_checks_},
              {"buys", buys_},
              {"open", open},
              {"closed", closed},
              {"wins", wins},
              {"pnl_closed_usd", pnl_closed},
              {"open_value_usd", open_value},
              {"fees_usd", fees},
              {"best_peak_multiple", best_peak},
              {"paper_balance_usd", books_.balance()},
              {"rejected_by_agent", rejects},
              {"agents", agents},
              {"apis", apis},
              {"positions", pos}}
      .dump(2);
}

std::string Orchestrator::summary_text() const {
  json j = json::parse(summary_json());
  char b[400];
  std::snprintf(b, sizeof b,
                "%.1f min | leads %ld | watching %zu | deep checks %ld | buys %ld | open %d | closed %d (wins %d) | "
                "closed PnL %s | fees %s | best peak %.2fx | balance %s",
                j["minutes"].get<double>(), j["leads"].get<long>(), j["watch_pool"].get<size_t>(),
                j["deep_checks"].get<long>(), j["buys"].get<long>(), j["open"].get<int>(), j["closed"].get<int>(),
                j["wins"].get<int>(), fmt_usd(j["pnl_closed_usd"].get<double>()).c_str(),
                fmt_usd(j["fees_usd"].get<double>()).c_str(), j["best_peak_multiple"].get<double>(),
                fmt_usd(j["paper_balance_usd"].get<double>()).c_str());
  return b;
}

void Orchestrator::write_reports() const {
  std::ofstream(dir_ + "/summary.json") << summary_json();
  std::lock_guard<std::mutex> lock(mu_);
  std::ofstream(dir_ + "/filter_audit.json") << books_.audit_json();
  std::ofstream(dir_ + "/traders.json") << traders_.leaderboard_json();
}

}  // namespace fomo
