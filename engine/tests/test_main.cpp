// FOMO Family engine tests. No network: every API answer comes from real
// responses recorded on 2026-11-19 (tests/fixtures) or small hand-made JSON.
#include <nlohmann/json.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "agents/agent.hpp"
#include "app/orchestrator.hpp"
#include "core/config.hpp"
#include "core/log.hpp"
#include "core/parse.hpp"
#include "core/util.hpp"

using namespace fomo;
using nlohmann::json;

static int g_fail = 0, g_pass = 0;
#define CHECK(cond)                                                              \
  do {                                                                           \
    if (cond) {                                                                  \
      g_pass++;                                                                  \
    } else {                                                                     \
      g_fail++;                                                                  \
      std::fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);       \
    }                                                                            \
  } while (0)
#define NEAR(a, b, tol) CHECK(std::fabs((a) - (b)) <= (tol))

static std::string fixture(const std::string& name) {
  std::ifstream f(std::string(FIXTURES) + "/" + name);
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

static const std::string GTA = "8MhvFChnHk55N286euYcE6pWNw8nRu1hLdJ11512pump";
static const std::string NOSELL = "6Jhj71pZztUHh6j1Au6yA61Lqv1bpm11iFc7Lav4pump";

// Fake network: first route whose key is a substring of the URL answers.
class FakeHttp : public Http {
 public:
  std::vector<std::pair<std::string, std::function<std::string()>>> routes;
  std::vector<std::string> seen;
  void on(const std::string& key, std::function<std::string()> f) { routes.emplace_back(key, std::move(f)); }
  void on(const std::string& key, const std::string& body) {
    routes.emplace_back(key, [body] { return body; });
  }
  HttpResponse get(const std::string& url) override {
    seen.push_back(url);
    for (auto& [k, f] : routes)
      if (url.find(k) != std::string::npos) return {200, f(), 1, ""};
    return {404, "{\"error\":\"not found\"}", 1, ""};
  }
  HttpResponse post_json(const std::string& url, const std::string&, const std::vector<std::string>&) override {
    return get(url);
  }
  std::map<std::string, HostStats> stats() const override { return {}; }
};

static Config base_config() {
  return Config::from_string(fixture("../../../config/fomo.conf"));
}

// ------------------------------------------------------------------ tests
static void test_util_and_config() {
  CHECK(is_solana_address(GTA));
  CHECK(!is_solana_address("0xdeadbeef"));
  CHECK(!is_solana_address("8MhvFChnHk55N286euYcE6pWNw8nRu1hLdJ11512pumO"));  // 'O' not base58 -> invalid
  CHECK(is_bonding_curve_dex("pumpfun") && !is_bonding_curve_dex("pumpswap"));
  Config c = Config::from_string("a = 1\n# comment\nb = hello # trailing\nflag = true\nladder = 2:0.5, 5:0.25\n");
  CHECK(c.num("a", 0) == 1);
  CHECK(c.str("b") == "hello");
  CHECK(c.flag("flag", false));
  CHECK(c.num("missing", 7) == 7);
  auto p = c.pairs("ladder");
  CHECK(p.size() == 2 && p[1].first == 5 && p[1].second == 0.25);
  setenv("FOMO_A", "42", 1);
  CHECK(c.num("a", 0) == 42);  // env wins
  unsetenv("FOMO_A");
  bool threw = false;
  try {
    Config::from_string("x = abc").num("x", 0);
  } catch (...) {
    threw = true;
  }
  CHECK(threw);
  // the shipped config parses
  Config shipped = base_config();
  CHECK(shipped.num("stake_usd", 0) == 1);
  CHECK(shipped.str("mode") == "paper");
}

static void test_parsers() {
  auto snaps = parse::dexscreener_tokens(fixture("dexscreener_tokens.json"), 123);
  CHECK(snaps.size() == 3);
  const auto& g = snaps.at(GTA);
  CHECK(g.dex == "pumpswap" && g.symbol == "GTA");
  NEAR(g.liquidity_usd, 44766.02, 0.01);
  NEAR(g.price_usd, 0.0003992, 1e-9);
  CHECK(g.buys_m5 == 1245 && g.sells_m5 == 448);
  CHECK(g.has_twitter && g.has_website && !g.has_telegram);
  CHECK(g.quote_mint == kWsolMint && g.ts_ms == 123);
  const auto& curve = snaps.at("AbaVzFBs99eCARKoBRC49hgpnNVf5TfUnwSRmVRJpump");
  CHECK(curve.dex == "pumpfun" && curve.liquidity_usd == 0);  // bonding curve: no liquidity field

  auto boosts = parse::dexscreener_leads(fixture("dexscreener_boosts.json"), "trend:boost");
  CHECK(boosts.size() == 23);  // 30 rows, 23 on Solana
  CHECK(boosts[0].boost > 0);
  auto pools = parse::geckoterminal_new_pools(fixture("geckoterminal_new_pools.json"));
  CHECK(pools.size() == 20);
  CHECK(parse::geckoterminal_new_pools("{\"status\":{\"error_code\":429}}").empty());
  CHECK(parse::dexscreener_tokens("<html>cloudflare</html>", 0).empty());

  auto q = parse::jupiter_quote(fixture("jupiter_quote.json"));
  CHECK(q.ok && q.out_amount == 3535217035.0 && q.route == "Pump.fun Amm");
  NEAR(q.price_impact_pct, 0.738, 0.001);
  auto qe = parse::jupiter_quote(fixture("jupiter_quote_error.json"));
  CHECK(!qe.ok && qe.error.find("not tradable") != std::string::npos);
  NEAR(parse::jupiter_price("{\"" + std::string(kWsolMint) + "\":{\"usdPrice\":73.7}}", kWsolMint), 73.7, 1e-9);
}

static void test_rugcheck_and_holders() {
  SafetyReport r = parse::rugcheck_report(fixture("rugcheck_report.json"));
  CHECK(r.fetched);
  CHECK(!r.mint_authority_set && !r.freeze_authority_set && !r.rugged);
  CHECK(r.token2022 && !r.transfer_fee && !r.permanent_delegate && !r.transfer_hook);
  CHECK(r.decimals == 6 && r.total_holders == 1826 && r.creator_tokens == 5);
  NEAR(r.lp_locked_pct, 100, 1e-9);
  NEAR(r.top10_pct_ex_pools, 37.385, 0.01);       // pool account (7.1%) excluded
  NEAR(r.max_holder_pct_ex_pools, 4.405, 0.01);
  CHECK(!parse::rugcheck_report("{}").fetched);

  Config c = base_config();
  RugGuard rug(c);
  CHECK(rug.check(r, "pumpswap").pass);
  SafetyReport bad = r;
  bad.mint_authority_set = true;
  auto v = rug.check(bad, "pumpswap");
  CHECK(!v.pass && v.hard);
  bad = r;
  bad.lp_locked_pct = 10;
  CHECK(!rug.check(bad, "raydium").pass);
  CHECK(rug.check(bad, "pumpfun").pass);  // bonding curve: no LP to lock
  bad = r;
  bad.danger_risks = 1;
  bad.risk_names = {"Creator history of rugged tokens"};
  v = rug.check(bad, "pumpswap");
  CHECK(!v.pass && v.hard && v.reason.find("rugged tokens") != std::string::npos);

  HolderAnalyst holders(c);
  v = holders.check(r);  // real coin: top-10 hold 37% -> rejected
  CHECK(!v.pass && v.hard && v.reason.find("top-10") != std::string::npos);
  SafetyReport ok = r;
  ok.top10_pct_ex_pools = 20;
  ok.creator_tokens = 1;
  CHECK(holders.check(ok).pass);
  ok.creator_tokens = 9;
  CHECK(!holders.check(ok).pass);  // serial launcher
}

static void test_honeypot_and_organic() {
  Config c = base_config();
  FakeHttp http;
  http.on("ultra/v1/shield", fixture("jupiter_shield.json"));
  http.on("tokens/v2/search", fixture("jupiter_tokens_v2.json"));
  http.on("outputMint=" + GTA, fixture("jupiter_quote.json"));
  std::string sell_out = "12500000";
  http.on("inputMint=" + GTA, [&] { return "{\"inAmount\":\"3535217035\",\"outAmount\":\"" + sell_out + "\",\"priceImpactPct\":\"0.01\",\"routePlan\":[]}"; });
  HoneypotTester hp(http, c);
  auto sh = hp.shield({GTA, NOSELL});
  CHECK(sh.count(NOSELL) == 1);
  auto v = hp.check_shield(sh[NOSELL]);
  CHECK(!v.pass && v.hard);
  CHECK(hp.check_shield(sh[GTA]).pass);  // info flags only

  Candidate cand;
  cand.mint = GTA;
  // $0.958 at SOL $73.69 = 13,000,000 lamports; sell back 12.5M = -3.8%
  v = hp.test(cand, 73.69, 0.958);
  CHECK(v.pass);
  CHECK(cand.buy_quote && cand.buy_quote->ok && cand.sell_quote && cand.sell_quote->ok);
  sell_out = "9000000";  // -31% round trip
  v = hp.test(cand, 73.69, 0.958);
  CHECK(!v.pass && v.hard);
  Candidate gone;
  gone.mint = NOSELL;  // no route answers -> 404 JSON
  v = hp.test(gone, 73.69, 1.0);
  CHECK(!v.pass);

  OrganicFlow org(http, c);
  auto facts = org.fetch({GTA});
  CHECK(facts.count(GTA) && facts[GTA].ok);
  NEAR(facts[GTA].organic_score, 0, 1e-9);
  CHECK(facts[GTA].organic_buy_share < 0.03);  // ~1% organic: bot volume
  CHECK(facts[GTA].dev_mints == 10);
  v = org.check(facts[GTA]);
  CHECK(!v.pass && v.reason.find("botted") != std::string::npos);
  CHECK(org.check(OrganicFlow::Facts{}).pass);  // no data = neutral
}

static PairSnapshot snap(double price, double liq, int64_t age_min, int64_t now) {
  PairSnapshot s;
  s.mint = GTA;
  s.symbol = "GTA";
  s.dex = "pumpswap";
  s.price_usd = price;
  s.liquidity_usd = liq;
  s.mcap = s.fdv = 400000;
  s.vol_m5 = 90000;
  s.buys_m5 = 300;
  s.sells_m5 = 150;
  s.chg_m5 = 10;
  s.has_twitter = true;
  s.pair_created_ms = now - age_min * 60000;
  s.ts_ms = now;
  return s;
}

static void test_cheap_filters() {
  Config c = base_config();
  const int64_t now = now_ms();
  LiquidityGuard liq(c);
  Candidate cand;
  cand.mint = GTA;
  cand.snap = snap(0.0004, 44000, 30, now);
  CHECK(liq.check(cand, now).pass);
  cand.snap = snap(0.0004, 44000, 1, now);
  auto v = liq.check(cand, now);
  CHECK(!v.pass && !v.hard);  // too new: wait
  cand.snap = snap(0.0004, 44000, 2000, now);
  CHECK(!liq.check(cand, now).pass && liq.check(cand, now).hard);  // too old
  cand.snap = snap(0.0004, 500, 30, now);
  CHECK(liq.check(cand, now).hard);  // dead
  auto curve = snap(0.0004, 0, 30, now);
  curve.dex = "pumpfun";
  cand.snap = curve;
  v = liq.check(cand, now);
  CHECK(!v.pass && !v.hard && v.reason.find("bonding curve") != std::string::npos);

  Momentum mom(c);
  cand.snap = snap(0.0004, 44000, 30, now);
  CHECK(mom.check(cand).pass);
  auto s2 = snap(0.0004, 44000, 30, now);
  s2.sells_m5 = 400;
  cand.snap = s2;
  CHECK(!mom.check(cand).pass);  // sellers winning
  s2 = snap(0.0004, 44000, 30, now);
  s2.chg_m5 = 300;
  cand.snap = s2;
  CHECK(!mom.check(cand).pass);  // don't chase

  SocialCheck social(c);
  cand.snap = snap(0.0004, 44000, 30, now);
  CHECK(social.check(cand).pass);
  s2 = snap(0.0004, 44000, 30, now);
  s2.has_twitter = false;
  cand.snap = s2;
  CHECK(!social.check(cand).pass);
  for (int i = 0; i < 5; ++i) social.observe_symbol("GTA", "mint" + std::to_string(i), now);
  cand.snap = snap(0.0004, 44000, 30, now);
  CHECK(!social.check(cand).pass);  // copy-cat ticker

  Volatility vol(c);
  for (int i = 0; i < 10; ++i) {
    auto s = snap(0.0004 * (i % 2 ? 1.5 : 1.0), 44000, 30, now + i * 60000);
    vol.observe(s);
  }
  CHECK(vol.vol_per_min(GTA) > 0.35);
  cand.snap = snap(0.0004, 44000, 30, now);
  CHECK(!vol.check(cand).pass);
  Volatility calm(c);
  for (int i = 0; i < 10; ++i) calm.observe(snap(0.0004 * (1 + 0.01 * i), 44000, 30, now + i * 60000));
  CHECK(calm.check(cand).pass);
}

static Position open_pos(int64_t now) {
  Position p;
  p.mint = GTA;
  p.symbol = "GTA";
  p.opened_ms = now;
  p.size_usd = 1;
  p.cost_usd = 1.004;
  p.entry_price = 0.001;
  p.peak_price = p.last_price = 0.001;
  p.tokens_initial = p.tokens_left = 1000;
  p.entry_liquidity = p.last_liquidity = 40000;
  return p;
}

static void test_exits() {
  const int64_t t0 = now_ms();
  Config c = base_config();  // exit_mode = trail
  ExitManager trail(c);
  CHECK(trail.ladder().empty());
  Position p = open_pos(t0);
  p.last_price = p.peak_price = 0.0014;
  CHECK(trail.evaluate(p, t0 + 2 * 60000).fraction == 0);  // up 40%, not armed: hold
  p.peak_price = 0.002;
  p.last_price = 0.00135;  // -32.5% from 2x peak
  auto a = trail.evaluate(p, t0 + 3 * 60000);
  CHECK(a.fraction >= 1 && a.why == "trailing stop");
  p = open_pos(t0);
  p.last_price = p.peak_price = 0.0011;
  CHECK(trail.evaluate(p, t0 + 6 * 60000).why == "time stop");
  p = open_pos(t0);
  p.last_price = 0.00045;
  CHECK(trail.evaluate(p, t0 + 60000).why == "stop-loss");
  p = open_pos(t0);
  p.last_liquidity = 15000;
  CHECK(trail.evaluate(p, t0 + 60000).why.rfind("rug exit", 0) == 0);

  Config fixed_cfg = base_config();
  fixed_cfg.set("exit_mode", "fixed");
  fixed_cfg.set("take_profit_pct", "100");
  ExitManager fixed(fixed_cfg);
  p = open_pos(t0);
  p.last_price = p.peak_price = 0.00205;
  a = fixed.evaluate(p, t0 + 60000);
  CHECK(a.fraction == 1.0 && a.why == "take-profit 2x");

  Config lad_cfg = base_config();
  lad_cfg.set("exit_mode", "ladder");
  ExitManager ladder(lad_cfg);
  p = open_pos(t0);
  p.last_price = p.peak_price = 0.0021;
  a = ladder.evaluate(p, t0 + 60000);
  CHECK(a.fraction == 0.5 && a.why == "take-profit 2x");
  p.ladder_step = 1;
  p.last_price = p.peak_price = 0.0052;
  CHECK(ladder.evaluate(p, t0 + 60000).fraction == 0.25);
}

static void test_executor_and_books() {
  Config c = base_config();
  FakeHttp http;
  http.on("inputMint=" + GTA, "{\"inAmount\":\"1\",\"outAmount\":\"26000000\",\"priceImpactPct\":\"0.01\",\"routePlan\":[]}");
  FeeModel fees = FeeModel::from(c);
  Executor ex(http, c, fees);
  CHECK(ex.paper());
  Candidate cand;
  cand.mint = GTA;
  cand.snap = snap(0.0004, 44000, 30, now_ms());
  cand.safety = SafetyReport{};
  cand.safety->decimals = 6;
  cand.buy_quote = parse::jupiter_quote(fixture("jupiter_quote.json"));
  cand.score = 70;
  auto pos = ex.buy(cand, 1.0, 73.69);
  CHECK(pos.has_value());
  Position p = *pos;
  NEAR(p.tokens_initial, 3535.217035 * 0.97, 1e-6);  // quote minus 3% landing drift
  NEAR(p.entry_price, 1.0 / p.tokens_initial, 1e-12);
  const double tx = (5000.0 + 50000.0) / 1e9 * 73.69;
  NEAR(p.cost_usd, 1.0 + tx, 1e-9);
  NEAR(p.rent_usd, 0.00203928 * 73.69, 1e-9);

  Accountant books(c, std::filesystem::temp_directory_path().string());
  const double bal0 = books.balance();
  books.on_buy(p);
  NEAR(books.balance(), bal0 - p.cost_usd - p.rent_usd, 1e-9);
  CHECK(books.trades_last_hour(now_ms()) == 1);

  // Sell everything: quote gives 0.026 SOL = $1.916 gross.
  p.last_price = p.entry_price * 2;
  const double got = ex.sell(p, 1.0, 73.69, "test");
  const double gross = 0.026 * 73.69 * 0.97;
  NEAR(got, gross - tx, 1e-9);  // rent is not profit; Accountant refunds it on close
  CHECK(p.closed && p.tokens_left == 0);
  books.on_sell(p, got, "test", 1.0);
  books.on_close(p);
  NEAR(books.day_pnl(), p.realized_usd - p.cost_usd, 1e-9);
  NEAR(books.balance(), bal0 + p.pnl_usd(), 1e-9);  // rent came back

  // Rugged pool: no quote and liquidity gone -> $0
  Position r = open_pos(now_ms());
  r.mint = NOSELL;
  r.last_liquidity = 0;
  CHECK(ex.sell(r, 1.0, 73.69, "rug exit") == 0.0);

  // Filter audit: follow two rejected coins.
  Candidate a1, a2;
  a1.mint = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  a2.mint = GTA;
  a1.snap = snap(0.001, 40000, 30, 0);
  a2.snap = snap(0.001, 40000, 30, 0);
  a1.reject_agent = "HolderAnalyst";
  a2.reject_agent = "Momentum";
  books.track_reject(a1, 0);
  books.track_reject(a2, 0);
  const int64_t later = 301 * 1000;
  auto due = books.audit_mints_due(later);
  CHECK(due.size() == 2);
  std::map<std::string, PairSnapshot> after;
  after[GTA] = snap(0.0025, 40000, 30, later);  // 2.5x after we said no
  books.audit_update(after, due, later);       // a1 missing -> dead
  json j = json::parse(books.audit_json());
  CHECK(j["by_agent"]["Momentum"]["later_2x"] == 1);
  CHECK(j["by_agent"]["HolderAnalyst"]["later_dead"] == 1);
}

static void test_strategist_and_traders() {
  Config c = base_config();
  Strategist st(c);
  Candidate cand;
  cand.verdicts = {{"A", true, false, 10, ""}, {"B", true, false, 8, ""}};
  CHECK(st.decide(cand) && cand.score == 68);
  cand.verdicts = {{"A", true, false, 2, ""}};
  CHECK(!st.decide(cand));
  cand.sources = {"smart:trader_a"};
  CHECK(st.decide(cand) && cand.score == 67);  // smart-money bonus
  cand.verdicts.push_back({"C", false, false, 0, "no"});
  CHECK(!st.decide(cand));

  const std::string path = (std::filesystem::temp_directory_path() / "fomo_wallets_test.csv").string();
  std::ofstream(path) << "# test\nalpha," << GTA << ",manual\nbad,0xnotsolana,manual\n";
  TraderScout ts(c);
  ts.load(path);
  CHECK(ts.snapshot().size() == 1);
  for (int i = 0; i < 8; ++i) ts.record_outcome(GTA, 1.1);
  CHECK(ts.snapshot()[0].muted);  // 0 of 8 reached 2x
}

static void test_risk_daemon() {
  Config c = base_config();
  c.set("risk_daemon", RISKD);
  RiskGovernor risk(c);
  CHECK(risk.alive());
  RiskGovernor::Request r;
  r.size = 1;
  r.balance = 100;
  r.score = 80;
  r.liq = 20000;
  CHECK(risk.approve(r).empty());
  r.day_pnl = -12;
  CHECK(risk.approve(r) == "daily_loss_limit");
  r.day_pnl = 0;
  r.size = 5;
  CHECK(risk.approve(r) == "size_above_cap");

  Config broken = base_config();
  broken.set("risk_daemon", "/nonexistent/riskd");
  RiskGovernor dead(broken);
  CHECK(!dead.alive());
  r.size = 1;
  CHECK(dead.approve(r) == "risk_daemon_down");  // fail closed
}

// Full pipeline on fake data: discover -> filters -> OCaml risk -> paper buy
// -> price runs to 2x -> falls back -> trailing stop sells.
static void test_end_to_end() {
  Config c = base_config();
  c.set("risk_daemon", RISKD);
  c.set("hold_max_top10_pct", "40");      // fixture coin is 37% top-10
  c.set("hold_max_creator_tokens", "10"); // and its creator made 5 coins
  c.set("organic_min_score", "-1");       // and it is botted
  c.set("wallets_file", "/nonexistent");
  const auto dir = (std::filesystem::temp_directory_path() / "fomo_e2e").string();
  std::filesystem::remove_all(dir);

  FakeHttp http;
  double price = 0.0003992;
  const int64_t created = now_ms() - 30 * 60000;
  json pairs = json::parse(fixture("dexscreener_tokens.json"));
  http.on("tokens/v1/solana", [&] {
    json out = json::array();
    for (auto p : pairs) {
      if (p["baseToken"]["address"] == GTA) {
        p["pairCreatedAt"] = created;
        char b[32];
        std::snprintf(b, sizeof b, "%.10f", price);
        p["priceUsd"] = b;
        std::snprintf(b, sizeof b, "%.12f", price / 73.69);
        p["priceNative"] = b;
      }
      out.push_back(p);
    }
    return out.dump();
  });
  http.on("token-profiles/latest", "[{\"chainId\":\"solana\",\"tokenAddress\":\"" + GTA + "\"}]");
  http.on("token-boosts", "[]");
  http.on("geckoterminal", "{\"data\":[]}");
  http.on("price/v3", "{\"" + std::string(kWsolMint) + "\":{\"usdPrice\":73.69}}");
  http.on("rugcheck.xyz", fixture("rugcheck_report.json"));
  http.on("ultra/v1/shield", fixture("jupiter_shield.json"));
  http.on("tokens/v2/search", fixture("jupiter_tokens_v2.json"));
  http.on("outputMint=" + GTA, fixture("jupiter_quote.json"));
  http.on("inputMint=" + GTA, "{\"inAmount\":\"1\",\"outAmount\":\"12600000\",\"priceImpactPct\":\"0.01\",\"routePlan\":[]}");

  Orchestrator eng(c, http, dir);
  const int64_t t = now_ms();
  eng.cycle(t);
  CHECK(eng.positions().size() == 1);
  if (eng.positions().empty()) return;
  CHECK(!eng.positions()[0].closed);
  const double entry = eng.positions()[0].entry_price;
  price = entry * 2.2;  // pumps
  eng.monitor(t + 60000);
  CHECK(!eng.positions()[0].closed);
  NEAR(eng.positions()[0].peak_multiple(), 2.2, 0.01);
  price = entry * 1.4;  // -36% from peak -> trailing stop
  eng.monitor(t + 120000);
  CHECK(eng.positions()[0].closed && eng.positions()[0].close_reason == "trailing stop");
  json s = json::parse(eng.summary_json());
  CHECK(s["buys"] == 1 && s["closed"] == 1);
  CHECK(s["agents"].size() == 20);
  CHECK(std::filesystem::exists(dir + "/ledger.jsonl"));
}

int main() {
  log::quiet() = true;
  test_util_and_config();
  test_parsers();
  test_rugcheck_and_holders();
  test_honeypot_and_organic();
  test_cheap_filters();
  test_exits();
  test_executor_and_books();
  test_strategist_and_traders();
  test_risk_daemon();
  test_end_to_end();
  std::printf("engine tests: %d passed, %d failed\n", g_pass, g_fail);
  return g_fail == 0 ? 0 : 1;
}
