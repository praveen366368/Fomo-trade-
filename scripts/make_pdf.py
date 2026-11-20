#!/usr/bin/env python3
"""Build reports/FOMO-Family-Complete-Work.pdf from everything in the repo.

    python3 scripts/make_pdf.py            (needs: pip install markdown; node + playwright)

Parts: work summary, full research report, 20-agent design, strategy +
settings, paper runs (from docs/paper-runs/*/summary.json), roadmap,
appendices (how to run, files, tests, research notes).
"""
import html
import json
import re
import subprocess
import sys
from pathlib import Path

import markdown

ROOT = Path(__file__).resolve().parent.parent
OUT_HTML = ROOT / "reports" / "FOMO-Family-Complete-Work.html"
OUT_PDF = ROOT / "reports" / "FOMO-Family-Complete-Work.pdf"
GH = "https://github.com/praveen366368/Fomo-trade-/blob/claude/tender-cerf-sdzlws/"
DATE = "20 Nov 2026"


def md(text: str, base: str = "") -> str:
    """Markdown -> HTML, headings shifted down one level, repo links -> GitHub."""
    out = markdown.markdown(text, extensions=["tables", "fenced_code", "sane_lists"])
    for n in range(5, 0, -1):
        out = re.sub(rf"<(/?)h{n}>", rf"<\1h{n + 1}>", out)

    def fix(m):
        href = m.group(1)
        if href.startswith(("http://", "https://", "#", "mailto:")):
            return m.group(0)
        p = (Path(base) / href).as_posix() if base else href
        parts = []
        for seg in p.split("/"):
            if seg == "..":
                if parts:
                    parts.pop()
            elif seg not in ("", "."):
                parts.append(seg)
        return f'href="{GH}{"/".join(parts)}"'

    return re.sub(r'href="([^"]+)"', fix, out)


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def esc(s) -> str:
    return html.escape(str(s))


def usd(v: float) -> str:
    return ("−" if v < 0 else "") + f"${abs(v):.2f}"


# ---------------------------------------------------------------- paper runs
def run_section(folder: str, title: str, note: str, best_label: str) -> str:
    s = json.loads(read(f"docs/paper-runs/{folder}/summary.json"))
    audit = json.loads(read(f"docs/paper-runs/{folder}/filter_audit.json"))
    stake = s["stake_usd"]
    rows = []
    for p in s["positions"]:
        got = stake + p["pnl"]
        cls = "neg" if p["pnl"] < 0 else "pos"
        rows.append(
            f"<tr><td>{esc(p['symbol'])}</td><td class='n'>{p['held_min']:.1f} min</td>"
            f"<td class='n'>{(p['peak_multiple'] - 1) * 100:+.1f}%</td>"
            f"<td class='n'><span class='{cls}'>{usd(p['pnl'])}</span></td><td>{esc(p['why'])}</td></tr>"
        )
    agents = "".join(
        f"<tr><td class='n'>{a['id']}</td><td>{esc(a['name'])}</td><td class='n'>{a['runs']:,}</td>"
        f"<td class='n'>{a['passes']:,}</td><td class='n'>{a['fails']:,}</td></tr>"
        for a in s["agents"]
    )
    apis = "".join(
        f"<tr><td>{esc(h)}</td><td class='n'>{v['calls']:,}</td><td class='n'>{v['errors']}</td>"
        f"<td class='n'>{v['rate_limited']}</td><td class='n'>{v['avg_ms']:.0f} ms</td></tr>"
        for h, v in s["apis"].items()
    )
    aud = "".join(
        f"<tr><td>{esc(k)}</td><td class='n'>{v['rejected']}</td><td class='n'>{v['later_2x']}</td>"
        f"<td class='n'>{v['later_dead']}</td></tr>"
        for k, v in audit["by_agent"].items()
    ) or "<tr><td colspan='4'>No rejected coins were re-checked in this run.</td></tr>"
    n = max(1, s["closed"])
    return f"""
<h3>{esc(title)}</h3>
<p>{note}</p>
<div class="stats">
  <div><b>{s['minutes']:.0f} min</b><small>run length</small></div>
  <div><b>{s['leads']:,}</b><small>leads found</small></div>
  <div><b>{s['deep_checks']}</b><small>deep safety checks</small></div>
  <div><b>{s['buys']}</b><small>paper buys at {usd(stake)}</small></div>
  <div><b>{s['wins']}</b><small>winners</small></div>
  <div><b class="{'bad' if s['pnl_closed_usd'] < 0 else ''}">{usd(s['pnl_closed_usd'])}</b><small>total, {s['pnl_closed_usd'] / n / stake * 100:+.0f}% per trade</small></div>
</div>
<h4>Trades</h4>
<table><thead><tr><th>Coin</th><th class='n'>Held</th><th class='n'>{best_label}</th><th class='n'>Profit / loss</th><th>Why sold</th></tr></thead>
<tbody>{''.join(rows)}</tbody></table>
<h4>Filter audit (coins we rejected, followed afterwards)</h4>
<table><thead><tr><th>Rejected by</th><th class='n'>Coins</th><th class='n'>Later 2x+</th><th class='n'>Later dead</th></tr></thead><tbody>{aud}</tbody></table>
<h4>API health</h4>
<table><thead><tr><th>API</th><th class='n'>Calls</th><th class='n'>Errors</th><th class='n'>Rate limited</th><th class='n'>Average</th></tr></thead><tbody>{apis}</tbody></table>
<h4>Agent activity</h4>
<table class="small"><thead><tr><th class='n'>#</th><th>Agent</th><th class='n'>Runs</th><th class='n'>Pass</th><th class='n'>Fail</th></tr></thead><tbody>{agents}</tbody></table>
"""


# ---------------------------------------------------------------- appendices
def file_table() -> str:
    files = subprocess.run(["git", "ls-files"], cwd=ROOT, capture_output=True, text=True, check=True).stdout.split("\n")
    groups = {}
    for f in files:
        if not f or f.endswith((".pdf", ".html")) and f.startswith("reports/FOMO-Family-Complete-Work"):
            continue
        top = f.split("/")[0] if "/" in f else "(root)"
        if f.startswith("engine/tests/fixtures/"):
            top = "engine/tests/fixtures"
        elif f.startswith("engine/"):
            top = "/".join(f.split("/")[:3]) if f.count("/") >= 2 else "engine"
        try:
            lines = sum(1 for _ in (ROOT / f).open(encoding="utf-8", errors="ignore"))
        except (IsADirectoryError, FileNotFoundError):
            continue
        groups.setdefault(top, []).append((f, lines))
    out = []
    for g in sorted(groups):
        items = groups[g]
        names = ", ".join(esc(Path(f).name) for f, _ in items)
        out.append(f"<tr><td><code>{esc(g)}</code></td><td class='n'>{len(items)}</td>"
                   f"<td class='n'>{sum(l for _, l in items):,}</td><td class='files'>{names}</td></tr>")
    return ("<table class='small'><thead><tr><th>Folder</th><th class='n'>Files</th><th class='n'>Lines</th>"
            "<th>Files</th></tr></thead><tbody>" + "".join(out) + "</tbody></table>")


def notes_table() -> str:
    desc = {
        "fomo_platform.md": "FOMO company, funding, chains, fees, wallets, Terms, API, social features, incidents",
        "top_traders.md": "How to find FOMO traders, documented examples (verified/unverified), strategies, copy-trading evidence",
        "market_stats.md": "Launch volume, graduation and death rates, returns, who wins, $1 costs, live 255-coin sample",
        "data_apis.md": "Market data, launch feeds, safety APIs, execution, social/news, live endpoint tests, prices",
        "strategy_filters.md": "Rug filters, entry signals, volatility, exit-rule simulation, sizing, failure modes",
        "stack_agents_security.md": "Latency, Solana libraries, Jane Street/OCaml, agent frameworks, Jev, security, India tax",
    }
    folder = ROOT / "research_notes" / "FOMO Family meme coin research"
    rows = "".join(
        f"<tr><td><code>{esc(p.name)}</code></td><td class='n'>{len(p.read_text(encoding='utf-8').split()):,}</td>"
        f"<td>{esc(desc.get(p.name, ''))}</td></tr>"
        for p in sorted(folder.glob("*.md"))
    )
    return ("<table><thead><tr><th>Note</th><th class='n'>Words</th><th>Covers</th></tr></thead><tbody>"
            + rows + "</tbody></table>")


def test_results() -> str:
    def run(cmd, cwd):
        r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
        return (r.stdout + r.stderr).strip().splitlines()[-1] if (r.stdout + r.stderr).strip() else f"exit {r.returncode}"
    engine = run(["engine/build/fomo-tests"], ROOT) if (ROOT / "engine/build/fomo-tests").exists() else "not built"
    risk = run(["dune", "test", "--force"], ROOT / "risk")
    return f"<ul><li>C++ engine: <code>{esc(engine)}</code></li><li>OCaml risk daemon: <code>{esc(risk)}</code></li>" \
           "<li>Build uses <code>-Wall -Wextra -Wpedantic -Wconversion -Werror</code>; clang-tidy (bugprone, performance, analyzer) is clean.</li></ul>"


# ---------------------------------------------------------------- page parts
LANES = """
<div class="lanes">
  <div><h5>Discover</h5><ol><li><i>1</i>LaunchScout</li><li><i>2</i>TrendScout</li><li><i>3</i>SmartMoney</li><li><i>4</i>TraderScout</li></ol></div>
  <div><h5>Observe</h5><ol><li><i>5</i>MarketData</li><li><i>6</i>ApiHealth</li><li><i>7</i>Volatility</li></ol></div>
  <div><h5>Filter</h5><ol><li><i>8</i>LiquidityGuard</li><li><i>9</i>SocialCheck</li><li><i>10</i>Momentum</li><li><i>11</i>RugGuard</li><li><i>12</i>HolderAnalyst</li><li><i>13</i>HoneypotTester</li><li><i>14</i>OrganicFlow</li></ol></div>
  <div><h5>Decide</h5><ol><li><i>15</i>NarrativeAI (Jev)</li><li><i>16</i>Strategist</li><li class="ocaml"><i>17</i>RiskGovernor (OCaml)</li></ol></div>
  <div><h5>Act</h5><ol><li><i>18</i>Executor</li><li><i>19</i>ExitManager</li></ol></div>
  <div><h5>Learn</h5><ol><li><i>20</i>Auditor</li></ol></div>
</div>"""

SUMMARY = f"""
<section class="part" id="summary">
<div class="partno">Part 1</div>
<h1>Work summary</h1>
<div class="saar"><b>Saar (short version):</b> Humne 6 tarah ki research ki, 20 agents wala trading bot banaya
(C++ engine + OCaml risk guard), aur use asli market data pe paper mode mein chalaya (asli paisa nahi).
Nateeja: $1 har coin wala plan fees ke baad ghate mein jaata hai. FOMO app bots ko ban karta hai, isliye
bot alag wallet se on-chain chalega. Bot ko abhi paper mode mein rakhein, pehle recorded data pe edge
saabit karein, phir $2&ndash;$5 stake aur alag burner wallet ke saath hi live jaayein.</div>

<h2>What was asked</h2>
<ul>
<li>Research FOMO (fomo.family) and meme-coin trading: $1 into each coin, take profit at a target %.</li>
<li>Design about 20 agents with separate jobs (market, news, data, API, volatility, testing/filtering, execution) to auto-trade coins that can return thousands of percent in minutes.</li>
<li>Find FOMO traders who turned $1&ndash;$100 into thousands or lakhs in 3&ndash;4 months, and learn their strategies.</li>
<li>Use a fast stack: C++, OCaml, and the "JEV" AI.</li>
</ul>

<h2>What was done</h2>
<table>
<thead><tr><th>Step</th><th>Result</th></tr></thead>
<tbody>
<tr><td>Research, 6 tracks in parallel</td><td>FOMO platform, top traders, market statistics (incl. a live 255-coin sample), data and execution APIs (tested live), filters and exit rules (with simulation), stack / agents / security / India tax. About 50,000 words of sourced notes.</td></tr>
<tr><td>Bot built</td><td>20 agents. C++20 engine (data, filters, paper execution, exits, books) and an OCaml risk daemon that approves or blocks every buy and fails closed.</td></tr>
<tr><td>Tested</td><td>126 C++ tests on recorded real API responses + 18 OCaml tests, all passing. Strict compiler warnings and clang-tidy clean.</td></tr>
<tr><td>Paper run 1 (45 min, live data)</td><td>4,860 leads, 145 deep checks, 5 buys, 0 wins, &minus;$0.93. Exposed two engine bugs.</td></tr>
<tr><td>Bugs fixed</td><td>Exits now act on a real sell quote (not a delayed screen price). Hard-rejected coins are banned permanently.</td></tr>
<tr><td>Paper run 2 (15 min, with fixes)</td><td>1,872 leads, 18 deep checks, 4 buys, 1 win, &minus;$0.22. Sellable marks now match sale proceeds.</td></tr>
<tr><td>Published</td><td>Full research report (Part 2), web page, and all code on GitHub: <code>praveen366368/Fomo-trade-</code>, branch <code>claude/tender-cerf-sdzlws</code>.</td></tr>
</tbody></table>

<h2>Key numbers</h2>
<div class="stats">
  <div><b>1 in 250</b><small>new coins touched 10x within an hour (live sample, 255 coins)</small></div>
  <div><b class="bad">90%</b><small>of 829,539 FOMO app wallets lost money after fees, Jan&ndash;Oct 2026</small></div>
  <div><b class="bad">&minus;19%</b><small>average per $1 trade, paper run 1 (5 buys, 5 losses)</small></div>
  <div><b>$579M</b><small>made by coin creators and bundled wallets, vs $31.5M for sniper bots</small></div>
  <div><b>3.3% &ndash; 34%</b><small>cost of one $1 round trip: lean bot vs trading-app defaults</small></div>
  <div><b>30%</b><small>India tax on gains, no loss offset</small></div>
</div>

<div class="fig">
  <h4>What happened to 255 new coins in their first hour</h4>
  <div class="bars">
    <div class="bar"><span>No trades in the last hour</span><span class="track"><span class="fill loss" style="width:66%"></span></span><span class="v">66%</span></div>
    <div class="bar"><span>Fell more than 90%</span><span class="track"><span class="fill loss" style="width:10%"></span></span><span class="v">~10%</span></div>
    <div class="bar"><span>Touched 2x</span><span class="track"><span class="fill gain" style="width:6.3%"></span></span><span class="v">6.3%</span></div>
    <div class="bar"><span>Touched 5x</span><span class="track"><span class="fill gain" style="width:2.4%"></span></span><span class="v">2.4%</span></div>
    <div class="bar"><span>Touched 10x</span><span class="track"><span class="fill gain" style="width:0.4%"></span></span><span class="v">0.4%</span></div>
  </div>
  <p class="src">Coins created 19:12&ndash;19:21 UTC, 19 Nov 2026, "bought" about 6 minutes after launch, tracked 60 minutes. Scale 0&ndash;100%.</p>
</div>

<h2>Status</h2>
<table>
<thead><tr><th>Item</th><th>Status</th></tr></thead>
<tbody>
<tr><td>Research report and notes</td><td><span class="ok">Done</span></td></tr>
<tr><td>20-agent bot, paper mode on live data</td><td><span class="ok">Done</span></td></tr>
<tr><td>Two engine bugs from run 1</td><td><span class="ok">Fixed and tested</span></td></tr>
<tr><td>Live trading with real money</td><td><span class="off">Off on purpose</span>: needs a separate burner wallet and signer set up by the owner</td></tr>
<tr><td>Copy-trade wallet list</td><td><span class="off">Empty</span>: pick traders by hand from FOMO's leaderboard</td></tr>
<tr><td>Jev AI agent</td><td><span class="off">Off</span> until a key is set on the server</td></tr>
<tr><td>Server deploy</td><td><span class="off">None yet</span>: no server or deploy script exists for this project</td></tr>
<tr><td>Owner decision</td><td>Stake stays $1 (owner's rule); research says $2&ndash;$5 beats fees</td></tr>
</tbody></table>
</section>
"""


def build_html() -> str:
    report = md(read("reports/FOMO Family meme coin research.md"), "reports")
    arch_md = read("docs/ARCHITECTURE.md")
    arch_md = re.sub(r"```\n DISCOVER.*?```", "@@LANES@@", arch_md, flags=re.S)
    arch = md(arch_md, "docs").replace("<p>@@LANES@@</p>", LANES).replace("@@LANES@@", LANES)
    strategy = md(read("docs/STRATEGY.md"), "docs")
    roadmap = md(read("docs/ROADMAP.md"), "docs")
    readme = md(read("README.md"), "")
    conf = esc(read("config/fomo.conf"))
    runs = run_section(
        "2026-11-19-run1", "Run 1: 19 Nov 2026, 19:37–20:22 UTC (45 min)",
        "Before the fixes. Exits were decided on Dexscreener's screen price; sales used Jupiter's real quote.",
        "Best (screen price)",
    ) + run_section(
        "2026-11-19-run2", "Run 2: 19 Nov 2026, 20:55–21:10 UTC (15 min)",
        "After the fixes. Positions marked at real sell quotes; hard rejects banned permanently.",
        "Best (sellable)",
    )
    css = (ROOT / "scripts" / "pdf.css").read_text(encoding="utf-8")
    return f"""<!doctype html><html lang="en"><head><meta charset="utf-8">
<title>FOMO Family - Complete Work Report</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Bricolage+Grotesque:opsz,wght@12..96,600;12..96,700&family=IBM+Plex+Sans:ital,wght@0,400;0,600;1,400&family=IBM+Plex+Mono:wght@400;500&display=swap">
<style>{css}</style></head><body>
<section class="cover">
  <div class="eyebrow">FOMO Family &middot; Complete work report</div>
  <h1>Meme-coin research and the 20-agent trading bot</h1>
  <p class="lede">Everything done so far: research on FOMO (fomo.family) and Solana meme coins, the C++ / OCaml bot with 20 agents, two live paper-trading runs, the fixes they led to, and what comes next.</p>
  <table class="kv">
    <tr><td>Date</td><td>{DATE}</td></tr>
    <tr><td>Mode</td><td>Paper trading on live market data (no real money)</td></tr>
    <tr><td>Code</td><td>github.com/praveen366368/Fomo-trade-, branch claude/tender-cerf-sdzlws</td></tr>
    <tr><td>Verdict</td><td>Do not fund $1-per-coin auto-trading. Keep the bot as a lab and safety screener; prove an edge on recorded data first.</td></tr>
  </table>
  <h3>Contents</h3>
  <ol class="toc">
    <li>Work summary</li><li>Full research report</li><li>The 20-agent design</li>
    <li>Strategy and settings</li><li>Paper-trading runs</li><li>Roadmap</li>
    <li>Appendices: how to run, files, tests, research notes</li>
  </ol>
  <p class="fine">Not financial advice. Many third-party figures came from search summaries rather than full documents; the research notes mark each one.</p>
</section>
{SUMMARY}
<section class="part"><div class="partno">Part 2</div><h1>Full research report</h1>{report}</section>
<section class="part"><div class="partno">Part 3</div><h1>The 20-agent design</h1>{arch}</section>
<section class="part"><div class="partno">Part 4</div><h1>Strategy and settings</h1>{strategy}
<h2>All settings (config/fomo.conf)</h2><pre class="conf">{conf}</pre></section>
<section class="part"><div class="partno">Part 5</div><h1>Paper-trading runs</h1>
<p>Both runs used live public data (Dexscreener, GeckoTerminal, RugCheck, Jupiter, Solana RPC), $1 stakes and a $250 paper balance. Five or four trades prove nothing about an edge; the runs test the machinery and show how costs behave.</p>
{runs}</section>
<section class="part"><div class="partno">Part 6</div><h1>Roadmap</h1>{roadmap}</section>
<section class="part"><div class="partno">Appendix</div><h1>How to run, files, tests, notes</h1>
<h2>A. How to run (README)</h2>{readme}
<h2>B. Files in the repository</h2>{file_table()}
<h2>C. Test results</h2>{test_results()}
<h2>D. Research notes</h2>{notes_table()}
</section>
</body></html>"""


PRINT_JS = r"""
const { chromium } = require('playwright');
(async () => {
  const [src, out] = process.argv.slice(2);
  const b = await chromium.launch();
  const p = await b.newPage();
  await p.goto('file://' + src, { waitUntil: 'networkidle' });
  await p.evaluate(() => document.fonts.ready);
  await p.pdf({
    path: out, format: 'A4', printBackground: true,
    margin: { top: '16mm', bottom: '18mm', left: '16mm', right: '16mm' },
    displayHeaderFooter: true,
    headerTemplate: '<div></div>',
    footerTemplate: '<div style="width:100%;font-size:8px;color:#6b7280;padding:0 16mm;display:flex;justify-content:space-between;font-family:Helvetica,Arial,sans-serif"><span>FOMO Family &middot; Complete work report &middot; 20 Nov 2026</span><span>Page <span class="pageNumber"></span> of <span class="totalPages"></span></span></div>',
  });
  await b.close();
})();
"""


def main() -> int:
    OUT_HTML.write_text(build_html(), encoding="utf-8")
    js = ROOT / "scripts" / ".print_pdf.js"
    js.write_text(PRINT_JS, encoding="utf-8")
    try:
        subprocess.run(["node", str(js), str(OUT_HTML), str(OUT_PDF)], check=True)
    finally:
        js.unlink(missing_ok=True)
    print(f"wrote {OUT_PDF.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
