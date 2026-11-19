# Roadmap

## Phase 1 — done (this build)
* 20 agents, C++ engine + OCaml risk daemon, paper trading on live data.
* All data keyless: Dexscreener, GeckoTerminal, RugCheck, Jupiter lite-api, Solana public RPC.
* Filter audit: every rejected coin is followed afterwards to grade each filter.

## Phase 2 — live trading (needs owner's setup, not keys in chat)
1. **Separate burner wallet** just for the bot. Not the FOMO account: FOMO's Terms
   (§16) ban bots, scripts and scraping, and trading the same wallet outside the app
   breaks its PnL. Keep only a small float in it (e.g. $50–$250).
2. **Signer service**: key held in a separate process/KMS (Turnkey/Privy policy or a
   local signer), never in `config/` or env files in this repo. Program allowlist
   (Jupiter, pump.fun AMM), per-day spend cap, simulate every transaction first.
3. **Execution**: Jupiter Swap API v2 (`api.jup.ag/swap/v2`, free key 1 rps; $25/mo
   for 10 rps) with tight min-out instead of 15% slippage (sandwich bots took ~$29.7M
   from wide-slippage swaps in the year to Oct 2026).
4. **Faster data**: PumpPortal websocket (free) for launches; Helius Developer
   ($49/mo) or a gRPC stream ($49–$499/mo) if paper results justify it.
5. **Server**: run the hot path near Solana leaders (New York / Amsterdam / Frankfurt),
   not from India; that alone is ~80–120 ms.
6. **Jev / AI**: set `ai_url` and `FOMO_AI_KEY` on the server. The AI only adds or
   subtracts score; it can never place or size a trade.

## Phase 3 — learn
* Tune thresholds from `filter_audit.json` (which filters rejected coins that later
  did 2x+, which saved us from rugs).
* Build a replay backtester from recorded snapshots (research: a +$0.20/trade edge
  needs tens of thousands of trades to prove).
* TraderScout feedback: keep wallets whose signals actually hit 2x; mute the rest.

## Owner decisions still open
* Stake: $1 per coin is the rule; research says fees make $2–$5 the practical minimum.
* Bankroll: research (Kelly) says ~$220+ for $1 stakes; small bankrolls ($20–$50) were
  ruined 47–84% of the time in simulation even with a positive edge.
