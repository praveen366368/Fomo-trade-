# FOMO Family

Meme-coin research and a 20-agent trading bot (C++ engine + OCaml risk guard).
$1 per coin, automatic exits, **paper trading on live market data** in this build.

> **Short version (Hinglish):** 20 agents hain — koi naye coins dhoondhta hai, koi
> market data laata hai, koi rug/scam check karta hai, koi volatility, koi API health,
> koi test karta hai ki reject kiye coins baad mein kya hue. Har coin mein $1, profit
> pe auto-exit. Abhi paper mode hai (asli paisa nahi lagta, asli market data pe chalta
> hai). FOMO app ke Terms bots ko ban karte hain, isliye live trading alag wallet se
> on-chain hogi (phase 2).

## Run it

```bash
scripts/check.sh          # build + all tests + lint (needs g++, cmake, libcurl, nlohmann-json, ocaml, dune)
scripts/paper.sh 60       # 60-minute paper run on live data -> data/run-*/
engine/build/fomo-engine --roster   # list the 20 agents
```

Results of a run: `summary.json` (trades, PnL, every agent's pass/fail counts, API
health), `ledger.jsonl` (every fill), `filter_audit.json` (what happened to coins we
rejected), `engine.log`.

Stop early: `touch data/run-*/STOP`. Block new buys: `touch data/run-*/KILL`.

## Settings

All in `config/fomo.conf`; any key can be overridden with `FOMO_<KEY>`.

* `stake_usd = 1` — money per coin.
* `exit_mode = trail` — research-best exit: arm a trailing stop at 1.5x, sell if it
  falls 30% from the peak, leave after 5 min if it never moved.
  `fixed` + `take_profit_pct = 100` sells everything at +100%. `ladder` sells slices.
* `config/wallets.csv` — public wallets to copy-signal (pick from the FOMO leaderboard by hand).
* AI (Jev or any OpenAI-compatible model): set `ai_url` in the config and `FOMO_AI_KEY`
  in the environment. Without it the AI agent is simply off.

## Docs

* `docs/ARCHITECTURE.md` — the 20 agents, data sources, threads, rate limits.
* `docs/STRATEGY.md` — filters, exits, costs and the numbers behind them.
* `docs/ROADMAP.md` — what live trading needs (phase 2).
* `reports/` — the full research report. `research_notes/` — the raw notes with sources.

**Risk:** most new meme coins go to zero (98.6% of pump.fun tokens collapsed or showed
pump-and-dump traits). This is research and paper trading, not financial advice.
