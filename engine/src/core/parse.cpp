#include "core/parse.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <set>

#include "core/util.hpp"

namespace fomo::parse {

using nlohmann::json;

namespace {

// APIs mix numbers, numeric strings and nulls. Read any of them as double.
double num(const json& j, const char* key, double def = 0) {
  if (!j.is_object()) return def;
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) return def;
  if (it->is_number()) return it->get<double>();
  if (it->is_string()) {
    try {
      return std::stod(it->get<std::string>());
    } catch (...) {
      return def;
    }
  }
  return def;
}

std::string str(const json& j, const char* key) {
  if (!j.is_object()) return "";
  auto it = j.find(key);
  if (it == j.end() || !it->is_string()) return "";
  return it->get<std::string>();
}

const json& sub(const json& j, const char* key) {
  static const json null_json = nullptr;
  if (!j.is_object()) return null_json;
  auto it = j.find(key);
  return it == j.end() ? null_json : *it;
}

bool is_base_quote_asset(const std::string& mint) {
  return mint == kWsolMint || mint == kUsdcMint || mint == kUsdtMint;
}

json safe_parse(const std::string& body) {
  json j = json::parse(body, nullptr, /*allow_exceptions=*/false);
  return j;
}

}  // namespace

std::map<std::string, PairSnapshot> dexscreener_tokens(const std::string& body, int64_t ts_ms) {
  std::map<std::string, PairSnapshot> best;
  json j = safe_parse(body);
  const json* arr = &j;
  if (j.is_object() && j.contains("pairs")) arr = &j["pairs"];  // older /latest/dex/tokens shape
  if (!arr->is_array()) return best;
  for (const auto& p : *arr) {
    if (str(p, "chainId") != "solana") continue;
    PairSnapshot s;
    s.mint = str(sub(p, "baseToken"), "address");
    if (!is_solana_address(s.mint) || is_base_quote_asset(s.mint)) continue;
    s.pair = str(p, "pairAddress");
    s.dex = str(p, "dexId");
    s.symbol = str(sub(p, "baseToken"), "symbol");
    s.name = str(sub(p, "baseToken"), "name");
    s.quote_mint = str(sub(p, "quoteToken"), "address");
    s.price_usd = num(p, "priceUsd");
    s.price_native = num(p, "priceNative");
    s.liquidity_usd = num(sub(p, "liquidity"), "usd");
    s.fdv = num(p, "fdv");
    s.mcap = num(p, "marketCap", s.fdv);
    const json& vol = sub(p, "volume");
    s.vol_m5 = num(vol, "m5");
    s.vol_h1 = num(vol, "h1");
    s.vol_h24 = num(vol, "h24");
    const json& chg = sub(p, "priceChange");
    s.chg_m5 = num(chg, "m5");
    s.chg_h1 = num(chg, "h1");
    s.chg_h24 = num(chg, "h24");
    const json& tx = sub(p, "txns");
    s.buys_m5 = static_cast<int>(num(sub(tx, "m5"), "buys"));
    s.sells_m5 = static_cast<int>(num(sub(tx, "m5"), "sells"));
    s.buys_h1 = static_cast<int>(num(sub(tx, "h1"), "buys"));
    s.sells_h1 = static_cast<int>(num(sub(tx, "h1"), "sells"));
    s.pair_created_ms = static_cast<int64_t>(num(p, "pairCreatedAt"));
    const json& info = sub(p, "info");
    if (sub(info, "websites").is_array() && !sub(info, "websites").empty()) s.has_website = true;
    if (sub(info, "socials").is_array()) {
      for (const auto& so : sub(info, "socials")) {
        const std::string t = str(so, "type");
        if (t == "twitter") s.has_twitter = true;
        if (t == "telegram") s.has_telegram = true;
      }
    }
    s.ts_ms = ts_ms;
    if (s.price_usd <= 0) continue;
    auto it = best.find(s.mint);
    if (it == best.end() || s.liquidity_usd > it->second.liquidity_usd) best[s.mint] = s;
  }
  return best;
}

std::vector<Lead> dexscreener_leads(const std::string& body, const std::string& source) {
  std::vector<Lead> out;
  json j = safe_parse(body);
  if (!j.is_array()) return out;
  for (const auto& x : j) {
    if (str(x, "chainId") != "solana") continue;
    Lead l;
    l.mint = str(x, "tokenAddress");
    if (!is_solana_address(l.mint)) continue;
    l.source = source;
    l.boost = num(x, "totalAmount", num(x, "amount"));
    out.push_back(l);
  }
  return out;
}

std::vector<Lead> geckoterminal_new_pools(const std::string& body) {
  std::vector<Lead> out;
  json j = safe_parse(body);
  if (!j.is_object() || !j["data"].is_array()) return out;
  for (const auto& pool : j["data"]) {
    const json& rel = sub(pool, "relationships");
    auto mint_of = [&](const char* side) {
      std::string id = str(sub(sub(rel, side), "data"), "id");
      auto us = id.find('_');
      return us == std::string::npos ? std::string() : id.substr(us + 1);
    };
    std::string m = mint_of("base_token");
    if (is_base_quote_asset(m)) m = mint_of("quote_token");
    if (!is_solana_address(m) || is_base_quote_asset(m)) continue;
    out.push_back(Lead{m, "launch:geckoterminal", 0, ""});
  }
  return out;
}

SafetyReport rugcheck_report(const std::string& body) {
  SafetyReport r;
  json j = safe_parse(body);
  if (!j.is_object() || !j.contains("mint")) return r;
  r.fetched = true;
  const json& tok = sub(j, "token");
  r.mint_authority_set = !sub(j, "mintAuthority").is_null() || !sub(tok, "mintAuthority").is_null();
  r.freeze_authority_set = !sub(j, "freezeAuthority").is_null() || !sub(tok, "freezeAuthority").is_null();
  r.decimals = static_cast<int>(num(tok, "decimals", 6));
  r.rugged = sub(j, "rugged").is_boolean() && j["rugged"].get<bool>();
  const json& meta = sub(j, "tokenMeta");
  r.mutable_metadata = sub(meta, "mutable").is_boolean() && meta["mutable"].get<bool>();
  r.token2022 = str(j, "tokenProgram") == "TokenzQdBNbLqP5VEhdkAS6EPFLC1PHnBqCXEpPxuEb";
  const json& ext = sub(j, "token_extensions");
  r.permanent_delegate = !sub(ext, "permanentDelegate").is_null();
  r.non_transferable = sub(ext, "nonTransferable").is_boolean() && ext["nonTransferable"].get<bool>();
  const json& hook = sub(ext, "transferHook");
  r.transfer_hook = hook.is_object() && !sub(hook, "programId").is_null();
  const json& das = sub(ext, "defaultAccountState");
  r.default_frozen = (das.is_string() && das.get<std::string>() == "frozen") ||
                     (das.is_object() && str(das, "state") == "frozen");
  r.transfer_fee_pct = num(sub(j, "transferFee"), "pct");
  r.transfer_fee = r.transfer_fee_pct > 0 || !sub(ext, "transferFeeConfig").is_null();
  if (sub(j, "risks").is_array()) {
    for (const auto& rk : j["risks"]) {
      const std::string lvl = str(rk, "level");
      if (lvl == "danger") r.danger_risks++;
      else if (lvl == "warn") r.warn_risks++;
      r.risk_names.push_back(str(rk, "name"));
    }
  }
  r.score_normalised = num(j, "score_normalised");
  r.total_holders = static_cast<int>(num(j, "totalHolders"));
  r.total_liquidity_usd = num(j, "totalMarketLiquidity");
  r.graph_insiders = static_cast<int>(num(j, "graphInsidersDetected"));
  r.launchpad = str(sub(j, "launchpad"), "platform");
  if (sub(j, "creatorTokens").is_array()) r.creator_tokens = static_cast<int>(j["creatorTokens"].size());

  // Accounts that belong to pools/bonding curves/lockers, not to traders.
  std::set<std::string> pool_accounts;
  if (sub(j, "markets").is_array()) {
    for (const auto& m : j["markets"]) {
      pool_accounts.insert(str(m, "pubkey"));
      pool_accounts.insert(str(m, "liquidityA"));
      pool_accounts.insert(str(m, "liquidityB"));
      r.lp_locked_pct = std::max(r.lp_locked_pct, num(sub(m, "lp"), "lpLockedPct"));
    }
  }
  if (sub(j, "knownAccounts").is_object()) {
    for (auto it = j["knownAccounts"].begin(); it != j["knownAccounts"].end(); ++it)
      if (str(it.value(), "type") != "CREATOR") pool_accounts.insert(it.key());
  }
  const double supply = num(tok, "supply");
  const double creator_bal = num(j, "creatorBalance");
  if (supply > 0) r.creator_pct = creator_bal / supply * 100.0;

  if (sub(j, "topHolders").is_array()) {
    std::vector<double> pcts;
    for (const auto& h : j["topHolders"]) {
      const bool is_pool = pool_accounts.count(str(h, "address")) || pool_accounts.count(str(h, "owner"));
      if (is_pool) continue;
      if (sub(h, "insider").is_boolean() && h["insider"].get<bool>()) r.insider_holders++;
      pcts.push_back(num(h, "pct"));
    }
    std::sort(pcts.rbegin(), pcts.rend());
    for (size_t i = 0; i < pcts.size() && i < 10; ++i) r.top10_pct_ex_pools += pcts[i];
    if (!pcts.empty()) r.max_holder_pct_ex_pools = pcts.front();
  }
  return r;
}

Quote jupiter_quote(const std::string& body) {
  Quote q;
  json j = safe_parse(body);
  if (!j.is_object()) {
    q.error = "bad_json";
    return q;
  }
  if (j.contains("error")) {
    q.error = j["error"].is_string() ? j["error"].get<std::string>() : "error";
    return q;
  }
  q.in_amount = num(j, "inAmount");
  q.out_amount = num(j, "outAmount");
  q.price_impact_pct = num(j, "priceImpactPct") * 100.0;  // API gives a fraction
  if (sub(j, "routePlan").is_array() && !j["routePlan"].empty())
    q.route = str(sub(j["routePlan"][0], "swapInfo"), "label");
  q.ok = q.out_amount > 0;
  if (!q.ok) q.error = "no_output";
  return q;
}

double jupiter_price(const std::string& body, const std::string& mint) {
  json j = safe_parse(body);
  if (!j.is_object()) return 0;
  if (j.contains("data") && j["data"].is_object()) j = j["data"];  // v2 shape
  return num(sub(j, mint.c_str()), "usdPrice", num(sub(j, mint.c_str()), "price"));
}

}  // namespace fomo::parse
