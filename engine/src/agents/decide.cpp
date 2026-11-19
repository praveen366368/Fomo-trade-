// Agents 15-17: NarrativeAI, Strategist, RiskGovernor.
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include <cstring>
#include <sstream>

#include "agents/agent.hpp"
#include "core/log.hpp"
#include "core/util.hpp"

namespace fomo {

using nlohmann::json;

// ------------------------------------------------------------ 15 NarrativeAI
// Works with Jev (TypeSafe AI) or any OpenAI-compatible chat endpoint.
// Turned on only when both ai_url and the FOMO_AI_KEY environment variable
// are set. The key never lives in a file in this repo.
NarrativeAI::NarrativeAI(Http& http, const Config& c)
    : Agent(15, "NarrativeAI", "AI read of the coin's story and hype (optional)"),
      http_(http),
      url_(c.str("ai_url")),
      model_(c.str("ai_model", "jev-latest")),
      key_(c.str("ai_key")) {
  enabled_ = !url_.empty() && !key_.empty();
}

Verdict NarrativeAI::judge(const Candidate& c) {
  if (!enabled_ || !c.snap) return pass(0, "AI off (no key)");
  const auto& s = *c.snap;
  std::ostringstream facts;
  facts << "Solana meme coin $" << s.symbol << " (" << s.name << "). Market cap $" << s.mcap
        << ", liquidity $" << s.liquidity_usd << ", 5m change " << s.chg_m5 << "%, 1h change " << s.chg_h1
        << "%, 5m buys/sells " << s.buys_m5 << "/" << s.sells_m5 << ", socials: "
        << (s.has_twitter ? "X " : "") << (s.has_telegram ? "Telegram " : "") << (s.has_website ? "website" : "")
        << ". Found via: ";
  for (const auto& src : c.sources) facts << src << " ";
  json body = {
      {"model", model_},
      {"temperature", 0},
      {"messages",
       {{{"role", "system"},
         {"content",
          "You score meme coins for a tiny $1 momentum trade. Judge only narrative strength, "
          "originality and scam smell from the facts given. Reply with JSON only: "
          "{\"score\": <0-100>, \"reason\": \"<10 words>\"}."}},
        {{"role", "user"}, {"content", facts.str()}}}}};
  auto r = http_.post_json(url_, body.dump(), {"Authorization: Bearer " + key_});
  if (!r.ok()) return pass(0, "AI unavailable (neutral)");
  json j = json::parse(r.body, nullptr, false);
  std::string content;
  try {
    content = j["choices"][0]["message"]["content"].get<std::string>();
  } catch (...) {
    return pass(0, "AI reply unreadable (neutral)");
  }
  const auto b = content.find('{');
  const auto e = content.rfind('}');
  if (b == std::string::npos || e == std::string::npos) return pass(0, "AI reply unreadable (neutral)");
  json v = json::parse(content.substr(b, e - b + 1), nullptr, false);
  if (!v.is_object() || !v["score"].is_number()) return pass(0, "AI reply unreadable (neutral)");
  const double score = v["score"].get<double>();
  const std::string why = v.value("reason", "");
  if (score < 30) return fail("AI: " + why, false, -10);
  return pass((score - 50.0) / 5.0, "AI " + std::to_string(static_cast<int>(score)) + ": " + why);
}

// ------------------------------------------------------------- 16 Strategist
Strategist::Strategist(const Config& c)
    : Agent(16, "Strategist", "adds every verdict into one score and decides"),
      entry_score_(c.num("entry_score", 65)),
      smart_bonus_(c.num("smart_money_bonus", 15)),
      boost_bonus_(c.num("boost_bonus", 3)) {}

bool Strategist::decide(Candidate& c) {
  double score = 50;
  for (const auto& v : c.verdicts) {
    if (!v.pass) {
      c.score = 0;
      fail("blocked by " + v.agent, false);
      return false;
    }
    score += v.score;
  }
  for (const auto& s : c.sources)
    if (s.rfind("smart:", 0) == 0) {
      score += smart_bonus_;
      break;
    }
  if (c.boost > 0) score += boost_bonus_;
  c.score = std::max(0.0, std::min(100.0, score));
  if (c.score < entry_score_) {
    fail("score below entry", false);
    return false;
  }
  pass(c.score, "buy");
  return true;
}

// ----------------------------------------------------------- 17 RiskGovernor
RiskGovernor::RiskGovernor(const Config& c)
    : Agent(17, "RiskGovernor", "OCaml limits engine; approves or blocks every buy") {
  path_ = c.str("risk_daemon", "risk/_build/default/bin/riskd.exe");
  std::ostringstream a;
  a << "--max-size " << c.num("stake_usd", 1.0) << " --max-open " << c.integer("risk_max_open", 25)
    << " --max-deployed " << c.num("risk_max_deployed_usd", 25) << " --daily-loss "
    << c.num("risk_daily_loss_usd", 15) << " --max-trades-hour " << c.integer("risk_max_trades_hour", 30)
    << " --reserve " << c.num("risk_reserve_usd", 2) << " --min-score " << c.num("entry_score", 65)
    << " --min-liq " << c.num("liq_min_usd", 8000);
  args_ = a.str();
  signal(SIGPIPE, SIG_IGN);
  start();
}

RiskGovernor::~RiskGovernor() { stop(); }

bool RiskGovernor::start() {
  int in_pipe[2], out_pipe[2];
  if (pipe(in_pipe) != 0) return false;
  if (pipe(out_pipe) != 0) {
    close(in_pipe[0]);
    close(in_pipe[1]);
    return false;
  }
  std::vector<std::string> parts = {path_};
  for (const auto& p : split(args_, ' '))
    if (!p.empty()) parts.push_back(p);
  pid_t pid = fork();
  if (pid < 0) return false;
  if (pid == 0) {
    dup2(in_pipe[0], STDIN_FILENO);
    dup2(out_pipe[1], STDOUT_FILENO);
    close(in_pipe[1]);
    close(out_pipe[0]);
    std::vector<char*> argv;
    argv.reserve(parts.size() + 1);
    for (auto& p : parts) argv.push_back(p.data());
    argv.push_back(nullptr);
    execv(path_.c_str(), argv.data());
    _exit(127);
  }
  close(in_pipe[0]);
  close(out_pipe[1]);
  pid_ = pid;
  to_child_ = in_pipe[1];
  from_child_ = out_pipe[0];
  // Health check: must answer PING with PONG.
  const std::string ping = "PING\n";
  if (write(to_child_, ping.data(), ping.size()) != static_cast<ssize_t>(ping.size())) {
    stop();
    return false;
  }
  pollfd pfd{from_child_, POLLIN, 0};
  char buf[16] = {0};
  if (poll(&pfd, 1, 3000) <= 0 || read(from_child_, buf, sizeof buf - 1) <= 0 || std::strncmp(buf, "PONG", 4) != 0) {
    log::error(name(), "risk daemon not answering at " + path_ + " - ALL BUYS BLOCKED (fail closed)");
    stop();
    return false;
  }
  log::info(name(), "OCaml risk daemon up (" + args_ + ")");
  return true;
}

void RiskGovernor::stop() {
  if (to_child_ >= 0) close(to_child_);
  if (from_child_ >= 0) close(from_child_);
  if (pid_ > 0) waitpid(pid_, nullptr, 0);
  to_child_ = from_child_ = pid_ = -1;
}

std::string RiskGovernor::approve(const Request& r) {
  if (pid_ <= 0) {
    fail("risk daemon down", false);
    return "risk_daemon_down";
  }
  const long id = next_id_++;
  char line[512];
  std::snprintf(line, sizeof line,
                "CHECK id=%ld size=%.6f open=%d deployed=%.6f day_pnl=%.6f trades_hour=%d balance=%.6f "
                "score=%.3f liq=%.2f holding=%d\n",
                id, r.size, r.open, r.deployed, r.day_pnl, r.trades_hour, r.balance, r.score, r.liq,
                r.holding ? 1 : 0);
  const size_t len = std::strlen(line);
  if (write(to_child_, line, len) != static_cast<ssize_t>(len)) {
    stop();
    fail("risk daemon write failed", false);
    return "risk_daemon_down";
  }
  std::string reply;
  char ch;
  pollfd pfd{from_child_, POLLIN, 0};
  while (reply.size() < 256) {
    if (poll(&pfd, 1, 2000) <= 0 || read(from_child_, &ch, 1) != 1) {
      stop();
      fail("risk daemon timeout", false);
      return "risk_daemon_timeout";
    }
    if (ch == '\n') break;
    reply.push_back(ch);
  }
  const std::string ok = "OK id=" + std::to_string(id);
  if (reply == ok) {
    pass(0, "approved");
    return "";
  }
  const auto p = reply.find("reason=");
  std::string why = p == std::string::npos ? "bad_reply:" + reply : reply.substr(p + 7);
  fail(why, false);
  return why;
}

}  // namespace fomo
