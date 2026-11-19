# FOMO Family — 20-agent architecture

One job per agent. Cheap checks run first; anything that costs an API call
runs only on coins that survived the cheap checks. Nothing an AI says can
touch the money: the last word on every buy is the OCaml RiskGovernor, and
it fails closed (no answer = no trade).

```
 DISCOVER          OBSERVE              FILTER (cheap → expensive)              DECIDE              ACT            LEARN
 1 LaunchScout ─┐  5 MarketData ──┐     8 LiquidityGuard   11 RugGuard         15 NarrativeAI(Jev) 18 Executor    20 Auditor
 2 TrendScout  ─┼─►6 ApiHealth   ─┼──►  9 SocialCheck   ►  12 HolderAnalyst ►  16 Strategist   ►   19 ExitManager    (books +
 3 SmartMoney  ─┤  7 Volatility  ─┘    10 Momentum         13 HoneypotTester   17 RiskGovernor                     filter audit)
 4 TraderScout ─┘                                          14 OrganicFlow         (OCaml)
```

| # | Agent | Job | Data source (keyless today) |
|---|-------|-----|------------------------------|
| 1 | LaunchScout | New pools and launches | GeckoTerminal `new_pools`, Dexscreener `token-profiles/latest` |
| 2 | TrendScout | Trending / paid-promoted coins | Dexscreener `token-boosts/latest`, `token-boosts/top` |
| 3 | SmartMoney | Fresh buys by tracked wallets → leads | Solana RPC `getSignaturesForAddress` + `getTransaction` |
| 4 | TraderScout | Keeps the copy-trade wallet list; mutes wallets whose signals don't hit 2x | `config/wallets.csv` + own results |
| 5 | MarketData | Price, liquidity, 5m/1h flow, SOL price (30 coins per call) | Dexscreener `tokens/v1/solana/{mints}` |
| 6 | ApiHealth | Latency, errors, 429s per API; pauses buys if the price feed is failing | own HTTP stats |
| 7 | Volatility | Realised volatility from our own snapshots | MarketData history |
| 8 | LiquidityGuard | Liquidity, age window, market-cap window, FDV/liquidity; waits for bonding-curve coins to graduate | snapshot |
| 9 | SocialCheck | Has X/Telegram/website; flags copy-cat tickers | snapshot |
| 10 | Momentum | Buys vs sells, 5m turnover, no chasing vertical candles | snapshot |
| 11 | RugGuard | Mint/freeze authority, Token-2022 traps, RugCheck dangers, LP locked | RugCheck `/v1/tokens/{mint}/report` |
| 12 | HolderAnalyst | Top-10 %, single whale, insiders, serial-launcher creators | RugCheck report |
| 13 | HoneypotTester | Jupiter Shield `NOT_SELLABLE`, buy→sell round-trip quote | Jupiter `ultra/v1/shield`, `swap/v1/quote` |
| 14 | OrganicFlow | Real buyers vs bots (organic score, organic buy share, traders in 5m) | Jupiter `tokens/v2/search` |
| 15 | NarrativeAI | Optional AI score of the story/hype. Jev (`jev-latest`) or any OpenAI-compatible model. Off without a key | `ai_url` + env `FOMO_AI_KEY` |
| 16 | Strategist | Adds every verdict into one 0–100 score; buys at ≥ `entry_score` | all verdicts |
| 17 | RiskGovernor | **OCaml** process. Size cap $1, max open, max deployed, daily loss, trades/hour, reserve | `risk/` |
| 18 | Executor | Paper fills at the real Jupiter quote minus fees and landing drift. Live mode = phase 2 | Jupiter quote |
| 19 | ExitManager | Trailing stop (default), fixed take-profit, or ladder; stop-loss, time stop, rug exit | snapshots |
| 20 | Auditor | Ledger, balance, daily PnL; follows coins we rejected and grades each filter | `data/*/ledger.jsonl`, `filter_audit.json` |

## Threads and timing

* **Discovery/filter loop** every `cycle_seconds` (10 s). Network checks for up to
  `deep_checks_per_cycle` coins run in parallel.
* **Position monitor** on its own thread every `monitor_seconds` (4 s), so exits are
  not delayed by slow safety checks. Positions are marked at a **real Jupiter sell
  quote** (`quotes_per_monitor` per pass); no exit fires on a mark older than
  `quote_fresh_seconds` (15 s) until a fresh quote confirms it.
* **Hard rejects are permanent** (ban list outside the watch pool).
* **Rate-limit buckets** (measured 2026-11-19): Dexscreener 250 ms, GeckoTerminal 20 s,
  RugCheck 4.5 s (15/window), Jupiter quote 0.7 s (~100/window), Jupiter tokens/shield/price
  13 s (~5/window shared), Solana RPC 150 ms.

## Languages

* **C++20** — engine, agents, HTTP, parsing (`engine/`).
* **OCaml** — risk limits (`risk/`). Pure function + line protocol, unit-tested.
  Talks to the engine over a pipe; if it dies, every buy is blocked.
* Research finding: on Solana the speed that matters is the data feed (gRPC/shreds),
  server location (NY/Amsterdam/Frankfurt) and how a transaction lands (Jito/Sender) —
  not C++ vs Rust. See `docs/ROADMAP.md`.

## Files

```
config/fomo.conf      every threshold (env override: FOMO_<KEY>)
config/wallets.csv    copy-trade watchlist (public wallets, picked by hand)
engine/src/core       config, HTTP + rate limits, types, parsers, fee model
engine/src/agents     the 20 agents
engine/src/app        orchestrator (loop + monitor thread + reports)
engine/tests          126 tests on recorded real API responses
risk/                 OCaml risk daemon + 18 tests
scripts/check.sh      build + tests + lint (gate)
scripts/paper.sh      paper-trade on live data
```
