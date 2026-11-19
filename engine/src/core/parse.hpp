#pragma once
// Turn API JSON into the engine's types. Pure functions, unit tested on
// recorded responses in engine/tests/fixtures.
#include <map>
#include <string>
#include <vector>

#include "core/types.hpp"

namespace fomo::parse {

// GET api.dexscreener.com/tokens/v1/solana/{mints} -> best (most liquid) pair per mint.
std::map<std::string, PairSnapshot> dexscreener_tokens(const std::string& body, int64_t ts_ms);

// GET api.dexscreener.com/token-profiles/latest/v1 or token-boosts/{latest,top}/v1
std::vector<Lead> dexscreener_leads(const std::string& body, const std::string& source);

// GET api.geckoterminal.com/api/v2/networks/solana/new_pools
std::vector<Lead> geckoterminal_new_pools(const std::string& body);

// GET api.rugcheck.xyz/v1/tokens/{mint}/report
SafetyReport rugcheck_report(const std::string& body);

// GET lite-api.jup.ag/swap/v1/quote
Quote jupiter_quote(const std::string& body);

// GET lite-api.jup.ag/price/v3?ids=...  -> usdPrice of one id (0 if missing)
double jupiter_price(const std::string& body, const std::string& mint);

}  // namespace fomo::parse
