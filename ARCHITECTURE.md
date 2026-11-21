# Architecture

## 1 The system

The `main` branch is almost empty. It holds one file, `README.md`, created by the first commit (20-11-2026).
There is no program, no database and no tests on `main`.

```
main branch:  README.md only
other branch: claude/tender-cerf-sdzlws  (the working code, not touched by these documents)
```

## 2 The parts

| Part | Where | Language | What it does |
|---|---|---|---|
| README | `README.md` | Markdown | Title only, plus the "At a glance" block added on 21-11-2026 |

## 3 The folder map (`main`)

```
README.md        title and document index
ARCHITECTURE.md  this page
CHANGELOG.md     history
CLAUDE.md        rules for AI coding tools
```

## 4 How one request travels

Not applicable on `main`: there is no code.

## 5 Sign-in and roles

Not applicable on `main`.

## 6 Background jobs

None on `main`.

## 7 Database

None on `main`.

## 8 Other branches

| Branch | What the file list shows |
|---|---|
| `claude/tender-cerf-sdzlws` | A project called "FOMO Family" in its own README: a meme-coin research set and a 20-agent trading bot. Folders seen in the file list: `engine/` (C++ sources and tests), `risk/` (OCaml), `config/`, `scripts/`, `docs/` (ARCHITECTURE, ROADMAP, STRATEGY, paper run results), `reports/` and `research_notes/`. Its README says it paper-trades on live market data. |

This was read from the file list and README of that branch only. It was not built, run or checked. Its own
`docs/ARCHITECTURE.md` is the place to learn more. Whether this branch should become `main` is a decision for the owner.
Those files were not copied here, to avoid a wrong picture of work that is not on `main`.

## 9 Known shape problems (compared with the Project Standard)

- The default branch has almost nothing; all real work sits on a side branch.
- No code, tests, `.gitignore` or migrations on `main`.
- The repository name ends with a dash (`Fomo-trade-`), which looks accidental.
- The other branch has a config file listing third-party wallet addresses (see CLAUDE.md, "Never").
