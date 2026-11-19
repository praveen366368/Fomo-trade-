# Strategy: filters, exits, costs

Every number here is a setting in `config/fomo.conf`. Sources and evidence strength
are in `research_notes/` and `reports/`.

## The base rate (why filters come first)
* 98.6% of 7M+ pump.fun tokens (2024 – Mar 2025) collapsed or showed pump-and-dump
  traits (Solidus Labs).
* Only ~0.2–3% of launches graduate from the bonding curve (0.198% of 832,941
  launches in May–June 2026).
* Only ~0.4% of 13.4M pump.fun wallets realised $10k+; most "winners" make $1–$500.
* All the edge in any rule set sits in the rare huge outliers. Remove the top 0.1%
  of trades and every rule in the simulation went negative.

## Live evidence, 19 Nov 2026
* **FOMO app itself:** 90% of 829,539 wallets lost money after fees, Jan–Oct 2026
  (median −$25). 74% of 2.9M Solana meme buyers lost SOL within two weeks.
* **Market sample (255 new coins, bought on paper ~6 min after launch, tracked 60 min):**
  median 1-hour price ratio 1.00; 66% had no trades in the last hour; 6.3% touched
  2x, 2.4% touched 5x, ~0.4% touched 10x; the 2x coins gave back a median 98.5%
  from peak. Buying every coin with a +100–400% take-profit: about −3% to −5% per $1
  with lean bot costs, about −31% to −35% with app-default fees.
* **Our engine (45 min, filters on):** 4,860 leads → 145 deep checks → 5 buys,
  0 wins, −$0.93 total. Four coins never rose 15% above our real entry in 5 min.
  Filter audit: 84 of 101 followed rejects went dead; 1 later did 2.6x.
* Who earns: dev wallets + bundles made at least $579M after fees vs $31.5M for
  snipers (Bitquery).

## Filters (agents 8–14), cheapest first
| Check | Default | Type | Evidence |
|---|---|---|---|
| Mint / freeze authority active | reject | hard | binary trap; a miss costs 100% |
| Token-2022 permanent delegate, transfer hook, transfer fee, non-transferable | reject | hard | binary trap |
| Jupiter Shield `NOT_SELLABLE` / failed sell quote | reject | hard | binary trap |
| Round-trip quote loss | > 12% reject | hard | cost math |
| RugCheck danger risks | > 0 reject | hard | tool signal |
| LP locked/burned (not on bonding curve) | < 90% reject | hard | practitioner |
| Top-10 holders (pools excluded) | > 30% reject | hard | tool convention; linked wallets make the real figure ~24% higher |
| Single wallet / creator | > 10% / > 5% reject | hard | tool convention |
| Creator launched other coins | > 3 reject | hard | serial launchers |
| Socials (X/Telegram/site) | required | soft | Telegram-advertised coins graduated 8.9× more often (fast launches only) |
| Age | 3 min – 24 h | mixed | skip first-minute bundles/snipers |
| Liquidity / market cap | ≥ $8k / $10k – $5M | soft | $1 round trip needs depth |
| 5-min buys, buy/sell ratio, turnover | ≥ 25, ≥ 1.1, ≥ 0.05× | soft | first-5-minute features are the best-studied predictors |
| 5-min change | −15% … +120% | soft | don't catch knives, don't chase vertical candles |
| Organic flow (Jupiter) | organic score ≥ 5 or organic buys ≥ 3% | soft | ≥17% of trades are wash trades |

"Soft" = the coin stays on the watch list for 30 min and can qualify later.
"Hard" = never reconsidered. The Auditor follows both and records which filters
rejected coins that later did 2x+ (`filter_audit.json`).

## Exits (agent 19)
Simulation results (assumption-driven, not fitted to real price paths):
| Rule | EV per $1 |
|---|---|
| Sell 100% at 2x | about −$0.40 (needs ~39–43% hit rate) |
| Ladder, 90% sold by 10x | about −$0.28 |
| **Trailing stop armed at 1.5x, 30% from peak, 5-min time stop** | **about +$0.23 … +$0.25** |

Default = the trailing rule (`exit_mode = trail`); time stop leaves at 5 min below 1.3x.
All exit decisions use the real sellable price (Jupiter sell quote), not the screen price. Owner's "sell at X%" rule is
`exit_mode = fixed`, `take_profit_pct = X`. Also always on: stop-loss at 0.5×, rug
exit when liquidity drops 50%, max hold 6 h. Real pull-backs may whipsaw a 30% trail;
test 40–50% on recorded data before going live.

## Costs at $1 (why $1 is hard)
* Solana fee 5,000 lamports + priority fee (~50,000) per transaction: about $0.004
  each at SOL ≈ $74.
* Pool fee: pump.fun AMM 1.25% per side (inside the Jupiter quote).
* Token-account rent ~0.00204 SOL (~$0.15) locked per open coin, refunded on close.
* Landing tips (Jito/Sender) of 0.001 SOL would be ~7% of $1 per side — not used.
* Through the FOMO app: 0.5% per side plus small-trade minimums (client code shows a
  $2 minimum; up to 0.95 USDC on small Solana trades) — a $1 trade there is not viable.
* Research estimate: ~12% round trip on $1, ~18% for a 5-step ladder. Practical
  minimum stake $2 (round trips) to $5 (ladders).
* First live paper trade: $JETSKI timed out at +2.1% and still returned $0.89 for $1.

## Sizing
* Flat $1 per coin (owner's rule). Kelly on the positive rule ≈ 0.45% of bankroll →
  ~$220 bankroll for $1 stakes; $20–$50 bankrolls were ruined 47–84% of the time.
* Daily loss limit 12 stakes; max 25 open; max 30 buys/hour (OCaml RiskGovernor).

## Copy trading
* FOMO "copy trading" is only notifications; its Terms ban bots and scraping.
* Evidence is negative for followers: copying at the earliest signal lost out of
  sample in 32/32 setups; copying 2 minutes late lost 41% per token; smart-money
  wallets ~14% vs ~3% for copiers after costs.
* So SmartMoney only adds a +15 score bonus; the coin must still pass every filter.
