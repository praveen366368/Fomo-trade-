# FOMO (fomo.family): platform profile for a small team planning $1-per-coin meme-coin trades and later automation

Research date: 2026-11-19. Most first-party facts come from fetching fomo.family directly (HTML pages, robots.txt, sitemap.xml, the React Router route manifest, and about 600 production JS chunks downloaded from https://fomo.family/assets/). Some hosts could not be reached from the research sandbox (DNS or proxy blocked): help.fomo.family (the official Help Center, which holds the "Trading fees on fomo" and "Supported chains" articles), prod-api.fomo.family (the app backend), techcrunch.com, fortune.com, protos.com, en.wikipedia.org and datawallet.com. Facts that only came through search-engine summaries of those pages are labelled "(via search summary)". They are weaker than direct fetches.

## 1. What FOMO is: company, founders, funding, launch, scale, mobile vs web

### Takeaway
FOMO ("fomo") is a social, cross-chain, non-custodial trading app run by FOMO Labs Inc., a Delaware corporation. Its co-founders are ex-dYdX staff (Paul Erlanger, Se Yong Park, Prashan Dharmasena). It has raised about $94M: a $2M angel pre-seed (Feb 2025), a $17M Series A led by Benchmark (Nov 2025) and a $75M Series B led by Index Ventures at a reported $550M valuation (Jun 2026). It launched publicly in about May 2025 on iOS/Android and added a full web app at fomo.family on 2026-04-29. The company's own most recent user count is about 2.5M users (Nov 2026).

### Cited Findings
- **Legal entity.** The Terms define the counterparty as "FOMO Labs Inc., a Delaware corporation". They cover the websites at fomo.family, the mobile apps, wallet and trading interfaces, social features and referral programs. Terms last updated October 26, 2026. — [fomo Terms](https://fomo.family/terms)
- **Founders (first names).** "Fomo was co-founded by Paul, Se, and Prashan." The team is described as veterans of Uniswap, dYdX and OpenSea. — [What is fomo (Dec 25, 2025)](https://fomo.family/blog/guide/what-is-fomo)
- **Founders (full names).** Paul Erlanger and Se Yong Park are named as co-founders and former dYdX staff (via search summary of TechCrunch and The Block). — [TechCrunch, Nov 6, 2025](https://techcrunch.com/2025/11/06/why-benchmark-made-a-rare-crypto-bet-on-trading-app-fomo-with-17m-series-a); [The Block](https://www.theblock.co/news/deals/2025-11-06-cross-chain-trading-app-fomo-raises-17-million-in-round-led-by-benchmark-377901)
- **Third co-founder.** Prashan Dharmasena is named as co-founder (via search summary). — [Protos](https://protos.com/crypto-trading-platform-fomo-denies-hack-of-its-ios-app/)
- **Conflict on the founder roster.** Wikipedia reportedly names only Erlanger and Park, other sources list three founders, and RootData reportedly gives 2024 as the founding year rather than 2025 (via search summary). — [Wikipedia: Fomo (platform)](https://en.wikipedia.org/wiki/Fomo_(platform)); [iq.wiki: Paul Erlanger](https://iq.wiki/en/wiki/paul-erlanger)
- **Team background.** "Our team is composed of seasoned operators and engineers from... dYdX, Uniswap, OpenSea, Square, Two Sigma... Most recently, we all worked together at dYdX." — [Fundraising announcement (Feb 18, 2025)](https://fomo.family/blog/news/fundraising-announcement)
- **Pre-seed.** $2M announced February 18, 2025, from "140+ angel investors".
  - Builder and operator angels named: Raj Gokal (Solana co-founder), Balaji Srinivasan, Bryan Pellegrino (LayerZero), Lucas Bruder (Jito Labs), Evgeny Gaevoy (Wintermute), Henri Stern (CEO, Privy), Yu Hu (Kaito), Paul Veradittakit (Pantera), Rob Hadick (Dragonfly), Alexander Pack (Hack VC) and others.
  - Trader angels named: Frankdegods, Threadguy, Cited and others.
  - At that point the product was a closed alpha on TestFlight, with "over 95,000 sign ups on our waitlist".
  - Source: [fomo blog: $2M Angel Pre-Seed](https://fomo.family/blog/news/fundraising-announcement)
- **Series A.** $17M led by Benchmark, bringing total funding to $19M, announced November 6, 2025.
  - The post says that since the "beta launch 6 months ago" fomo had processed "nearly $700M in volume", onboarded "120K+ users and 35K+ traders", facilitated "$5M in onramp volume across nearly 15K net-new crypto users", and was growing "nearly 10% each week".
  - Angels and funds in the round include Coinbase Ventures, Archetype, Harry Stebbings (20VC), Ivan Soto-Wright (MoonPay CEO), Jacob Horne (Zora CEO) and Luca Netz (Pudgy Penguins).
  - Source: [fomo blog: Benchmark leads Series A](https://fomo.family/blog/news/series-a)
- **Series A board seat.** Benchmark partner Chetan Puttagunta reportedly led the round and took a board seat (via search summary). — [TechCrunch](https://techcrunch.com/2025/11/06/why-benchmark-made-a-rare-crypto-bet-on-trading-app-fomo-with-17m-series-a)
- **Series B.** $75M led by Index Ventures, "with strong participation from Union Square Ventures and our existing investor, Benchmark", dated June 22, 2026.
  - "Since our launch a year ago, more than 625,000 people have joined fomo, completing over $4 billion in trading volume and generating more than 110 million unique social interactions."
  - "68,000 users have purchased crypto for the very first time using fomo via Apple Pay, totaling $25 million."
  - Angels include Mark Pincus, Kevin and Julia Hartz, and Humam Sakhnini.
  - Source: [fomo blog: Series B](https://fomo.family/blog/news/fomo-series-b)
- **Series B valuation.** $550M; the company is described as New York-based; total disclosed funding is about $94M (via search summary). — [The Block, Jun 22, 2026](https://www.theblock.co/news/deals/2026-06-22-crypto-trading-app-fomo-raises-75-million-at-550-million-valuation-in-index-ventures-led-series-b-report-405563); [Index Ventures](https://www.indexventures.com/perspectives/on-chain-trading-goes-mainstream-fomos-75-million-series-b/); [Cointelegraph](https://cointelegraph.com/news/fomo-raise-series-b-trading-app-550m-valuation)
- **Launch date.**
  - "Since our launch last May, half a million people have signed up to use fomo on their phones" (posted April 29, 2026), which implies a public launch in May 2025. — [Announcing fomo web](https://fomo.family/blog/news/announcing-fomo-web)
  - Wikipedia reportedly also says May 2025 (via search summary). — [Wikipedia](https://en.wikipedia.org/wiki/Fomo_(platform))
- **Volume milestone.** "We've just crossed $1B in volume" (December 15, 2025). — [Trenches Are "Dead". So How Did fomo Hit $1B Volume?](https://fomo.family/blog/news/trenches-are-dead)
- **Latest user count.** "As of the time of writing, fomo has about 2.5 million users and is backed by Benchmark, Index Ventures, and Union Square Ventures." This is from a fomo-published comparison dated November 2, 2026. — [fomo vs Moby](https://fomo.family/blog/compares/fomo-vs-moby)
- **Referral payouts.** "$16.3M+ in referral fees paid to date" (page fetched 2026-11-19). — [fomo Affiliate Program](https://fomo.family/affiliates)
- **Mobile apps.**
  - iOS listing: "fomo - never miss out", app id 6741115427. Android package: `family.fomo.app`. Both links come from the site footer and download buttons. — [App Store](https://apps.apple.com/us/app/fomo-never-miss-out/id6741115427); [Google Play](https://play.google.com/store/apps/details?id=family.fomo.app)
  - One App Store snapshot showed 4.6/5 from 441 ratings. Third-party trackers show 4.6 from 890, or 4.8 from 15.8K. A Bitcoin.com piece says the app reached the top 5 US finance apps on the App Store by Aug 21 (all via search summary; the figures conflict). — [App Store](https://apps.apple.com/us/app/fomo-never-miss-out/id6741115427); [Bitcoin.com](https://news.bitcoin.com/finance/fomo-trading-app-top-5-us-finance-charts/)
- **Web app.**
  - Launched April 29, 2026: "trade on fomo directly from your desktop using fomo web (fomo.family)... fomo web is not a trading terminal".
  - It uses the same account, balance, positions and social graph as mobile.
  - New web users can fund with crypto. Apple Pay and debit-card onramps require the mobile app.
  - Perps came to web on October 24, 2026.
  - Source: [Announcing fomo web](https://fomo.family/blog/news/announcing-fomo-web)
- **TradingView.** Charts were integrated into fomo web (April 14, 2026). — [TradingView partnership](https://fomo.family/blog/news/tradingview-partnership)
- **Product scope beyond meme coins.**
  - Perpetual futures launched June 11, 2026, "powered by Hyperliquid and Trade[XYZ]", covering crypto, equities, pre-IPO, index and commodity perps. — [Perpetuals now on fomo](https://fomo.family/blog/news/perpetuals-now-on-fomo); [Release notes](https://fomo.family/release-notes)
  - The sitemap lists perp pages for BTC, ETH, SOL, HYPE, NVDA, GOOGL, AMD, MU, SPACEX, gold, silver, oil, natural gas, SP500, JAPAN-225 and XYZ100. — [sitemap.xml](https://fomo.family/sitemap.xml)
- **Current promotion.** An "Anthropic Pre-IPO Perp Trading Competition" started 2026-11-18 at 9:00am ET on the ANT pre-IPO perp market. It excludes US persons and UK residents. — [Competition terms](https://fomo.family/anthropic-trading-competition-terms)
- **Languages.** Blog posts announce localization into German, French, Portuguese, Spanish, Chinese and Japanese (titles in sitemap). — [sitemap.xml](https://fomo.family/sitemap.xml)

### Inferences
- Growth trajectory from fomo's own numbers:

  | Date | Users | Volume |
  |---|---|---|
  | Feb 2025 (waitlist) | about 95K sign-ups | — |
  | Nov 2025 | 120K+ | about $0.7B |
  | Dec 2025 | — | $1B |
  | Apr 2026 | about 500K sign-ups | — |
  | Jun 2026 | 625K+ | $4B+ |
  | Nov 2026 | about 2.5M ("users" in a fomo marketing page) | — |

  The last figure is self-reported and its definition is not stated (sign-ups or active users).
- fomo is a well-funded company backed by top-tier VCs. It is not a fly-by-night bot. That also means it has a legal team and a detailed ToS that explicitly targets automation (Section 5).
- Privy's CEO is a pre-seed angel, which is consistent with fomo's use of Privy wallets (Section 4).

### Gaps
- No independently audited user or volume numbers. Daily or monthly active user counts were not found.
- TechCrunch, Fortune and Wikipedia pages could not be fetched (sandbox DNS block), so founder bios, headcount, revenue and HQ address are not verified beyond search summaries.
- The current App Store and Google Play rating and review count could not be fetched live.

## 2. Chains, tokens, and execution routing (DEXes, routers, launchpads)

### Takeaway
Spot trading runs from one USD/USDC "cash balance". The chain list is Solana, Base, Ethereum, BNB Chain, Monad, Robinhood Chain and Arc (fomo's nav says "seven chains"), plus Elysium in one November 2026 page. Meme coins are mainly on Solana, including pump.fun "pre-bonded" (bonding-curve) and graduated tokens, plus Base (Zora) and BNB. On Solana, fomo's own pages say trades route through DFlow. The web bundle also shows a Relay-based cross-chain/EVM path, a Mobula "solver" intent path, Jito tip transactions and the Helius sender. Explicit per-DEX routing (Jupiter, PumpSwap, Raydium, Meteora) is not documented.

### Cited Findings
- **Chain rollout.**
  - Base launched in September 2025 ("buy and sell any ERC-20 token on Base, deposit and withdraw USDC"). Featured sections at that time were "Zora ecosystem token creators and Pump.fun creator coins". — [September 2025 recap](https://fomo.family/blog/news/september-2025-recap)
  - BNB Chain launched in October 2025. — [October 2025 recap](https://fomo.family/blog/news/october-2025-recap)
  - Monad mainnet launched in November 2025 ("fomo now supports four chains from a single unified balance"). — [November 2025 recap](https://fomo.family/blog/news/november-2025-recap)
  - Arc (Circle's L1, USDC gas) launched September 16, 2026: "deposit, trade and withdraw USDC on Arc". — [Arc on fomo](https://fomo.family/blog/news/arc-on-fomo)
- **Current chain list (conflicting counts).**
  - The fomo vs Axiom page (Nov 13, 2026) lists "Solana, Base, Ethereum, BNB Chain, Monad, Robinhood Chain, Arc, and Elysium". — [fomo vs Axiom](https://fomo.family/blog/compares/fomo-vs-axiom)
  - The fomo vs Binance page (updated Nov 2, 2026) lists seven chains without Elysium. — [fomo vs Binance](https://fomo.family/blog/compares/fomo-vs-binance)
  - The site navigation says "Seven chains, one cash balance". — [fomo.family](https://fomo.family/)
- **Chain IDs in the web bundle.** The chain module defines EVM chain IDs 1 (Ethereum), 8453 (Base), 56 (BNB), 143 (Monad), 4663, 5042 and 99901. The last three are assumed to be Robinhood Chain, Arc and Elysium; the ID-to-name mapping is not verified. Chain names "Arc" and "Elysium" appear explicitly. — [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js) (analysis of `/assets/chains-v2-*.js`)
- **Deposit assets.** "You can deposit USDC into your fomo account on Solana, Base, BNB Chain, Monad, or Ethereum (USDG on Robinhood Chain)". The older guide says "You can ONLY send USDC and have to send it on supported networks." — [fomo vs Phantom](https://fomo.family/blog/compares/fomo-vs-phantom-wallet); [Deposits & Withdrawals guide](https://fomo.family/blog/guide/a-guide-to-deposits-and-withdrawals)
- **Token categories and pump.fun access.**
  - "The home screen surfaces trending, verified, pre-bonded, and graduated tokens." — [fomo vs Phantom](https://fomo.family/blog/compares/fomo-vs-phantom-wallet)
  - "fomo lets users find and trade coins launched on pump.fun, even while those coins are still in their earliest trading phase." — [fomo vs pump.fun](https://fomo.family/blog/compares/fomo-vs-pump-fun)
- **Launchpad support.**
  - "Zora Solana whitelisted as a launchpad" (February 2026), which implies a launchpad whitelist. — [February 2026 recap](https://fomo.family/blog/news/february-2026-recap)
  - October 2026 web release: "Click a token's launchpad icon to open its launchpad page", and "Launchpad tokens now display token-tax warnings". — [Release notes](https://fomo.family/release-notes)
- **DFlow routing (Solana).**
  - "DFlow integration - Routes trades for better execution and MEV protection." — [Which memecoin trading app has the lowest fees? (Feb 8, 2026)](https://fomo.family/answers/memecoin-trading-app-lowest-fees)
  - "Trades execute through DFlow for secure, fair order routing... MEV protection." — [Is fomo app safe?](https://fomo.family/answers/is-fomo-app-safe)
- **Execution paths in client code** (from fomo.family production JS):
  - Swaps go to backend endpoints `/swaps/v2`, `/swaps/v2/fast-fill`, `/swaps/v2/refresh-tx` and `/swaps/v3` on `https://prod-api.fomo.family`.
  - Quote objects carry `source` values `"solana"`, `"relay"` (with `relayTxType` "EVM" or "SOLANA") and `"mobula"` (intents polled via `/swaps/v3/status?provider=mobula`).
  - Request fields include `solanaSwapProvider` and `useMobulaSolver`.
  - Quote fields include `feeTierBps`, `flatFee`, `usdFees`, `priceImpactPct`, `estimatedTaxPct`, `dynamicSlippageBps` and `jitoTipTx`.
  - The bundle references `sender.helius-rpc.com`, `bundler.prod-edge.fomo.family` (ERC-4337 bundler) and `api.hyperliquid.xyz`.
  - Source: [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **"No contract approvals."** fomo says "There's no seed phrase to steal, no contract approvals to sign". — [Is fomo app safe?](https://fomo.family/answers/is-fomo-app-safe)
- **Third-party disclaimer in the Terms.** "The Services facilitate access to and interaction with third-party materials, such as decentralized exchanges ('DEXs')... Fomo does not operate the independent DEXs, blockchains, or settlement protocols." — [fomo Terms §1, §6](https://fomo.family/terms)

### Inferences
- **Pool fees on top of fomo's fee.** fomo appears to be an aggregator-of-aggregators: DFlow on Solana, Relay for cross-chain/EVM, a Mobula solver for some intents. On pump.fun bonding-curve tokens, pump.fun's own protocol fee (1.25% on bonding-curve trades, per fomo's own comparison page) would likely still be paid inside the swap, on top of fomo's fee. This is inferred from how on-chain swaps work. fomo does not state it.
- **Tradable universe.** Tokens on non-whitelisted launchpads may not be tradable or may carry warnings. A strategy that buys arbitrary fresh launches may hit gaps.

### Gaps
- No first-party list of the DEXes and pools fomo routes to (Jupiter, PumpSwap, Raydium, Meteora DBC/DAMM, Uniswap, PancakeSwap, etc.). Jupiter, Raydium and Meteora appear in the JS mostly as educational blog text, not routing config.
- The current launchpad whitelist (beyond pump.fun and Zora/Zora Solana) was not found.
- help.fomo.family "Supported chains" ([link](https://help.fomo.family/en/articles/14436558-supported-chains)) could not be fetched.

## 3. Fees, minimums, deposits/withdrawals, and what a $1 trade really costs

### Takeaway
Standard spot fee is 0.5% per side, or 0.45% with a referral code. Blue chips (BTC/ETH/SOL/BNB) are 0.05%. On Solana, fomo pays gas, priority fees and token rent, but charges "a minimum of up to 0.95 USDC on smaller Solana trades". The percentage fee only takes over at about $190 per trade. On other chains, network fees are added to the quote.

A $1 trade is a poor fit:
- The client code's fallback minimum buy and sell is $2.
- A third-party fee table claims a flat $0.10 fee below $5 (10% per side).
- If the $0.95 minimum applied, it would consume about 95%.

Deposits: Apple Pay, debit card, US ACH, Cash App or USDC. Withdrawals: USDC to a wallet, or a bank (not all regions, up to 3 business days). No explicit withdrawal fee was found.

### Cited Findings
- **Terms on fees.** "Fomo charges a fee on transactions conducted through the Services, including buy and sell transactions. The applicable fee depends on factors including the size and type of the transaction, the token involved, and how the transaction is routed. The fee applicable to your transaction is displayed before you confirm... Fomo may modify its fees at any time." Third-party costs ("liquidity pool fees, network (gas) fees on select blockchain networks", price differences) are separate and may not all be displayed: "Network charges may be incurred even if a transaction fails." — [fomo Terms §7](https://fomo.family/terms)
- **fomo-published fee table (Nov 13, 2026).**
  - "Meme coin and altcoin per spot trade: 0.5% or 0.45% with referral. Minimum of up to 0.95 USDC on smaller Solana trades."
  - "Blue-chip per spot trade (BTC, ETH, SOL, BNB...): 0.05%."
  - "Perpetual futures per side trade: 0.05%, plus venue fee", with venue fee "0.045% venue taker fee or 0.015% venue maker fee".
  - "Gas fees: $0, covered by fomo on Solana."
  - "fees below are for one side of a trade".
  - Source: [fomo vs Axiom](https://fomo.family/blog/compares/fomo-vs-axiom)
- **fomo's own concession on small trades.** "fomo is cheaper on Solana trades above about $127. On smaller trades, fomo's minimum makes Moby cheaper." It also states "Spot trades are market orders only, a minimum fee of up to $0.95 applies to small Solana trades". — [fomo vs Moby (Nov 2, 2026)](https://fomo.family/blog/compares/fomo-vs-moby)
- **Network fees.** "On Solana, fomo also sponsors priority fees, gas fees, and token rent... on other chains, network fees are added to the quote." — [fomo vs Phantom](https://fomo.family/blog/compares/fomo-vs-phantom-wallet); [fomo vs pump.fun](https://fomo.family/blog/compares/fomo-vs-pump-fun)
- **Conflicting gas statement.** The March 2026 security guide says ERC-4337 smart wallets give "Gas sponsorship... trade on Base without holding ETH, or on BNB Chain without holding BNB". — [fomo Security & Wallet Architecture](https://fomo.family/blog/guide/fomo-security-wallet-architecture)
- **In-app fee wording** (production i18n strings):
  - "Enjoy lowest fees on blue chip tokens. A minimum fee is charged on smaller trades to cover blockchain costs."
  - "Lowest fees: 0.05%"
  - "10% off fees with code"
  - "Network fee", "High network fee"
  - "Solana cashback is now labeled 'Trading rebate'" (Oct 2026 release note).
  - Sources: [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js); [Release notes](https://fomo.family/release-notes)
- **Fee display logic in the client.** A quote shows either a flat dollar fee (`flatFee > 0`) or a percentage (`feeTierBps/100`). The fee line is highlighted when total USD fees are ≥ $1 or exceed 10% of the swap's USD value. — [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js) (TokenStats module)
- **Third-party tier table (unverified, referral-code marketing site).** Solana orders "under 5 USDC pay a flat 0.10 USDC, orders from 5 to 47.50 USDC pay 2%, orders from 47.50 to 190 USDC pay a flat 0.95 USDC, and orders of 190 USDC or more pay the percentage rate" (via search summary). The same source says referrers earn 25% of invitees' fees. Neither figure was confirmed in fomo's own pages. — [Datawallet: Fomo referral code](https://www.datawallet.com/crypto/fomo-referral-code)
- **Minimum trade size in client code.**
  - The app-config fallback is `minPurchaseAmountByChain` → default `{buy: 2, sell: 2}` (USD). It can be overridden per chain by the server `/config` response.
  - The quick-buy minimum is `max(2, config.buy)`. The UI shows "Minimum buy is {X}", and the server can reject with `ERR_SWAP_BELOW_MINIMUM`.
  - Custom buy presets must be between $2 and $25,000. Defaults are buy presets [$10, $100, $500, $1,000] and sell presets [10%, 25%, 50%, 100%].
  - Default USDC transfer minimums are $0.50 in and out, and $5 in and out on one specific chain (likely Ethereum).
  - Source: [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js) (datadog and settings-gear modules)
- **Other errors and slippage.**
  - Swap errors include `ERR_SWAP_PRICE_IMPACT_TOO_HIGH`, `ERR_SWAP_NETWORK_FEE_TOO_HIGH` and `ERR_SWAP_SIMULATION_REVERTED`.
  - Slippage is "'Auto' adjusts the tolerance for each token based on market and network conditions. 'Custom' uses your chosen limit." The maximum custom slippage constant is 9,900 bps (99%).
  - Source: [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Presets.** The December 2025 recap introduced "Dollar preset buy amounts... ($10, $25, $50, etc.)". — [December 2025 recap](https://fomo.family/blog/news/december-2025-recap)
- **Small accounts.** "No minimum deposit"; for a $50 account, fomo suggests "$10-15 per trade". — [Is $50 too little to start?](https://fomo.family/answers/is-50-dollars-too-little-to-start)
- **Deposit methods.**
  - Debit card, Apple Pay and crypto (USDC). — [Deposits & Withdrawals guide](https://fomo.family/blog/guide/a-guide-to-deposits-and-withdrawals)
  - "Native Apple Pay onramp through Coinbase" (January 2026). — [January 2026 recap](https://fomo.family/blog/news/january-2026-recap)
  - Cash App (Aug 31, 2026; "not available in every region"). — [Cash App deposits](https://fomo.family/blog/news/cash-app-deposits-on-fomo)
  - US-bank ACH (Oct 12, 2026; "If this is your first bank deposit, fomo may ask you to verify your identity"). — [Bank deposits](https://fomo.family/blog/news/bank-deposits-on-fomo)
- **Onramp providers.** The Terms name Mesh Connect as the "Connection Provider" for connecting exchange accounts and wallets. The JS references MoonPay, Stripe onramp, Coinbase onramp and Crossmint token checkout (which has per-chain minimum and maximum amounts). — [fomo Terms §6](https://fomo.family/terms); [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Withdrawals.**
  - To a bank: "not supported in all regions". To crypto: USDC to a wallet address, "Give it a couple of seconds". — [Deposits & Withdrawals guide](https://fomo.family/blog/guide/a-guide-to-deposits-and-withdrawals)
  - Bank withdrawals: "funds arrive in your bank within 3 business days"; "First-time withdrawals may require additional verification". — [How long to withdraw](https://fomo.family/answers/how-long-to-withdraw-from-fomo)
  - The UI warns against withdrawing to Robinhood, PayPal, Venmo or Wealthsimple app accounts ("permanent loss of funds"), and shows "a clearer message when a withdrawal is below the minimum". — [Release notes](https://fomo.family/release-notes); [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Free transfers.** "You can also send tokens to other fomo users for free." — [fomo vs Phantom](https://fomo.family/blog/compares/fomo-vs-phantom-wallet)
- **Perps minimums and fees.** "Minimums: 5 USDC into your perps balance, 15 USDC per position (including leverage)". Transfer to the perps balance: "fomo charges no fee". — [fomo Perps Guide](https://fomo.family/blog/perps/fomo-perps-guide)

### Inferences
- **Arithmetic for a $1 Solana meme-coin trade** (illustrative; the server-side minimum and fee tiers could not be confirmed live):

  | Fee scenario | Buy fee | Round trip | Share of $1 |
  |---|---|---|---|
  | Nominal 0.5% (not what applies at this size) | $0.005 | $0.01 | 1% |
  | Third-party "flat $0.10 under $5" tier | $0.10 | about $0.20 | about 20% |
  | $0.95 minimum fully applied | $0.95 | — | about 95% on entry |

  On top of the fomo fee: slippage and price impact on thin bonding-curve tokens (often 1–10%, per fomo's slippage explainer), plus pump.fun's 1.25% curve fee if routed through the curve.
  - Under the flat $0.10 tier, a token must gain roughly 20–25% just to break even.
  - The $0.95 minimum only "breaks even" with the percentage fee at $190 per trade (0.95 / 0.005).
- **Likely blocks on $1 orders.**
  - The client fallback minimum is $2 for both buy and sell, so a $1 order is likely blocked in the UI unless server config sets a lower per-chain minimum. Verify in-app before planning around $1.
  - The same $2 sell minimum suggests a position that decays below about $2 may not be sellable in-app ("dust"). You would need to export keys or wait.
- **Base, BNB and Monad.** These chains may avoid the Solana-specific flat minimum but add network fees to the quote. Whether they carry their own flat minimums is unknown.
- **Referral code.** Only a 10% discount (0.5% to 0.45%), which is negligible for $1 trades.

### Gaps
- The official help article "Trading fees on fomo" ([help.fomo.family/en/articles/14436214](https://help.fomo.family/en/articles/14436214-trading-fees-on-fomo)) and "How referral codes and fee discounts work" ([link](https://help.fomo.family/en/articles/15171268-how-referral-codes-and-fee-discounts-work)) were unreachable. The exact tiered flat-fee schedule below $190 remains unverified first-party.
- The live server `/config` (per-chain min purchase and transfer amounts) could not be fetched: prod-api.fomo.family was blocked by the sandbox proxy.
- No explicit statement of a withdrawal fee (crypto or bank) was found. The card and Apple Pay onramp spread or fee (Coinbase/MoonPay/Stripe) was not found.

## 4. Wallet model: custody, provider, key export, address visibility

### Takeaway
Wallets are non-custodial Privy embedded wallets. Keys are sharded (Shamir) and reassembled in TEEs. EVM chains use ERC-4337 smart wallets with a bundler. Users sign in with email, Apple ID or Google, can add MFA (authenticator or passkey) and FaceID, and can export private keys through a Privy flow at fomo.family/export-key. Users can see and copy their own deposit address. Whether a user's wallet address is shown publicly on their profile is unverified, but profiles do publicly show portfolio, positions, cash balance and trades.

### Cited Findings
- **Privy in the Terms.** "Wallet functionality uses Privy, a third-party wallet infrastructure provider... The Fomo platform and Services do not custody user assets. Fomo does not guarantee the availability of any export, recovery, or access method, as such availability may be subject to Privy functionality." — [fomo Terms §6](https://fomo.family/terms)
- **Privy in comparisons.** "fomo uses a self-custodial wallet powered by Privy. You control your funds, the app cannot move them, and you can export your private keys at any time." — [fomo vs Moby](https://fomo.family/blog/compares/fomo-vs-moby)
- **Key architecture.**
  - "Your private key is split into multiple shares using... Shamir's Secret Sharing... Keys are only reassembled inside secure hardware environments (trusted execution environments, or TEEs)."
  - "On EVM-compatible chains (like Base and BNB Chain), fomo uses smart contract wallets that comply with the ERC-4337 account abstraction standard."
  - "FaceID is used to open the app, authorize withdrawals, and authorize private key exports."
  - "Only your wallet address and transaction data are on-chain."
  - Source: [fomo Security & Wallet Architecture (Mar 4, 2026)](https://fomo.family/blog/guide/fomo-security-wallet-architecture)
- **MFA.** Launched September 9, 2026, using an authenticator app or passkey. It is required at login, "Before every withdrawal" and "Before you export your keys". — [MFA on fomo](https://fomo.family/blog/news/mfa-on-fomo)
- **Key export route.** The web app has an `export-key` route (`/export-key`, disallowed in robots.txt). UI strings include "You'll continue to Privy to export keys", "Export keys", "I understand that fomo cannot help me recover my funds if I lose my private keys", and "You must export your private keys to access your funds." — [fomo robots.txt](https://fomo.family/robots.txt); [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Key-export warnings in the Terms.** "An exported key may continue to control the wallet outside Fomo... Availability of key export does not guarantee continued access through Fomo." — [fomo Terms §6](https://fomo.family/terms)
- **Own deposit address.** The deposit flow shows "a QR code and wallet address to which you can send the assets". — [Deposits & Withdrawals guide](https://fomo.family/blog/guide/a-guide-to-deposits-and-withdrawals)
- **Arc address.** "Your account already has an Arc address." — [Arc on fomo](https://fomo.family/blog/news/arc-on-fomo)
- **Solana wallet.** "fomo creates a Solana wallet that belongs to you, and on Solana it pays the network gas, priority fees and token rent." — [Cashtags on X](https://fomo.family/blog/news/x-cashtags-on-fomo)
- **Public profile contents.** Profiles show "portfolio chart, current open positions, cash balance, and recent trades placed". Theses display "the trader's PnL and balance for full transparency". — [Leveraging fomo's Social Features](https://fomo.family/blog/guide/leveraging-fomos-social-features); [February 2026 recap](https://fomo.family/blog/news/february-2026-recap)
- **Fee-payer signing (Solana).** fomo-originated Solana transactions are signed by a fomo "fee payer". The co-founder used this in the August 2026 dispute ("the wallet in question has no transactions signed by FOMO's fee payer") (via search summary). The client code also carries `feePayerSignature` and `feePayerAddress` on relay transactions. — [Protos](https://protos.com/crypto-trading-platform-fomo-denies-hack-of-its-ios-app/); [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Activity outside fomo breaks tracking.** "fomo does not support tracking your activity on other platforms. Your trades and portfolio may become inaccurate and you may be removed from the leaderboard." The competition terms add: "If you trade the Eligible Market from the same wallet outside fomo... we may exclude you from the rankings." — [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js); [Competition terms §4.6](https://fomo.family/anthropic-trading-competition-terms)

### Inferences
- **On-chain linkability.** Because every fomo trade is an on-chain transaction from the user's own address, and fomo sponsors fees from an identifiable fee-payer account on Solana, a fomo user's activity is linkable on-chain by anyone who knows or derives the address. A team's wallets would be publicly observable.
- **Using an exported key.** You could trade the same wallet from a script, but fomo says this makes in-app PnL and leaderboard tracking inaccurate. It may also be flagged under the ToS (Section 7).

### Gaps
- Whether the web profile page displays the user's wallet address publicly was not confirmed. Profile data is client-rendered from the API, which was unreachable, and the bundle's "Copy address" buttons observed were for token addresses.
- Exact Privy configuration was not determined: whether the Solana wallet is a plain embedded EOA, and whether EVM smart-wallet keys can be exported as a single EOA key.

## 5. API, SDK, webhooks, automation and order types

### Takeaway
There is no official public API, SDK, webhook or developer documentation. The ToS explicitly bans bots, scripts, third-party trading extensions, session-token sharing and scraping "except through functionality we expressly authorize". Inside the app:
- Spot trading is market orders only: no limit, stop-loss, take-profit, DCA or auto-sell on spot.
- There is no copy trading. "Follow" only sends notifications, and you tap to trade.
- Automation-like order types exist only on perps (TP/SL, limit close, limit orders, scale orders, trailing stops), and perps are not available to US persons.

The realistic automation path is to trade the same tokens directly on-chain from your own wallets (or an exported fomo key) through public DEX and aggregator APIs, outside fomo.

### Cited Findings
- **Automation ban (§16 Prohibited Uses).** Users may not:
  - "Use automated scripts, bots, software, or any other automated means to create accounts, send messages, post content, execute trades, or otherwise control or interact with account activity on the Services, except through functionality we expressly authorize"
  - "Use or permit a third-party trading extension, bot, script, or other trading or data-extraction tool to access or interact with the Services, retrieve data, or submit transactions, except through an integration or functionality expressly authorized by Fomo. You must not supply, disclose, transfer, or permit a third party to obtain or use your authentication tokens, session tokens, session cookies, API keys..."
  - "Use any automated tool, scraper, crawler, spider... to extract... user-generated content, transaction data, pricing information, or account information"
  - "Attempt to decipher, decompile, disassemble, or reverse-engineer any of the software"
  - Source: [fomo Terms §16](https://fomo.family/terms)
- **Enforcement (§4).** "If we determine or suspect that your account or credentials are being used through an unapproved tool... we may... disable or revoke affected credentials or sessions and restrict, suspend, or terminate your account." — [fomo Terms §4](https://fomo.family/terms)
- **No copy trading.** "fomo does not offer copy trading. You can follow traders on the 30-day leaderboard and trade with one tap. Spot trades on fomo are market orders only. Limit, stop-loss, and take-profit orders are available only on perpetual futures, which are not offered to US users." — [fomo vs Moby (Nov 2, 2026)](https://fomo.family/blog/compares/fomo-vs-moby)
- **Follow does not execute.** "Following a trader does not automatically execute their orders in your account." — [About fomo](https://fomo.family/about)
- **"Copy" means manual.** fomo calls its model "copy trading" in the sense of "you decide whether to execute each trade". It contrasts this with "Mirror trading? automatically replicates trades without your input". — [How does copy trading work?](https://fomo.family/answers/how-does-copy-trading-work)
- **Marketing wording conflict.** The homepage meta description says "Follow traders, copy trades". — [fomo.family](https://fomo.family/)
- **Perps order types.**
  - "Market orders to open; take profit, stop loss and limit close on open positions". — [fomo Perps Guide](https://fomo.family/blog/perps/fomo-perps-guide)
  - Scale orders and trailing stops ("Set a trailing stop to trigger a closing order when the price reverses by your chosen percentage") were added November 2–15, 2026. — [Release notes](https://fomo.family/release-notes)
- **Notifications** (the closest thing to signals):
  - Followed traders' buys and sells "filter the notifications by trade size (e.g., $1000 minimum)", plus price alerts, trending activity and top-trader activity. — [Leveraging fomo's Social Features](https://fomo.family/blog/guide/leveraging-fomos-social-features)
  - "Quick buys get you into tokens faster" on web. — [Announcing fomo web](https://fomo.family/blog/news/announcing-fomo-web)
- **Internal backend** (undocumented; observed in production JS, not a public API). Using it outside the official clients would breach §16.
  - REST base `https://prod-api.fomo.family`, with paths such as:
    - `/public/prices`
    - `/config`
    - `/swaps/v2`, `/swaps/v3`
    - `/v2/users/{id}/swaps`, `/v2/users/{id}/activity`, `/v2/users/{id}/followers`, `/v2/users/userHandle/{handle}`, `/v2/users/fuzzy-search`
    - `/v2/leaderboard/{window}`, `/v2/leaderboard/trades?pnlType=realized&window=...&sizeClass=...`
    - `/v2/clans/leaderboard`
    - `/v1/perps/feed`
  - A WebSocket at `wss://prod-api.fomo.family/ws` and auth origin `https://auth.fomo.family`. Many `/api/v1/...` paths in the bundle belong to the Privy SDK, not to fomo.
  - Source: [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **No developer surface on the site.** The sitemap (200 URLs) and footer list Product, Prices, Learn, Company and Legal pages. There is no developer, API or docs section. — [sitemap.xml](https://fomo.family/sitemap.xml)
- **fomo's own view of bots.** It publishes a guide to Solana trading bots (Telegram bots like Trojan, web terminals like Axiom, copy-trading and sniping bots) but does not offer one. — [Best Solana Trading Bot (Feb 7, 2026)](https://fomo.family/blog/compares/best-solana-trading-bot)
- **Competitors with spot automation** (per fomo's own pages). Moby offers "limit, stop-loss, take-profit, and DCA on spot". Axiom offers a desktop terminal with "hotkeys", "wallet tracking", and "MEV protection modes". — [fomo vs Moby](https://fomo.family/blog/compares/fomo-vs-moby); [fomo vs Axiom](https://fomo.family/blog/compares/fomo-vs-axiom)

### Inferences
- **No sanctioned automation path.** Automating trades through the fomo app or web (UI automation, replaying API calls, session-token reuse) is explicitly prohibited. It risks credential revocation or account termination, and "authorized" integrations are not published.
- **Compliant alternative.** Run automation from wallets the team controls directly, for example via DEX or aggregator APIs on Solana (Jupiter, PumpSwap) and Base. Use fomo only manually for discovery and social signals.
  - Trading an exported fomo key from a bot is technically possible on-chain. fomo warns it breaks in-app PnL and leaderboard tracking, and it could still be construed as coordinated or deceptive activity (§9).
- **Signals without scraping.** Using fomo profiles or leaderboards as a signal source for a bot would require scraping, which §16 prohibits. A manual human-in-the-loop "watch, then trade elsewhere" workflow avoids that.

### Gaps
- No evidence of any "expressly authorized" third-party integration program, partner API, or webhook. Only the TradingView charts and X cashtag "Trade on fomo" deep-link integrations were found.
- help.fomo.family could not be searched for automation or API articles.

## 6. Social features: profiles, feed, leaderboards, clans, referrals, web URLs

### Takeaway
fomo's core is social:
- A global and a Following feed of real trades with entry and exit prices and PnL.
- "Theses" (trade commentary), comments and likes.
- Follows, mutuals and notifications.
- Leaderboards with Traders, Positions and Clans tabs, over 24h, 7D, 30D and all-time windows.
- Clans of up to 150 members.
- Creator rewards, plus a referral and affiliate program (10% fee discount for invitees; $16.3M+ referral fees paid).

Web URL formats are `fomo.family/profile/<handle>` (`/u/<handle>` redirects), `/tokens/<chain>/<address>`, `/leaderboard`, `/clans/<id>` and `/r/<refCode>`. These routes sit under an authenticated layout and robots.txt disallows them. Whether they show data logged-out is unverified.

### Cited Findings
- **Feed contents.** "Coins purchased, Amount purchased, Amount sold, When a position is fully closed, Price change since purchase, Total profit/loss". — [Leveraging fomo's Social Features (Dec 25, 2025)](https://fomo.family/blog/guide/leveraging-fomos-social-features)
- **Leaderboard windows.** "The leaderboard displays the top traders... across multiple time frames (24h, 7D, 30D, All-time)." — [Leveraging fomo's Social Features](https://fomo.family/blog/guide/leveraging-fomos-social-features)
- **Leaderboard redesign.**
  - October 2026: "revamped leaderboard with dedicated tabs for Traders, Positions and Clans". — [Release notes](https://fomo.family/release-notes)
  - November 2026: "Browse the Positions leaderboard and filter by amount invested". — [Release notes](https://fomo.family/release-notes)
  - fomo also refers to "the 30-day leaderboard". — [fomo vs Moby](https://fomo.family/blog/compares/fomo-vs-moby)
- **Multichain rankings.** "The leaderboard now aggregates performance across Solana, Base, and BSC... Profit and loss calculations are also multichain." — [September 2025 recap](https://fomo.family/blog/news/september-2025-recap)
- **Leaderboard verification claim.** "All trading data is verified on-chain, so the rankings reflect actual performance." — [What are crypto trading leaderboards?](https://fomo.family/answers/what-are-crypto-trading-leaderboards)
- **Client API path for leaderboard trades.** `/v2/leaderboard/trades?pnlType=realized&window=…&sizeClass=…`. — [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Theses and comments.**
  - Theses launched January 2026 on token feeds and went global in February 2026. — [January 2026 recap](https://fomo.family/blog/news/january-2026-recap); [February 2026 recap](https://fomo.family/blog/news/february-2026-recap)
  - Nested comment replies and a 200-character limit (December 2025). — [December 2025 recap](https://fomo.family/blog/news/december-2025-recap)
  - Automatic thesis translation (October 2026). — [Release notes](https://fomo.family/release-notes)
- **Profile and follow features.**
  - "Other users' activity is now visible on their profiles" (December 2025). — [December 2025 recap](https://fomo.family/blog/news/december-2025-recap)
  - Profile header with "average hold time stats" and "Mutual followers" (November 2025). — [November 2025 recap](https://fomo.family/blog/news/november-2025-recap)
  - Web "Profile feeds: See a trader's full activity and theses directly on their profile" (November 2026). — [Release notes](https://fomo.family/release-notes)
- **Clans.** Launched August 19, 2026.
  - "One trader belongs to one clan. A clan holds up to 150 members."
  - "Invite four traders to make the clan official."
  - Clan PnL over 24h, 7d and 30d, holdings, a feed, a thesis tab, and a clans tab on the leaderboard.
  - Source: [Clans on fomo](https://fomo.family/blog/news/clans-on-fomo)
- **Referrals (Terms).** Referral codes route "a portion of an eligible Fomo fee" to the referrer via smart contracts. fomo "may reduce any share to zero". Self-referral, "multiple identities or accounts" and coordinated artificial activity are prohibited, as are paid ads or keyword bidding on the fomo brand. — [fomo Terms §8.3](https://fomo.family/terms)
- **Referrals (rates).** "Sign up with a referral code and the standard 0.5% fee drops to 0.45% for the life of the account." — [fomo vs Moby](https://fomo.family/blog/compares/fomo-vs-moby)
- **Affiliate program.** "$16.3M+ in referral fees paid to date". The link format is shown as "fomo.family/r/yourname", payouts are "processed in real time when a trader makes a swap", and there are Silver, Gold and Platinum tiers. — [Affiliates](https://fomo.family/affiliates)
- **Creator rewards.** Covered by Terms §8.2. Creator rewards expanded to perps in October 2026 ("Earn more by trading actively, posting your thesis and building a following"). — [Release notes](https://fomo.family/release-notes); [fomo Terms §8.2](https://fomo.family/terms)
- **Web routes** (from the production route manifest):
  - Profiles: `profile/:handle`, `u/:handle`, `user`
  - Tokens: `tokens/:chain/:tokenAddress`, `x/solana/:tokenAddress` (X cashtag)
  - Other: `leaderboard`, `discover`, `perps/:slug`, `clans/:clanId`, `clans/join/:token`, `r/:refCode`, `ref/:refCode`, `export-key`
  - The profile, token, leaderboard, discover and clan routes are children of `layouts/authenticated`.
  - Source: [fomo route manifest](https://fomo.family/assets/manifest-90574f0d.js)
- **Live check of profile URLs.**
  - Fetching `https://fomo.family/profile/fomo` returned HTTP 200 with `<title>@fomo on fomo</title>` and description "Follow @fomo's trades and portfolio on fomo, the social crypto trading app." — [fomo.family/profile/fomo](https://fomo.family/profile/fomo)
  - `/u/fomo` and `/leaderboard` returned the generic shell (client-rendered).
  - robots.txt disallows `/profile/`, `/u/`, `/user`, `/token`, `/coin`, `/r/`, `/ref/` and `/export-key` for general crawlers (Twitterbot is allowed). — [robots.txt](https://fomo.family/robots.txt)
- **No logged-out data in SSR.** The UI string "Trading and wallet actions are not available in this preview." suggests a logged-out preview mode exists. — [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
- **Metrics disclaimers (Terms §9).** "Metrics may be estimates and may differ from realized, withdrawable returns... users should not interpret any communications, social posts, or blue 'checkmarks' in the Services as investment advice." Manipulating metrics through "selective transfers of tokens between wallets, wash trading... coordinated activity with other accounts" is prohibited. — [fomo Terms §9](https://fomo.family/terms)

### Inferences
- **Value for the team.** A small team could use a fomo clan as its shared PnL and holdings page and leaderboard identity. All members' trades would then be public.
- **Coordination risk.** If a 20-member team coordinates buys of the same coins, fomo could interpret it as the "coordinated activity with other accounts" that §9 and §16 prohibit, especially if it moves leaderboard metrics.
- **No logged-out scraping target.** Profile URLs are shareable and have OG previews, but the data is client-rendered via the authenticated API. robots.txt disallows them, so they are not a sanctioned scraping target.

### Gaps
- Whether profiles, leaderboards and trade histories are viewable without logging in on the web was not confirmed. The pages are client-rendered and the API host was unreachable.
- The current referrer fee-share percentage is unverified. The UI uses a variable `feeSharePct`; one third-party claim says 25%.
- Leaderboard PnL methodology (realized vs unrealized, inclusion of transfers) is only partially documented.

## 7. Terms of Service: bots, scraping, copy trading, geography (India, US), KYC

### Takeaway
The ToS (updated Oct 26, 2026):
- Bans bots, scripts, third-party trading extensions, scraping and VPN or geo-evasion.
- Bans account sale or transfer, multi-account referral abuse and coordinated metric manipulation.
- Excludes sanctioned jurisdictions (the Crimea, Donetsk and Luhansk regions, Cuba, DRC, Syria, Iran, Libya, Myanmar, North Korea, Sudan, Venezuela, Yemen).

India is not named as restricted. Spot is available to US persons. Perps are barred for US persons, and tokenized stocks are barred for US, Canada, UK and Switzerland. Spot needs no upfront KYC, but fomo "may request information to verify eligibility". KYC may be required for perps, a first bank deposit or a first withdrawal, and by third-party onramps. Users must be 18+.

### Cited Findings
- **Restricted jurisdictions.** "THE SERVICES... ARE NOT OFFERED TO RESTRICTED PERSONS OR TO INDIVIDUALS OR ENTITIES WHO RESIDE IN... THE CRIMEA, DONETSK OR LUHANSK REGIONS OF UKRAINE, CUBA, THE DEMOCRATIC REPUBLIC OF CONGO, SYRIA, IRAN, LIBYA, MYANMAR (BURMA), NORTH KOREA, SUDAN, VENEZUELA, OR YEMEN, OR ANY OTHER TERRITORY IN WHICH ACCESS... IS PROHIBITED BY APPLICABLE SANCTIONS OR EXPORT CONTROL LAWS". Also: "Fomo may also impose additional commercial or product restrictions at any time." — [fomo Terms §1](https://fomo.family/terms)
- **VPNs prohibited.** "You must not use a VPN, proxy, false identity, or other means to evade applicable eligibility, sanctions, or product restrictions." — [fomo Terms §1, §16](https://fomo.family/terms)
- **Age.** "you are at least eighteen (18) years old". — [fomo Terms §1](https://fomo.family/terms)
- **US access.**
  - Spot: "fomo is a mobile-first app available to US persons for spot trading". — [fomo vs Axiom](https://fomo.family/blog/compares/fomo-vs-axiom)
  - Perps: "Perps functionality is not available to U.S. Persons". — [fomo Terms §10](https://fomo.family/terms)
  - Tokenized assets are not available to "(a) U.S. Persons... (b) residents of, or persons located in, Canada, the United Kingdom, or Switzerland". — [fomo Terms §11](https://fomo.family/terms)
- **UK.** A UK FCA-style "UK Risk Summary" page exists (high-risk warning, no FSCS or FOS protection). — [UK Risk Summary](https://fomo.family/uk-risk-summary)
- **General availability.** "fomo is a mobile-first crypto social trading app available in most countries." — [fomo vs Axiom](https://fomo.family/blog/compares/fomo-vs-axiom)
- **India.**
  - India is not in the Terms' named restricted list. — [fomo Terms](https://fomo.family/terms)
  - Third-party guides say fomo restricts "12 named jurisdictions and any country that sanctions or bans crypto". Search summaries did not show India named (via search summary). — [Coinperps: Fomo restricted countries](https://www.coinperps.com/learn/fomo-restricted-countries); [Datawallet: Fomo restricted countries](https://www.datawallet.com/crypto/fomo-app-restricted-countries)
- **KYC and verification.**
  - "Fomo may request information to verify eligibility, restrict access while reviewing that information". — [fomo Terms §1](https://fomo.family/terms)
  - "Where required for a Third-Party Service, you must: Provide accurate and complete personal identity information when requested". — [fomo Terms §6](https://fomo.family/terms)
  - "We reserve the right to require identity verification and/or know-your-customer ('KYC') documentation as a condition of accessing Perps". — [fomo Terms §10](https://fomo.family/terms)
  - First US bank deposit "may ask you to verify your identity". — [Bank deposits](https://fomo.family/blog/news/bank-deposits-on-fomo)
  - The UI includes the string "Verify your identity to continue". — [fomo web JS bundle](https://fomo.family/assets/manifest-90574f0d.js)
  - Third-party summary: "KYC: None for spot. Card top-ups are checked by a payment partner" (via search summary). — [Datawallet](https://www.datawallet.com/crypto/fomo-app-restricted-countries)
- **Bots, scraping and reverse engineering.** All prohibited; see Section 5 quotes. — [fomo Terms §16](https://fomo.family/terms)
- **Team and agency use.**
  - "If others are using the Services on your behalf, you accept responsibility for the actions and inactions of all such persons as if they were your own". — [fomo Terms §1](https://fomo.family/terms)
  - Prohibited: "Use the Services for any unauthorized commercial purpose or the benefit of a third party". — [fomo Terms §16](https://fomo.family/terms)
  - Prohibited: selling or transferring "any account, username, handle". — [fomo Terms §5, §16](https://fomo.family/terms)
  - Prohibited: "use multiple identities or accounts to obtain referral benefits, coordinate artificial activity". — [fomo Terms §8.3(f)](https://fomo.family/terms)
- **Market manipulation.** Prohibited: "pump and dump schemes, wash trading, self-trading, front-running, quote stuffing, spoofing, or layering". — [fomo Terms §16](https://fomo.family/terms)
- **Disputes.** Individual arbitration, class-action and jury-trial waivers, and limited time to file claims. — [fomo Terms §19–21](https://fomo.family/terms)

### Inferences
- **India.** Nothing in fomo's ToS bars Indian residents from spot trading. However, the US-centric funding rails (Apple Pay via Coinbase, ACH, Cash App) and bank withdrawals may be unavailable in India, which would leave crypto (USDC) deposits and withdrawals as the main rails. Indian users also bear Indian crypto tax and reporting obligations, which are outside the scope of these notes.
- **Team structure.** A team of several people each running their own account is allowed. Running many accounts from one operator, sharing credentials, or automating them is not.

### Gaps
- No official country-by-country availability list for spot, onramps or bank withdrawals was found. The help center was unreachable.
- No statement on whether fomo geoblocks India or any India-specific restriction exists (none found).
- No explicit ToS clause about copy-trading services (offering signals to others) beyond the general "benefit of a third party" and promotion rules.

## 8. Incidents, criticisms, security issues, reviews

### Takeaway
There is no confirmed breach.
- In 2026 (reported around August), a trader alleged a fomo iOS update drained "about $6M". Co-founder Prashan Dharmasena denied it, saying the cited wallet had no transactions signed by fomo's fee payer. ZachXBT tied the accuser to ZKasino figures. The claim remains unsubstantiated.
- A typosquat phishing site (fomoo.family) has been reported.
- Criticisms in fomo's own and third-party materials: high effective fees on small Solana trades (the minimum fee), market-orders-only spot, no copy-trading automation, and a sparse, unclaimed Trustpilot profile with low scores.

### Cited Findings
- **The $6M allegation.**
  - The claim was that a recent fomo iOS update caused about $6 million in losses. The cited example was 662 SOL (about $62K). The $6M estimate came with "no breakdown, victim list, or methodology" (via search summary). — [Crypto-Economy](https://crypto-economy.com/crypto-trading-app-fomo-denies-ios-hack-after-trader-claims-6m-loss/)
  - Co-founder Prashan Dharmasena "said the wallet in question has no transactions signed by FOMO's fee payer". ZachXBT tied the accusing account (Derivatives_Ape) to a ZKasino co-founder (via search summary). — [Protos](https://protos.com/crypto-trading-platform-fomo-denies-hack-of-its-ios-app/)
  - The transaction cited is real but "doesn't establish who caused it". The iOS app remained on the App Store (via search summary). — [edgeX news](https://pro.edgex.exchange/en-US/news/article/fomo-denies-ios-app-hack)
  - The exact date of the allegation was not directly verified. One search summary placed it in August 2026.
- **Phishing.** "A typosquatted site, fomoo.family, copies the legitimate fomo.family and its Connect wallet flow was designed to expose visitors to a crypto drainer" (via search summary). — [MalwareTips](https://malwaretips.com/blogs/fake-fomo-trading-platform-scam-how-fomoo-family-drains-connected-crypto-wallets/); [PCrisk](https://www.pcrisk.com/removal-guides/35598-fake-fomo-trading-platform-scam)
- **fomo's own phishing guidance.** fomo says it "will never ask you for your private keys". The web app has a "security-warning/ublock" route (warns about ad-blocker interference). — [fomo Security & Wallet Architecture](https://fomo.family/blog/guide/fomo-security-wallet-architecture); [fomo route manifest](https://fomo.family/assets/manifest-90574f0d.js)
- **Trustpilot.** A 2.0 TrustScore from 12 reviews, all 1-star, as of about October 2026. The profile is unclaimed and may be unrepresentative (via search summary). — [Trustpilot: fomo.family](https://ca.trustpilot.com/review/fomo.family)
- **App Store ratings.** 4.6/5 (441 ratings) in one snapshot; trackers show different counts (via search summary; conflicting). — [App Store listing](https://apps.apple.com/us/app/fomo-never-miss-out/id6741115427); [AppGoblin](https://appgoblin.info/apps/6741115427)
- **Small-trade fee criticism (fomo's own admission).** "On smaller trades, fomo's minimum makes Moby cheaper." — [fomo vs Moby](https://fomo.family/blog/compares/fomo-vs-moby)
- **Follower risk (fomo's own admission).** "Following popular traders does not guarantee future success... Users also risk copying trades too late or following traders who exit positions without notice." — [fomo vs pump.fun](https://fomo.family/blog/compares/fomo-vs-pump-fun)
- **Self-custody limits in the Terms.** "WE MAKE NO REPRESENTATIONS REGARDING THE SECURITY OR SAFETY OF YOUR DIGITAL ASSETS"; "Recovery may be limited or unavailable depending on the wallet configuration." — [fomo Terms §4](https://fomo.family/terms)
- **Cost display limits in the Terms.** fomo "may not display all such costs" (third-party fees). — [fomo Terms §7](https://fomo.family/terms)
- **Name collision.** Search results for "fomo reviews" are polluted by an unrelated Shopify social-proof app also called "Fomo" (fomo.com). — [Shopify App Store: Fomo reviews](https://apps.shopify.com/fomo/reviews?rating=3)

### Inferences
- **Practical risks** for a team: phishing and lookalike domains; account takeover through a compromised email or Apple ID (fomo's guide calls email "the gateway to your fomo wallet"); and the ToS-enforcement risk if automation is detected. Security posture appears standard for Privy-based apps (MFA, passkeys, FaceID).
- **Unresolved allegation.** The $6M claim should be treated as unresolved and weakly evidenced, not as a confirmed exploit.

### Gaps
- Primary articles (Protos, Crypto-Economy, MalwareTips) could not be fetched directly. Details rely on search summaries, and the exact date of the iOS-hack allegation is unconfirmed.
- No independent security audit, bug-bounty program, or SOC report for fomo was found.
- No Reddit threads specifically about fomo.family fees or withdrawals surfaced in searches.
