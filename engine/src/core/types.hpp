#pragma once
// Data that moves between agents.
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace fomo {

// A mint some discovery agent thinks is worth a look.
struct Lead {
  std::string mint;
  std::string source;   // "launch:geckoterminal", "trend:boost", "smart:<label>" ...
  double boost = 0;     // paid Dexscreener boost amount, if any
  std::string wallet;   // for smart-money leads: the wallet that bought
};

// One market snapshot of a token's most liquid pair (Dexscreener).
struct PairSnapshot {
  std::string mint, pair, dex, symbol, name, quote_mint;
  double price_usd = 0, price_native = 0;
  double liquidity_usd = 0, fdv = 0, mcap = 0;
  double vol_m5 = 0, vol_h1 = 0, vol_h24 = 0;
  double chg_m5 = 0, chg_h1 = 0, chg_h24 = 0;   // percent, e.g. 976 = +976%
  int buys_m5 = 0, sells_m5 = 0, buys_h1 = 0, sells_h1 = 0;
  int64_t pair_created_ms = 0;
  bool has_website = false, has_twitter = false, has_telegram = false;
  int64_t ts_ms = 0;
};

// Contract and holder facts from RugCheck.
struct SafetyReport {
  bool fetched = false;
  bool mint_authority_set = false, freeze_authority_set = false;
  bool rugged = false, mutable_metadata = false;
  bool token2022 = false, transfer_fee = false, permanent_delegate = false;
  bool non_transferable = false, transfer_hook = false, default_frozen = false;
  double transfer_fee_pct = 0;
  int danger_risks = 0, warn_risks = 0;
  std::vector<std::string> risk_names;
  double score_normalised = 0;
  double lp_locked_pct = 0;            // best market
  double top10_pct_ex_pools = 0;       // holders excluding AMM/bonding-curve accounts
  double max_holder_pct_ex_pools = 0;
  int insider_holders = 0;             // top holders flagged insider
  int graph_insiders = 0;              // insider network size
  double creator_pct = 0;
  int creator_tokens = 0;              // other tokens by the same creator
  int total_holders = 0;
  int decimals = 6;
  double total_liquidity_usd = 0;
  std::string launchpad;
};

struct Quote {
  bool ok = false;
  double in_amount = 0;    // raw units
  double out_amount = 0;   // raw units
  double price_impact_pct = 0;
  std::string route;       // first AMM label
  std::string error;
};

struct Verdict {
  std::string agent;
  bool pass = true;
  bool hard = false;       // hard fail = never reconsider this coin
  double score = 0;        // contribution to the entry score
  std::string reason;
};

struct Candidate {
  std::string mint;
  std::string symbol;
  std::vector<std::string> sources;
  std::string smart_wallet;
  double boost = 0;
  int64_t first_seen_ms = 0;
  int64_t last_eval_ms = 0;
  std::optional<PairSnapshot> snap;
  std::optional<SafetyReport> safety;
  int64_t safety_ms = 0;
  std::optional<Quote> buy_quote, sell_quote;
  std::vector<Verdict> verdicts;
  double score = 0;
  bool rejected = false;
  bool hard_rejected = false;
  std::string reject_agent, reject_reason;
  bool bought = false;
};

struct Position {
  std::string mint, symbol, pair;
  int64_t opened_ms = 0;
  int64_t closed_ms = 0;
  double size_usd = 0;          // stake, $1 by design
  double cost_usd = 0;          // stake + entry fees (rent tracked separately)
  double rent_usd = 0;          // token-account rent, refunded when fully closed
  double entry_price = 0;       // USD per token (effective)
  double tokens_initial = 0;
  double tokens_left = 0;
  int decimals = 6;
  double peak_price = 0;
  double last_price = 0;        // SELLABLE price per token (real quote, or screen x quote_ratio)
  double screen_price = 0;      // Dexscreener price (can lag up to ~30 s)
  double quote_ratio = 1.0;     // last real sell quote / screen price
  int64_t quote_ms = 0;         // when last_price came from a real quote
  double entry_liquidity = 0;
  double last_liquidity = 0;
  double realized_usd = 0;      // net proceeds received so far
  double fees_usd = 0;          // all fees paid
  size_t ladder_step = 0;       // next take-profit level
  bool closed = false;
  std::string close_reason;
  double entry_score = 0;
  std::string source;
  std::string smart_wallet;

  double multiple() const { return entry_price > 0 ? last_price / entry_price : 0; }
  double peak_multiple() const { return entry_price > 0 ? peak_price / entry_price : 0; }
  double pnl_usd() const { return realized_usd - cost_usd; }
};

}  // namespace fomo
