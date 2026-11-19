# Strategy, Filters and Risk Rules for $1-per-coin Automated Solana Meme-coin Trading (as of Nov 2026)

Research notes for the report writer. Method note: arxiv.org and several docs hosts (e.g. docs.solanatracker.io) could not be fetched from this session (blocked by the network allowlist), so the academic findings below come from search-engine abstracts and excerpts, not full-text reads. Treat exact numbers from arXiv preprints as "as quoted in the abstract or excerpt". Most 2026 arXiv items are recent, non-peer-reviewed preprints, and several have corrigenda. Evidence-strength tags used throughout: **[STUDY]** = academic or quantitative study (mostly preprints), **[BACKTEST]** = published open-source backtest, **[VENDOR-DATA]** = data from a tool vendor or exchange blog, **[PRACTITIONER]** = tool defaults, bot guides, opinion, **[MODEL]** = my own simulation or math below (assumption-driven, not fitted to real data).

Simulation code (ephemeral scratchpad, reproducible with seeds): `/tmp/claude-0/-home-user-Fomo-trade-/3e4dd943-72b3-5a68-a92f-d602376fc661/scratchpad/exit_sim.py` and `/tmp/claude-0/-home-user-Fomo-trade-/3e4dd943-72b3-5a68-a92f-d602376fc661/scratchpad/sens_and_bankroll.py`. All parameters are restated in the text, so the results can be checked without the code.

## 1. Rug/scam detection checklist: thresholds and which checks have the best evidence

### Takeaway
The base rate is overwhelmingly bad. About 98.6% of the 7M+ pump.fun tokens studied in 2024 to Q1 2025 collapsed or showed pump-and-dump traits, and roughly 0.2% to 3% of launches graduate. A filter is therefore needed mainly to remove *unsellable* or *instantly extractable* tokens. Hard, binary on-chain checks carry the most reliable signal and have zero false-negative tolerance: mint and freeze authority, Token-2022 permanent delegate, transfer hook and non-transferable flags, and sell simulation. The best-*studied* predictive signals are early trading and liquidity features in the first 5 minutes, plus bundle- and entity-adjusted holder concentration. The paper says visible top-10% understates true concentration by about 24% for high-risk tokens. Most numeric thresholds in tools (top-10 < 15–30%, dev < 5–10%, bundles < 20%) are practitioner conventions, not validated cut-offs.

### Cited Findings
**Base rates (context for any filter)**
- Solidus Labs studied 7M+ pump.fun tokens with at least five trades (Jan 2024 to Mar 2025). Only about 97,000 kept liquidity above $1,000. The headline figure is that 98.6% (site says about 98.7%) showed rug-pull or pump-and-dump traits. On Raydium, 361,000 of 388,000 maker-funded pools (93%) showed "soft rug" characteristics. The median rug was about $2,832 and the largest $1.9M. [VENDOR-DATA/STUDY] — [Solidus Labs report](https://www.soliduslabs.com/reports/solana-rug-pulls-pump-dumps-crypto-compliance); [CryptoPotato coverage](https://cryptopotato.com/98-of-tokens-on-pump-fun-are-rug-pulls-or-fraud-report/). Pump.fun disputed the report — [CoinDesk](https://www.coindesk.com/business/2025/05/07/98-of-tokens-on-pump-fun-have-been-rug-pulls-or-an-act-of-fraud-new-report-says). Caveat: "98.6%" is a low-liquidity or pump-and-dump classification, not proven fraud. The exact "soft rug" definition (sudden vs gradual liquidity withdrawal) is described inconsistently in coverage.
- The pump.fun graduation rate is low and regime-dependent. A pooled 0.198% (Wilson 95% CI 0.189–0.208%) across 832,941 launches, May 8 to Jun 10 2026. The same paper cites 0.63% for Sep–Oct 2025 [STUDY] — [arXiv 2607.02823v3](https://arxiv.org/html/2607.02823v3). Other figures: about 0.26% in mid-2026 [VENDOR-DATA] — [DEXTools](https://www.dextools.io/news/pump-fun-graduation-collapse-solana-fees-2026). 0.84% in April 2026 data — [pumpfundata](https://pumpfundata.com/blog/pumpfun-graduation-rate-analysis). The best week was Nov 2024 at 1.67% — [Cointelegraph](https://cointelegraph.com/news/pump-fun-memecoins-are-dying-at-record-rates-less-than-1-survive). A one-day spike to 6.7% followed the BOOST change (Jul 2026). BOOST deploys migration liquidity into automatic market buys for 5 minutes after migration and burns the tokens bought — [The Block](https://www.theblock.co/news/defi/2026-07-29-pump-fun-token-graduation-rate-jumps-boost-changes-launch-incentives-409815). A separate measurement study reports 2.8% graduation among "complete" tokens in its sample — [arXiv 2610.39458](https://arxiv.org/abs/2610.39458).
- MemeTrans (Hu, Tekin, Xu, Liu, 2026) covers 41k+ launches and 200M+ transactions with bundled-account traces and 122 features (launchpad trading behaviour, holding concentration, bundle statistics). 84.13% of the labelled (migrated) launches are high-risk, 11.34% medium and 4.53% low. ML models trained on it reduce simulated financial loss by 56.1% [STUDY] — [arXiv 2602.13480](https://arxiv.org/html/2602.13480v1); [Emergent Mind summary](https://www.emergentmind.com/papers/2602.13480).

**Which signals predict rugs (study evidence)**
- "Catching the Rug" (arXiv 2608.20271) frames rug detection as binary classification using **only the first 5 minutes of trading and liquidity data**, with no contract code. It covers Raydium and Pump.fun and compares Random Forest, XGBoost and MLP. XGBoost and RF are most robust on AUC-PR and F1 with fused data, and XGBoost generalizes best across platforms. Positive-class F1 values in excerpts are about 0.72–0.79, but one cross-platform configuration drops to about 0.38. The study reports that most memecoins show rug characteristics within an hour of launch. Secondary summaries list about 23 features: early-block price change, buy/sell ratios, and unique buyer and seller counts [STUDY, preprint] — [arXiv 2608.20271v1](https://arxiv.org/abs/2608.20271v1); [ResearchGate](https://www.researchgate.net/publication/413051617_Catching_the_Rug_Early_Prediction_of_Fraudulent_Memecoins_on_Solana_via_Machine_Learning). Caveat: dataset counts differ between versions (e.g. Raydium 2,931,946 / 1,893,539 vs 43,835 / 9,711 rows), and I could not confirm the "6.4M tokens" figure. A further preprint, "Early Rug Pull Detection on Solana" (arXiv 2611.13342), exists but I could not read it — [arXiv PDF](https://arxiv.org/pdf/2611.13342).
- MemeTrans finds that engineered features, specifically **sale duration and bundle statistics**, predict better than long-range sequence models. It also finds the **true top-10 holder percentage of high-risk tokens averages 24% higher than the surface figure** once bundled or linked accounts are merged [STUDY] — [Lacuna summary of MemeTrans](https://lacuna.tiptreesystems.com/work/memetrans-a-dataset-for-detecting-high-risk-memecoin-launches-on-solana/wrk_9bae5b5f543b1331ffd891ca59a05ee0).
- "Meme Coin Factories" (arXiv 2609.10246, reportedly ACM CCS 2026) covers all about 15M pump.fun coins over about 2 years. It identifies five manipulation classes: wash trading, creator-address obfuscation, coordinated selling, copycat coins and social-media manipulation. At least 17% of trading transactions are wash trades and at least 10% of coins are copycats. **Coins with heavier wash trading were more likely to graduate**, so manipulation "works" on the graduation metric [STUDY, preprint] — [arXiv 2609.10246](https://arxiv.org/abs/2609.10246). Coinpaprika cites a more conservative 8.26% of coins carrying wash trades — [Coinpaprika](https://coinpaprika.com/education/launchpad-red-flags/). The two figures measure different things (share of transactions vs share of coins), and the discrepancy is unresolved.
- Serial or team exits: "Serial Rug-Pull-Like Exits on Pump.fun" (arXiv 2610.41597) measures repeated team-associated selling around sharp collapses. It explicitly makes no fraud, common-ownership or realized-profit claims. Most price-based episodes are also found by a reserve-and-SOL screen. Daily SOL price shows no association with episode incidence. Groups' cash flow on their *other* launches is negative in aggregate in 3 periods, so teams do not win on every launch [STUDY, preprint] — [arXiv 2610.41597](https://arxiv.org/abs/2610.41597).
- Coordinated snipers: 1,012 persistent wallet cohorts (2–12 wallets, 2,965 addresses) co-fire as early buyers across 166,098 launches in about 13.4 days (Jun 2026). Of these, 153 are "high-tier" (≥10 launches) and 22 "premium" (≥20 launches). The **arXiv PDF text says the 1,012 cohorts are withdrawn pending correction** [STUDY, under correction] — [arXiv 2607.02795](https://arxiv.org/abs/2607.02795); [Zenodo catalogue](https://zenodo.org/records/21765387).
- Concentration and fragility: the ME2F framework (arXiv 2512.00377) includes a Whale Dominance Score (top-100 cumulative share plus HHI-style concentration). Memecoins show sharper concentration gradients than mainstream assets [STUDY] — [arXiv 2512.00377](https://arxiv.org/abs/2512.00377).

**Tool and practitioner thresholds (conventions, not validated cut-offs)**
- Solana Tracker risk API flags "Top 10 Holders" as danger above 15% of supply and "Single Holder Ownership" above 10%. It also flags mint authority enabled, and LP not burned or locked (score 4000) [PRACTITIONER/tool] — [Solana Tracker risk docs](https://docs.solanatracker.io/public-data-api/risk); [token-safety guide](https://docs.solanatracker.io/guides/token-safety.md).
- A third-party RugCheck-style checklist scores active mint authority −50 and disabled +10 — [skills.sh rug checklist](https://skills.sh/sanctifiedops/solana-skills/rug-detection-checklist). ZapScanner uses top-10 ≤ 20–30% — [ZapScanner docs](https://zapalpha.gitbook.io/zapalpha/how-it-works/zapscanner-tg-bot). RugCheck reportedly detects insider wallet clusters that a plain holder list misses — [Solana Compass](https://solanacompass.com/projects/rugcheck). RugCheck's own scoring weights were not found.
- Axiom "Pulse" filters set maximums for top-10, dev %, snipers, insiders and bundles, and minimums for holders and liquidity. Axiom counts a "bundle" as **≥4 buys in the same block**. Guides suggest dev > 5% is risky (another uses a 10% max), a bundle max of about 20% for new pairs (30–40% bundling "doesn't always mean a rug"), top-10 > 30% as a bundled or dev-snipe sign, and a > 30% sniper-supply flag [PRACTITIONER] — [axiompro.app](https://axiompro.app/memecoin-sniper/); [Meme Coin Navigator](https://memecoinnavigator.com/best-axiom-trade-filters-for-new-pairs/).
- GMGN's "Bundle %" measures **bundled trading volume**, while other tools measure **supply held by bundled wallets**, so the same token shows different numbers across platforms [PRACTITIONER] — [GMGN blog](https://gmgn.ai/blog/what-is-a-meme-coin-bundle/).
- Open-source sniper bots: CHECK_IF_MINT_IS_RENOUNCED defaulted to `false` in one sample config. "CHECK_IF_BURNED" means *LP burned* in one repo and something else in others [PRACTITIONER] — [texora/solana-sniper-bot](https://awesome.ecosyste.ms/projects/github.com%2Ftexora%2Fsolana-sniper-bot); [bortoom/solana-raydium-sniper](https://awesome.ecosyste.ms/projects/github.com%2Fbortoom%2Fsolana-raydium-sniper).

**Token-2022 and honeypot mechanics**
- Permanent delegate lets the mint's delegate move or burn holders' tokens at any time. Holders cannot revoke it because it sits on the mint. A transfer hook runs a custom program on every transfer and can make sells fail. The non-transferable flag is a permanent honeypot. Screening guidance treats permanent delegate and active transfer hook as disqualifying for memecoins, and also checks non-zero transfer fee and default-frozen state. One vendor reports transfer hooks (58 rejections) and active freeze authority (45) as the most common honeypot mechanisms it rejects (single vendor, unverified) [PRACTITIONER] — [dev.to: Token-2022 traps](https://dev.to/mrwizardlyloaf/token-2022-traps-that-drain-ai-trading-agents-and-how-to-screen-them-33bo); [Solana docs: extensions](https://solana.com/docs/tokens/extensions).
- A 2026 blog describes an automated "permanent delegate burn" factory that burns buyers' tokens within seconds to a minute of purchase (single source, uncorroborated) — [dev.to](https://dev.to/ohmygod/solanas-permanent-delegate-burn-scam-how-token-2022-extensions-power-2026s-largest-automated-rug-4579).
- The same extensions have legitimate compliance uses, e.g. PYUSD/USDG use permanent delegate and transfer hooks. Their presence is decisive for memecoins but not for stablecoins — [Spark research](https://www.spark.money/research/solana-token-extensions-payments).

**Socials, copycats, boosts**
- Copycats are at least 10% of pump.fun coins [STUDY] — [arXiv 2609.10246](https://arxiv.org/abs/2609.10246).
- DexScreener boosts and paid profiles are "paid visibility upgrades that any project can buy, scams included" [PRACTITIONER] — [AirdropAlert review](https://airdropalert.com/blogs/dexscreener-review/). No rigorous study of boost effects on price or rug rate was found (see Gaps).

### Inferences
- **Hard rejects.** Encode these as instant rejects first. They are binary, cheap (one RPC `getAccountInfo` on the mint plus a sell simulation), and a miss costs 100%:
  1. Mint authority not null.
  2. Freeze authority not null.
  3. Token-2022 with PermanentDelegate, TransferHook (non-null program), NonTransferable, DefaultAccountState=Frozen, or TransferFee > 0.
  4. Sell simulation (simulateTransaction of a sell of the bought amount) fails.
  5. Mutable metadata with update authority held by the creator (soft flag).

  For pump.fun bonding-curve tokens, mint and freeze authority are normally handled by the program, so these checks matter most for tokens launched elsewhere (Raydium LaunchLab, direct pools). LP burn is likewise relevant only to non-pump.fun pools; PumpSwap migration handles the LP. This is my inference from the pump.fun design, not verified in this session. Evidence: mechanism-level certainty (on-chain semantics), plus [PRACTITIONER] prevalence data.
- **Concentration and coordination.** Compute these *after entity merging*, i.e. cluster wallets by common funder, same-slot or same-block buys (Axiom's ≥4 buys in a block), and Jito-bundle co-inclusion. Then apply a threshold. MemeTrans shows visible top-10 understates true concentration by about 24% for high-risk tokens. Suggested starting thresholds, excluding the bonding curve or pool account: merged top-10 ≤ 25–30%, largest non-pool entity ≤ 8–10%, dev plus linked wallets ≤ 5–10%, bundled-supply ≤ 20%, sniper-supply ≤ 20–30%. These are [PRACTITIONER] levels; no study validates the cut-points.
- **Creator history.** Use it as a *soft* feature. Rejecting creators or funders that launched ≥N tokens in the last 24–72h is intuitive, but the team-level studies show group exits are measurable while group aggregate cash flow can still be negative. No study found here gives a validated "N prior launches" cut-off.
- **Best-evidence layer for a pipeline.** A gradient-boosted classifier on first-5-minute trading features: buy/sell ratio, unique buyers and sellers, early price change, sale duration and bundle statistics. This is the only approach with published out-of-sample metrics (F1 about 0.72–0.79 in-platform). Note that it degrades sharply cross-platform (F1 about 0.38 in one configuration), so the model must be trained on the venue you trade.
- **Wash trading.** Because wash trading *raises* graduation odds, volume-based entry signals must be de-washed (self-trades and circular funder flows removed) before use.

### Gaps
- RugCheck, SolSniffer and GoPlus official scoring weights and methodology docs were not retrieved. Only third-party descriptions were found, and docs.solanatracker.io was unreachable for a full read.
- No study validates specific numeric thresholds (e.g. top-10 < 25% vs < 30%) against forward outcomes. All thresholds above are conventions.
- No reliable measurement was found of how often Token-2022 traps appear on pump.fun specifically in 2026, or whether pump.fun's current create flow uses Token-2022 by default (unverified).
- Chainalysis 2025–2026 memecoin or rug reports were not retrieved.
- Fake-social detection (bought followers, recycled Telegram) has no quantitative evidence beyond the graduation-lift result for *having* socials (see §2).

## 2. Entry signals that precede large moves; market-cap bands; quantitative studies

### Takeaway
Published evidence points to *early curve state* (SOL in the bonding curve and speed of filling it) and *structural launch features* (advertised socials, above-default initial market cap) as the informative early signals. Following "smart" or early-buyer wallet groups loses money out of sample. Graduation predictors only work for launches that move fast: effects concentrate in tokens completing within about 6 minutes. Advertised Telegram presence carries roughly an 8.9x graduation lift (lower bound). There is no published backtest showing a profitable entry rule after costs. Delayed copy-trading of KOLs and smart wallets lost 20–41% per trade in the only controlled tests found.

### Cited Findings
- Kamat (arXiv 2607.02823v3, corrected v2) studies 832,941 launches (May 8 to Jun 10 2026) using Kaplan–Meier and Cox models.
  - **Advertised Telegram presence and above-default initial market cap are the two strongest predictors of graduation, but only for launches completing within about 6 minutes of creation.**
  - Telegram: 1.485% vs 0.166% graduation (8.94x lift; Cox HR 5.40, 95% CI 4.73–6.17), described as a lower bound.
  - All three socials: 1.919% vs 0.110% (17.4x), but after the corrigendum, twitter and ≥2-social effects become *upper* bounds.
  - Has_website survives unchanged.
  - A proposed late-graduation correction was withdrawn.

  [STUDY, preprint with corrigenda] — [arXiv 2607.02823v3](https://arxiv.org/html/2607.02823v3); [SSRN](https://papers.ssrn.com/sol3/papers.cfm?abstract_id=6915560); dataset [Zenodo RED-PUMP-2026-v1](https://zenodo.org/records/21908375).
- Marino, Naviglio, Tarantelli, Lillo (arXiv 2602.14860) model graduation probability conditioned on **SOL locked in the bonding curve** plus structural and behavioural launch variables. Conditioning on these significantly improves prediction. The paper frames this as evidence about informational efficiency and manipulation in bonding-curve launches [STUDY] — [arXiv 2602.14860](https://arxiv.org/abs/2602.14860); [SSRN](https://papers.ssrn.com/sol3/papers.cfm?abstract_id=6543715). Specific numeric results were not retrievable.
- Gafarov (arXiv 2610.39458, prospective measurement study):
  - Fast-wallet groups are associated with a **2–4x higher chance of reaching 20 SOL in the curve, but no lift in graduation**.
  - Tokens with no group signal graduated at 6.1%, as high as those with medium buyers-only groups.
  - Team signals often arrive when the curve already holds 20 SOL and add no information beyond early curve state.
  - **A follower entering at the earliest group signal loses money out of sample in all 32 configurations and does no better than random entry.**

  [STUDY, preprint] — [arXiv 2610.39458](https://arxiv.org/abs/2610.39458).
- Gafarov (arXiv 2611.11436, Nov 2026) studies team-following profitability with funding-network teams ranked in hindsight across 10 windows (Apr 2024 to Sep 2026). The weakest window is close to break-even, and the result "rises several-fold with a stop rule or another funding-graph construction". Only 3 of 40 selection-matched null tests reject after Holm adjustment. Hindsight ranking means this is not a tradable result [STUDY, preprint] — [arXiv 2611.11436](https://arxiv.org/abs/2611.11436).
- Coordinated sniper cohorts are associated with +16.1% first-30-minute non-cohort buyer count (propensity-matched), but **SOL-inflow lift is not distinguishable from zero**. The cohort results are partly withdrawn pending correction [STUDY] — [arXiv 2607.02795](https://arxiv.org/abs/2607.02795); [Zenodo](https://zenodo.org/records/21765387).
- The delayed copy-trading test entered 2 minutes after large wallet clusters (450–2,000 wallets, often buying within 1–2 minutes of launch) bought. It **lost 41% per token across 93 tokens**. Following a single wallet lost 20% per trade. Early cluster buyers often doubled before the drop, but followers did not capture it [STUDY/journalism] — [TokenPost](https://www.tokenpost.com/news/investing/35409).
- Kombucho backtest of **4,293 pump.fun "callouts"** by 443 caller wallets (open data and code):
  - 1h median −12.6%, mean +16.5% gross.
  - 24h median −50.3%, mean +2.7% gross.
  - With about 7.24% round-trip base cost, means are +8.1% (1h) and −4.8% (24h).
  - **Only 0.6% (26 calls) reached ≥10x within 1h.** Removing the best 24 calls turns the gross mean negative.
  - Entry is assumed at the call price, which is optimistic because real followers buy later into the spike.
  - Monte Carlo traders with fixed bet fractions mostly ended below their start (median ending bankroll reported as $570; starting value not confirmed in the excerpt).

  [BACKTEST] — [GitHub Kombucho/memecoin-gamble-research](https://github.com/Kombucho/memecoin-gamble-research).
- KOL-signal integrity: reports in Nov 2026 allege pump.fun paid callers about $700K to shill mostly tiny tokens, and allegedly offered an influencer $100K/month (headline-level only; not analysed) — [Protos](https://protos.com/pump-fun-paid-700k-to-callers-shilling-mostly-tiny-tokens/); [crypto.news](https://crypto.news/pump-fun-alleged-100k-monthly-influencer-deal).
- Bot crowding at launch: in a DEXTools sample of 215 fresh pump.fun tokens (29 Jun 2026), about 24% showed a heavy early bot burst (≥20 trades in the first 15 min), with p90 at 54 trades. The Japanese version adds that about 30% had zero trades and the median was 3 trades [VENDOR-DATA] — [DEXTools](https://www.dextools.io/news/pumpfun-new-token-bot-activity-data-june-2026).
- Wallet-level outcomes: across 921 pump.fun coins, only about 20% of trading wallets were in profit 46h after launch, and the top 1% of wallets took half the profit [VENDOR-DATA] — [Alchemii](https://www.alchemii.io/blog/is-meme-coin-trading-profitable). CoinGecko-based figures cited elsewhere show higher profitable-wallet shares on realized PnL only (method conflict) — [BananaGun blog](https://blog.bananagun.io/blog/can-you-make-money-trading-meme-coins).
- Post-graduation mechanics changed in mid-2026. BOOST deploys migration liquidity into automatic buys in the first 5 minutes after migration, with bought tokens burned — [The Block](https://www.theblock.co/news/defi/2026-07-29-pump-fun-token-graduation-rate-jumps-boost-changes-launch-incentives-409815).

### Inferences
- **Encodable entry gate (evidence-weighted).** Prefer tokens whose bonding curve is filling fast, measured as SOL in curve and Δ SOL per minute in the first 2–6 minutes. Combine with structural features: advertised Telegram or website and above-default initial market cap. Drop tokens with no buyer growth after about 5 minutes. Pass all of this through the rug filter in §1. Evidence: [STUDY] for the direction of these features, none for exact cut-points.
- **Do not build a pure copy or KOL-follow entry.** Every controlled test found loses money for followers: all 32 configurations in 2610.39458, −41% per token with a 2-minute delay, and callout medians negative at 1h and 24h. If wallet signals are used at all, use them as a *feature* in the classifier, not as a trigger.
- **Market-cap bands.** No study directly compares entering at $5k–$50k (bonding curve) with post-graduation ($60k–$500k). Indirect evidence: (a) graduation is about 0.2–3%, so a curve-stage entry is a lottery ticket on a roughly 10–15x move to graduation; (b) predictors only work for fast completers; (c) post-migration flows are now partly mechanical (BOOST buys in the first 5 min). Inference: a $1 bot should either (i) enter early in the curve only when fill speed and structure gates fire, or (ii) enter just after graduation only with a post-migration momentum rule. It should not buy the BOOST window blindly, since that buy pressure is mechanical and time-limited. Evidence: [MODEL/inference].
- **De-wash volume before using it.** At least 17% of trades are wash trades, and wash trading raises graduation odds ([arXiv 2609.10246](https://arxiv.org/abs/2609.10246)). Raw volume and buyer counts overstate organic demand, so use unique *funded-by-distinct-sources* buyers.

### Gaps
- No published, out-of-sample entry backtest with realistic latency and costs that is profitable was found, for bonding-curve or post-graduation entries.
- No quantitative study was found on X/Telegram mention velocity or narrative/meta as entry signals (beyond social presence at launch).
- The Marino et al. numeric results (feature importances, AUC) and exact fill-speed thresholds could not be read (arXiv blocked).
- No study compares market-cap entry bands directly.

## 3. Volatility handling: realized-vol measures, ATR stops, slippage vs liquidity, when not to trade

### Takeaway
There is almost no published evidence specific to memecoin stop placement on 1s–1m candles. The quantitative evidence concerns *execution risk*. High slippage tolerance in volatile, illiquid pools invites sandwiching: 6.85M sandwiches took about $29.7M from Solana traders in the year to Oct 2026, and memecoin terminals' users were the most affected. For $1 orders, pool price impact is negligible. Latency slippage, sandwich exposure and gap-through on crashes dominate. Volatility rules should therefore centre on (a) tight, explicit slippage caps with MEV-protected submission, (b) stops sized to realized volatility but accepting gap fills, and (c) "no-trade" conditions.

### Cited Findings
- Bitquery counted **6,850,144 sandwich attacks on Solana (30 Oct 2025 to 7 Oct 2026) by 2,545 bot wallets, extracting 309,010 SOL (about $29.7M)**. Victim losses were about 1.3x the bots' take on PumpSwap [VENDOR-DATA] — [Bitquery](https://bitquery.io/investigations/solana-sandwich-attacks).
- An IMC 2025 measurement on Jito found over 500K sandwiches with over $7.7M in victim losses (early 2025). Sandwiching is rare per transaction but induces widespread defensive behaviour [STUDY, peer-reviewed] — [ACM IMC paper](https://dl.acm.org/doi/10.1145/3730567.3764493); [PDF](https://cnitarot.github.io/papers/imc26_solana.pdf).
- bloXroute, summarizing Ghost's analysis, says wide (multi-slot) sandwiches were 93% of sandwiches and extracted over 529,000 SOL in a year. Validator-run sandwiching now matters more than external bots — [bloXroute](https://bloxroute.com/pulse/a-new-era-of-mev-on-solana/).
- In Oct 2025 Jito banned 15 more validators from JitoSOL stake. Up to 6% of the blocks those validators proposed contained sandwiches, and memecoin traders on Axiom, Bloom and Photon were most affected — [KuCoin news](https://www.kucoin.com/news/flash/jito-bans-15-solana-validators-for-sandwich-attacks-amid-rising-mev-concerns).
- Memecoin traders are especially exposed because they set high slippage tolerance on illiquid, volatile assets. A 10-day sample analysis argues standard loss figures understate victim losses, with drains up 40% on PumpSwap/Raydium and 70% on Pump.fun under its method — [Medium MEV-sandwich report](https://medium.com/@mejordev/solana-mev-sandwich-report-bcc2fcff94ee).
- Liquidity reality at launch: about 30% of fresh tokens had zero trades in the first 15 min, and the median had 3 trades (DEXTools sample) — [DEXTools](https://www.dextools.io/news/pumpfun-new-token-bot-activity-data-june-2026).

### Inferences
- **Price impact at $1 is negligible; latency slippage is not.** On a constant-product curve, the price impact of buying x against quote reserve R is about x/(R+x). With $1 ≈ 0.0067 SOL at an assumed $150/SOL and R on the order of tens of SOL (pump.fun's virtual SOL reserve is about 30 SOL at creation, from background knowledge of the program, not verified here), impact ≈ 0.0067/30 ≈ 0.02%. The relevant costs are therefore (a) the price moving between signal and landing, often several percent in seconds during a pump, and (b) adversarial fills when slippage tolerance is wide. **Rule:** set slippage tolerance to a small multiple of expected latency drift, e.g. 3–5%, not the 20–50% common in terminals. Submit via a protected path (bundle or private relay). Accept a higher failed-buy rate in exchange. A failed *buy* costs only the fee, while a sandwiched buy costs the tolerance. Evidence: [STUDY] for sandwich prevalence; [MODEL] for the sizing.
- **Realized-vol measure.** Compute σ_1m as the std of 1-minute log returns over the last 5–10 one-minute bars, or from 5–10s bars scaled by √(bars per minute). A volatility-scaled stop is `stop = entry × exp(−k·σ_1m·√h)` with h = intended holding minutes and k ≈ 2–3. In my model, the mid-ascent noise level used (8%/√min early) means a 30% trailing stop (ln 0.7 ≈ −0.36) is about 2σ over 5 minutes (0.08·√5 ≈ 0.18). A 30% trail would be whipsawed more often if real early-minute vol is higher. Evidence: [MODEL]. No memecoin-specific ATR study was found.
- **Stops gap through.** Rugs and dev dumps move price 85–98% within one slot. A stop at −40% fills at whatever the next tradable price is. In the simulation, a nominal 0.5x stop produced a *median* outcome of about −0.47 to −0.59 per $1 after costs. Budget for stop-loss fills well below the stop level.
- **No-trade conditions (encodable).** Skip trading when any of these hold:
  - Priority-fee or tip levels spike, so fixed cost per tx is greater than about 2% of position.
  - Curve or pool quote reserve is below a floor, e.g. too thin to exit the position with less than 2% impact; this is rarely binding at $1.
  - Zero or under 3 trades in the first N minutes (dead token).
  - Trade count is dominated by a single bot burst (≥20 trades in seconds, from a few wallets).
  - Wallet RPC latency is degraded.

### Gaps
- No published 1s–1m realized-volatility statistics for pump.fun tokens (by phase: curve vs post-migration) were found.
- No study of ATR- or σ-scaled stops for memecoins was found.
- Typical 2026 Jito tip sizes for memecoin trades were not found. Only a vendor snapshot of average priority fee (about 0.000056 SOL) was located (see §5).

## 4. Exit rules compared under a fat-tailed return distribution (with math and simulation)

### Takeaway
Under fat-tailed, rug-prone distributions calibrated to observed graduation and ≥10x frequencies (pessimistic and base scenarios), **fixed full take-profits (sell 100% at 2x/5x/10x) have strongly negative EV**. They were barely positive (5x/10x only) even in the optimistic scenario. They cap the upside that pays for the roughly 85–95% of entries that lose. Their required hit rates (about 39–43% for 2x with a 50–60% average loss) are far above observed frequencies: about 0.6% of calls reach 10x in 1h, and graduation is about 0.2–3%. Only rules that keep a meaningful fraction exposed to >50x outliers had positive simulated EV, specifically a trailing stop activated early, and only in base or optimistic tails. Even then the edge vanished when the top 0.1% of trades were removed or the price was capped at 50x. Practical implication: if you trade at all, never sell 100% at a fixed target. Recover cost early only if it doesn't kill tail exposure, and trail the rest. But expect the strategy to be negative unless your entry filter genuinely raises P(≥50x).

### Cited Findings
- Real data are tail-dominated. In the Kombucho callout backtest, removing the best 24 of 4,293 calls flips the gross mean negative, and only 0.6% of calls reached ≥10x in 1h — [GitHub Kombucho](https://github.com/Kombucho/memecoin-gamble-research).
- Short-window bot backtests are fragile. A make.fun replay bot (TP +8%, stop −18%, trailing −22%) was positive on its dev window, but 20 rebuilt copies produced a *median loss*, and the author calls the edge fragile — [GitHub gunrflxtion](https://github.com/gunrflxtion/makefun-memecoin-backtest). A dip-buying 5%-TP bot reported a positive 24h result, and also reported results excluding its 3 best coins — [GitHub WhoIsTyfur](https://github.com/WhoIsTyfur/makefun-memecoin-backtest-48h).
- Open-source sniper defaults vary widely: 200% TP / 80% SL ([NateTrades sol-sniper-bot](https://gitea.com/NateTrades/sol-sniper-bot/src/)); 50% TP / 30% SL ([bortoom](https://awesome.ecosyste.ms/projects/github.com%2Fbortoom%2Fsolana-raydium-sniper)); 200% TP / 50% SL ([tnywlf](https://awesome.ecosyste.ms/projects/github.com%2Ftnywlf%2Fsol-sniper-bot)) [PRACTITIONER]. None publish performance evidence.
- Team-following returns "rise several-fold with a stop rule" in hindsight-ranked windows, indicating that exits (stop rules) matter as much as entries in this market — [arXiv 2611.11436](https://arxiv.org/abs/2611.11436).
- Pump.fun-associated team exits are often *early* (the agent-based model in 2610.41597 failed to reproduce the early exits common in the data). This supports fast time stops — [arXiv 2610.41597](https://arxiv.org/abs/2610.41597).

### Inferences

**(a) Analytic: how much of the expected value lives in the far tail.** Model the peak multiple M of a pumping token as Pareto with P(M ≥ m) = (m/m0)^(−α), m ≥ m0 = 1.15. Then E[M] = m0·α/(α−1), and the share of E[M] contributed by tokens with M > K equals (K/m0)^(1−α). Computed values:

| α | E[M \| pump] | share of E[M] from M>10x | from M>50x | from M>100x | from M>1000x |
|---|---|---|---|---|---|
| 1.0 | ∞ | 100% | 100% | 100% | 100% |
| 1.125 | 10.35 | 76.3% | 62.4% | 57.2% | 42.9% |
| 1.25 | 5.75 | 58.2% | 38.9% | 32.7% | 18.4% |
| 1.5 | 3.45 | 33.9% | 15.2% | 10.7% | 3.4% |

The base calibration below uses α ≈ 1.125, chosen so that P(10x) ≈ 2% and P(100x) ≈ 0.15%. Under it, a rule that sells everything by 10x forfeits about 3/4 of the theoretical peak value, and one that sells everything by 100x still forfeits more than half. Mean estimates are also statistically unstable: for α < 2 the variance is infinite, which is consistent with the Kombucho "remove 24 calls and the mean flips" result.

**(b) Break-even hit rates for fixed take-profits.** Costs used: 1.25% pump.fun curve fee per side ([pump.fun docs](https://pump.fun/docs/fees)), 3% latency slippage per side, $0.02 fixed per tx, 2 tx. The net gain at TP k is k·0.9587·0.97·0.9875 − 1 − 0.04. With average loss ℓ, break-even p* = ℓ/(ℓ + win):

| TP | net win per $1 | p* if ℓ=0.3 | p* if ℓ=0.5 | p* if ℓ=0.6 |
|---|---|---|---|---|
| 1.5x | +0.338 | 47.1% | 59.7% | 64.0% |
| 2x | +0.797 | 27.4% | 38.6% | 43.0% |
| 3x | +1.715 | 14.9% | 22.6% | 25.9% |
| 5x | +3.552 | 7.8% | 12.3% | 14.5% |
| 10x | +8.144 | 3.6% | 5.8% | 6.9% |

Compare these with observed frequencies: 0.6% of calls reach 10x in 1h; graduation (about 10–15x from early-curve price) is about 0.2–3%. Realized average losses in the simulation were about 0.5–0.6 because stops gap through. Every fixed-TP row needs a hit rate several times what the evidence supports. A filter would have to raise P(2x) to about 40%, or P(10x) to about 6%, for fixed TPs to break even.

**(c) Monte Carlo path simulation [MODEL, assumption-driven].** Model setup:
- Grid: 5-second steps for the first 10 min, then 1-minute steps to 4h, with the rule evaluated at each step and fills at the step price, so stops gap through.
- Pump share and size: a share p_pump of entries "pump", with the trend peak M ~ Pareto(m0=1.15, α). Time to peak = lognormal around 1.5 + 2.5·log2(M) minutes, so a 1000x takes about 25 minutes and the "1000x in minutes" case is represented.
- Price after the peak: decays to 3–50% of peak by 4h. Non-pumping tokens decay to 3–50% of entry within 2–30 min.
- Noise: 8%/√min for the first 10 min, then 2.5%/√min.
- Rugs: a one-step −85% to −98% jump with probability p_rug, timed near the peak for pumpers.
- Honeypots: with probability p_honey the position is unsellable (−$1.33, since rent and tx are lost).
- Costs: 1.25% fee per side, 3% slippage per side, $0.02 per tx, and rent $0.31 lost only if unsellable.

Scenarios, with the realized 4h path-max frequencies they produced:

| scenario | p_pump, α, p_rug, p_honey | P(max≥2x) | P(≥10x) | P(≥100x) | P(≥1000x) |
|---|---|---|---|---|---|
| pessimistic | 0.07, 1.22, 0.45, 2% | 3.8% | 0.50% | 0.020% | 0.001% |
| base | 0.228, 1.125, 0.35, 1% | 13.4% | 2.05% | 0.16% | 0.017% |
| optimistic | 0.435, 1.00, 0.25, 0.5% | 27.4% | 5.1% | 0.50% | 0.041% |

Results: net PnL per $1 trade, 100k paths per scenario (seed 42). Base is also shown at 200k paths with seeds 1 and 2 for stability.

| exit rule | pessimistic mean | base mean (100k) | base mean 200k s1 / s2 (±se) | optimistic mean | base median | base P(win) | base mean excl. top 0.1% |
|---|---|---|---|---|---|---|---|
| Hold 60 min | −0.77 | +0.36 | −0.07 / −0.15 (±0.07) | +1.89 | −0.87 | 10.7% | −0.42 |
| Sell 100% at 2x, SL 0.5x, 30-min time stop | −0.57 | −0.40 | −0.40 / −0.40 (±0.001) | −0.16 | −0.59 | 16.2% | −0.40 |
| Sell 100% at 5x, SL 0.5x, 60 min | −0.56 | −0.33 | n/a | +0.06 | −0.59 | 11.6% | −0.33 |
| Sell 100% at 10x, SL 0.5x, 120 min | −0.56 | −0.33 | −0.33 / −0.34 | +0.09 | −0.59 | 8.1% | −0.34 |
| Trailing 30% from peak, armed at 1.5x; SL 0.6x; exit if peak < 1.3x at 5 min; 4h max | **−0.35** | **+0.61** | **+0.23 / +0.25 (±0.07–0.09)** | **+2.28** | −0.47 | 12.9% | **−0.07** |
| Trailing 60%, armed at 2x; SL 0.5x; same time stops | −0.43 | +0.17 | −0.10 / −0.07 | +1.12 | −0.49 | 8.2% | −0.28 |
| Ladder A: 50% at 2x, 25% at 5x, 15% at 10x, 10% moonbag trailing 50% | −0.44 | −0.26 | −0.28 / −0.28 | +0.05 | −0.46 | 10.9% | −0.31 |
| Ladder B: 50% at 2x (recover cost), 50% trailing 40% | −0.42 | +0.05 | −0.11 / −0.11 | +0.85 | −0.46 | 12.7% | −0.25 |
| Ladder C: 50% at 2x, 50% trailing 60% | −0.44 | −0.08 | n/a | +0.49 | −0.46 | 12.7% | −0.30 |
| Ladder D: 40% at 3x, 30% at 10x, 20% at 30x, 10% trailing 50% | −0.44 | −0.24 | n/a | +0.12 | −0.48 | 10.0% | −0.30 |

Sensitivity results (base tail, 200k paths, mean per $1):
- **Price capped at 50x**, i.e. the far tail never happens to you: Trail30 drops to −0.056, Ladder B to −0.24, Hold-60 to −0.38. All rules turn negative. The edge is entirely in >50x outcomes.
- **Choppier paths** (16%/√min early, 5%/√min later; p_pump lowered to keep P(10x) about 2%): Trail30 +0.21, Ladder B −0.13, TP2x −0.42. Rankings are unchanged in this model.
- **Cheap execution** (1% slippage per side, $0.005 per tx): Trail30 +0.32, Ladder B −0.04, TP2x −0.34.
- **Expensive execution** (8% per side, e.g. sandwiched or wide tolerance, $0.05 per tx): Trail30 +0.05, Ladder B −0.27, TP2x −0.52.

How to read the simulation:
1. Fixed full TPs were negative in the pessimistic and base scenarios, and only the 5x/10x variants were slightly positive in the optimistic one. In base, the 2x TP had the highest win rate (16%) and the most negative EV (−0.40). In pessimistic, only Hold-60 was worse. The win rate is a misleading KPI.
2. Selling 90% by 10x (Ladder A) is nearly as bad as a fixed TP, because it discards the tail.
3. "Recover cost at 2x, trail the rest" (Ladder B) is a reasonable compromise on variance, but it was negative in base because selling half at 2x halves tail exposure.
4. The best rule in this model was a trailing stop armed early (1.5x), 30% from peak, plus a 5-minute "no-move" time stop and a hard stop. This works because the model places rugs and dumps near the peak, so a trail exits before most of the give-back.
5. **This ranking is model-dependent.** Real pumps often have 30–50% mid-ascent pullbacks (practitioner observation, not quantified in any source found), which would whipsaw a 30% trail. A 40–50% trail, or a volatility-scaled trail (k·σ), is the robust choice to test on real replay data.
6. In every scenario, Monte Carlo means have huge standard errors: ±0.07–0.09 per $1 for Trail30 at 200k paths. A live bot would need tens of thousands of trades to distinguish +0.2 from 0.

**(d) Encodable exit policy (to be validated on replay data before use).**
- Hard rug exits fire immediately, market sell, ignoring other rules: dev or linked-entity sell above X% of supply, LP removal (non-pump.fun pools), freeze or authority change, sell-simulation failure.
- Time stop: if peak < 1.3x after 5 min, exit.
- Initial stop: 0.5–0.6x, accepting gap fills.
- Trailing stop armed at 1.5–2x, trail 30–50% from peak or k·σ_1m-scaled.
- Optional partial: sell at most 25–35% at 2–3x to recover cost. Selling 50%+ early cut simulated EV materially.
- No fixed 100% take-profit.
- Hard horizon (e.g. 4h). Beyond that, the moonbag decision is a separate, unmodelled bet.

### Gaps
- No published real-data comparison of laddered vs trailing vs fixed exits on pump.fun replay data was found. The simulation above is not fitted to real paths.
- No data was found on the distribution of intra-pump drawdowns, which is what determines trailing-stop whipsaw.
- P(100x) and P(1000x) for filtered entries are not published anywhere found. The tails used here are anchored only indirectly (graduation rates, the 0.6% callout ≥10x figure).

## 5. Position sizing, bankroll, Kelly, loss limits, trades/day, and fees at $1 size

### Takeaway
At $1 per trade, fixed per-transaction costs and the token-account rent lock dominate the math. A 2-transaction round trip costs about 12% of a $1 position at a flat price (fees 2.5%, 6% slippage, about 4% network costs). A 5-transaction ladder costs about 18%, and each open position also locks about $0.31 of refundable rent (31% of size). Even in the one positive-EV simulated rule, full Kelly is only about 0.45% of bankroll, which implies a bankroll of about $220 for full Kelly and about $450 for half Kelly at $1 per bet. With a $20–$50 bankroll and flat $1 bets, the simulated probability of ruin was 47–84% *even with positive EV*. The minimum viable trade size, defined as fixed costs ≤ 2% of position, is about $2 for 2-tx round trips and about $5 for 5-tx ladders.

### Cited Findings
- Pump.fun official fees: bonding curve 1.25% per trade (0.30% creator + 0.95% protocol); PumpSwap canonical pool 0.30% (0.05% creator, 0.05% protocol, 0.20% LP); graduation fee 0.015 SOL. Third-party guides disagree on PumpSwap tiers (dynamic or market-cap-tiered creator fees under "Project Ascend", Sep 2025) — [pump.fun docs/fees](https://pump.fun/docs/fees).
- Token account rent: **0.00203928 SOL for a standard 165-byte SPL token account**, a refundable deposit returned when the account is closed — [Raydium docs](https://docs.raydium.io/solana-fundamentals/rent-and-reclaimable-rent).
- Base fee is 5,000 lamports per signature. Priority fee = CU price × CUs. About 80% of user transactions in a recent epoch paid a priority fee averaging about 0.000056 SOL (vendor snapshot) — [Datawallet](https://www.datawallet.com/crypto/solana-gas-fees). Jito tips are a separate transfer instruction, not part of the "fee" field, and must be added to all-in cost — [Bitquery fee anatomy](https://docs.bitquery.io/docs/blockchain/Solana/solana-fee-anatomy/).
- The Kombucho backtest assumes about 7.24% round-trip base cost, and Monte Carlo traders with fixed bet fractions mostly ended below their start — [GitHub Kombucho](https://github.com/Kombucho/memecoin-gamble-research).
- Only about 20% of wallets trading 921 pump.fun coins were in profit at 46h, and the top 1% of wallets took half the profit — [Alchemii](https://www.alchemii.io/blog/is-meme-coin-trading-profitable).

### Inferences
**(a) Cost per $1 trade [MODEL math, assumptions explicit].** Fixed cost per tx: c_tx = (5,000 lamports + priority + tip) × SOL price. With priority about 0.00006 SOL and tip about 0.0001 SOL (tip size assumed, not sourced), c_tx ≈ 0.00017 SOL ≈ $0.025 at $150/SOL. The simulation used $0.02. Round-trip friction at a flat price is 1 − (1−0.0125)²·(1−0.03)/(1+0.03) + n_tx·c_tx/size:

| size | 2 tx (buy + 1 sell) | 5 tx (buy + 4-step ladder) | rent lock as % of size (refundable if ATA closed) |
|---|---|---|---|
| $0.10 | 48% | 108% | 310% |
| $0.25 | 24% | 48% | 124% |
| $0.50 | 16% | 28% | 62% |
| **$1** | **12.2%** | **18.2%** | **31%** |
| $2 | 10.2% | 13.2% | 16% |
| $5 | 9.0% | 10.2% | 6% |
| $10 | 8.6% | 9.2% | 3% |

The proportional part (about 8.2%) is the floor, and is mostly latency slippage; it shrinks only with faster or better execution. Implications:
- **Minimum viable size:** fixed cost ≤ 2% of size requires size ≥ n_tx·c_tx/0.02, i.e. about $2 for 2-tx trades and about $5 for 5-tx ladders.
- At $1, **every extra partial sell costs about 2% of the original stake**, so multi-step ladders are structurally penalized.
- **Always close the token account after a full exit** to reclaim about $0.31 per position. Unsellable or dust positions lose it permanently.
- Rent also caps concurrency for a small wallet: 20 open positions lock about 0.04 SOL (about $6).
- If a third-party terminal (Axiom/Photon/GMGN/Trojan) is used, its per-trade fee (commonly cited around 1%; not verified here) adds to the proportional part.

**(b) Kelly [MODEL].** The growth-optimal fraction f* maximizes E[ln(1 + f·R)] over the simulated per-$1 PnL distribution R (base scenario, 200k paths; minimum R = −1.33 for honeypots):
- Trail30 (mean +0.23 per $1): **full Kelly f* ≈ 0.45% of bankroll**, with growth about 0.00025 per trade. To bet $1 at full Kelly you need a bankroll of about $222; at half Kelly, about $445.
- Ladder B (mean −0.11) and TP2x (mean −0.40): f* ≈ 0, the grid minimum, with negative growth. **Kelly says do not bet a negative-EV rule at any size.**

Kelly is unusually small despite a large mean because the mean comes from rare (<0.2%) outliers, while about 87% of trades lose about 45–60%.

**(c) Risk of ruin with flat $1 bets [MODEL].** Bootstrap from base-scenario per-trade PnL, 4,000 paths. Ruin is defined as bankroll < $1.35, which cannot fund the next trade plus costs.

| rule | start bankroll | trades | P(ruin) | P(end < start) | median end | mean end |
|---|---|---|---|---|---|---|
| Trail30 (+EV) | $20 | 500 | 80% | 81% | $0 | $58 |
| Trail30 | $50 | 500 | 47% | 59% | $18 | $139 |
| Trail30 | $50 | 1,500 | 64% | 65% | $0 | $231 |
| Trail30 | $100 | 500 | 10% | 50% | $100 | $185 |
| Trail30 | $100 | 1,500 | 36% | 45% | $141 | $386 |
| Ladder B (−EV) | $100 | 1,500 | 93% | 96% | $0 | $31 |
| TP2x (−EV) | $100 | 500 | 100% | 100% | $0 | $0 |

The mean bankroll can grow while most accounts are ruined, because positive skew means a few accounts hit outliers. With a $20–$50 start, flat $1 bets on even a +EV rule ruin most accounts before the outliers arrive. Survival needs about 100–200+ units of bankroll per unit bet.

**(d) Daily loss limit under regime changes [MODEL].** Days are drawn as pessimistic (50%), base (40%) or optimistic (10%), with 60 trades/day for 30 days on the Trail30 rule (3,000 paths):

| daily loss limit | mean 30-day PnL | median | P(30d < 0) | trades taken | PnL per trade |
|---|---|---|---|---|---|
| none | +$372 | −$85 | 59% | 1,800 | +0.21 |
| $25 | +$374 | −$89 | 60% | 1,773 | +0.21 |
| $15 | +$531 | +$22 | 48% | 1,415 | +0.37 |
| $8 | +$510 | +$70 | 37% | 942 | +0.54 |

A tight daily stop helps only because losses cluster in bad regimes: it stops trading on bad days. If outcomes were i.i.d., a loss limit would not change EV per trade. Its value therefore depends on regime persistence, which is real here (graduation rate swings from 0.2% to 6.7% between periods), but the magnitude is model-dependent.

**(e) Encodable sizing and bankroll policy.**
- Flat stake s = min($1, f_Kelly/2 × bankroll), estimated on the bot's own realized PnL with heavy shrinkage. Until about 1,000+ trades of live or paper data exist, assume EV ≤ 0 and run paper or minimum size.
- Bankroll ≥ 150–250 × stake.
- Daily loss limit about 8–15 stakes. Weekly kill-switch at about 30% drawdown.
- Max concurrent positions limited by rent and attention, e.g. 10–20.
- Max trades/day set by the filter's pass rate, not by a target.
- Trades needed for statistical confidence: with per-trade SD ≈ σ_R, the SE after n trades is σ_R/√n. In the base simulation σ_R for Trail30 is about 30 per $1 (se 0.067 at n=200k ⇒ σ_R ≈ 30). Distinguishing a +0.2 mean from 0 at 2σ would need n ≈ (2·30/0.2)² ≈ 90,000 trades. Live-performance evaluation of tail strategies is therefore nearly impossible at small n, and paper replay on large historical samples is essential.

### Gaps
- Typical 2026 Jito tip and priority-fee levels for memecoin trades were not found (only one vendor average), and SOL price for Nov 2026 was not checked. All $ conversions assume $150/SOL; rescale linearly.
- Trading-terminal fee levels for 2026 were not verified.
- Rent for Token-2022 accounts with extensions (larger than 165 bytes) was not verified.
- No published study on Kelly or risk of ruin for memecoin bots was found. All sizing numbers are from the model.

## 6. Common failure modes of meme-coin bots

### Takeaway
The documented failure modes are:
- **Following others late** (copy, KOL or callout entries lose 12–50% at the median).
- **MEV extraction from wide slippage** (about $29.7M sandwiched in a year).
- **Unsellable tokens** (Token-2022 hooks, freeze authority, permanent-delegate burns).
- **Fake demand** (at least 17% of trades are wash trades, and wash trading raises graduation odds).
- **Tail-dependent, fragile backtests** that look profitable on a short window.

Smaller but real costs at $1 size are unclosed token accounts (rent), multi-sell ladders and failed transactions.

### Cited Findings
- **Chasing / late entry.** Following loses for 2-minute-delayed copy trades (−41% per token across 93 tokens; −20% per trade for a single wallet) — [TokenPost](https://www.tokenpost.com/news/investing/35409). Earliest-group-signal followers lose out of sample in 32/32 configurations — [arXiv 2610.39458](https://arxiv.org/abs/2610.39458). Callout medians are −12.6% at 1h and −50.3% at 24h even with an optimistic entry at call price — [Kombucho](https://github.com/Kombucho/memecoin-gamble-research).
- **Sandwich / slippage traps.** 6.85M sandwiches and 309k SOL extracted (Oct 2025 to Oct 2026) — [Bitquery](https://bitquery.io/investigations/solana-sandwich-attacks). Memecoin terminal users were most affected, and Jito banned validators — [KuCoin](https://www.kucoin.com/news/flash/jito-bans-15-solana-validators-for-sandwich-attacks-amid-rising-mev-concerns). The cause is high slippage tolerance on illiquid tokens — [Medium report](https://medium.com/@mejordev/solana-mev-sandwich-report-bcc2fcff94ee).
- **Failed sells / stuck tokens.** Transfer hooks and active freeze authority are the most common honeypot mechanisms in one vendor's rejection logs. Permanent delegate allows removal or burning of holder balances — [dev.to Token-2022 traps](https://dev.to/mrwizardlyloaf/token-2022-traps-that-drain-ai-trading-agents-and-how-to-screen-them-33bo); [dev.to permanent-delegate burn](https://dev.to/ohmygod/solanas-permanent-delegate-burn-scam-how-token-2022-extensions-power-2026s-largest-automated-rug-4579).
- **Fake volume.** At least 17% of trades are wash trades, at least 10% of coins are copycats, and wash-traded coins graduate more often — [arXiv 2609.10246](https://arxiv.org/abs/2609.10246). About 24% of new tokens get a bot burst at launch while almost none graduate — [DEXTools](https://www.dextools.io/news/pumpfun-new-token-bot-activity-data-june-2026).
- **Hidden concentration.** True top-10 share is about 24% above the visible figure for high-risk tokens — [Lacuna/MemeTrans](https://lacuna.tiptreesystems.com/work/memetrans-a-dataset-for-detecting-high-risk-memecoin-launches-on-solana/wrk_9bae5b5f543b1331ffd891ca59a05ee0). Metrics like "bundle %" are not comparable across tools — [GMGN blog](https://gmgn.ai/blog/what-is-a-meme-coin-bundle/).
- **Overfit and fragile backtests.** One replay strategy was positive on its dev window and negative at the median across 20 rebuilt copies — [GitHub gunrflxtion](https://github.com/gunrflxtion/makefun-memecoin-backtest). Graduation labels from collectors may not generalize across time windows, and one correction (late graduations) was withdrawn after 0/100 live-state checks — [arXiv 2607.02823v3](https://arxiv.org/html/2607.02823v3).
- **Malicious bot code.** Many public sniper-bot repos are copies of each other, and coverage warns that some are scams or contain malicious code, so configs must be verified against the actual repo [PRACTITIONER] — [awesome.ecosyste.ms listing (bortoom)](https://awesome.ecosyste.ms/projects/github.com%2Fbortoom%2Fsolana-raydium-sniper).
- **Platform-incentive distortion.** Callers have allegedly been paid to promote tiny tokens, which undermines KOL signals — [Protos](https://protos.com/pump-fun-paid-700k-to-callers-shilling-mostly-tiny-tokens/).
- **Regime shifts.** Graduation swung from 0.198% (May–Jun 2026) to a one-day 6.7% (Jul 2026, BOOST) — [arXiv 2607.02823v3](https://arxiv.org/html/2607.02823v3); [The Block](https://www.theblock.co/news/defi/2026-07-29-pump-fun-token-graduation-rate-jumps-boost-changes-launch-incentives-409815).

### Inferences
Encodable guards, one per failure mode:
1. **Slippage and MEV.** Slippage cap 3–5% and protected submission. Never auto-raise slippage on retry.
2. **Unsellable tokens.** Pre-buy sell simulation, plus a post-buy sell simulation within N seconds. If the post-buy simulation fails, flag the token and blacklist its creator and funder cluster.
3. **Rent leakage.** Auto-close token accounts after a full exit, and periodically burn and close dust.
4. **Late following.** Treat wallet, KOL and callout signals as features, never triggers. Reject entries more than about 60–120s after the signal wallet's buy.
5. **Fake volume.** De-wash volume and buyer counts: remove self-funded clusters and same-block bundles.
6. **Overfitting.** Never evaluate on fewer than about 10k historical entries. Always report results with and without the top 0.1–1% of trades, and with the price capped at 50x.
7. **Regime awareness.** Track the rolling graduation rate and the bot's rolling hit-rate. Halt when they fall below the levels the strategy was validated on.
8. **Software supply chain.** Pin and audit any open-source bot code. Never paste private keys into third-party repos or bots.

### Gaps
- No quantitative data was found on failed-transaction rates for memecoin buys and sells (landing rate vs priority fee and tip) in 2026.
- No measured frequency was found for "stuck" tokens or failed sells after a bot passed standard filters.
- Chainalysis and other independent forensic reports for 2025–2026 Solana memecoins were not retrieved. The arXiv full texts (exact feature importances, thresholds) were unreachable from this session, because arxiv.org would need to be allowed in network settings.
