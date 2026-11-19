// FOMO Family engine.
//   fomo-engine --config config/fomo.conf --minutes 60 --data data
//   fomo-engine --roster        (print the 20 agents and exit)
#include <cstring>
#include <iostream>
#include <string>

#include "app/orchestrator.hpp"
#include "core/config.hpp"
#include "core/http.hpp"
#include "core/log.hpp"

int main(int argc, char** argv) {
  std::string config_path = "config/fomo.conf", data_dir = "data";
  double minutes = 60;
  bool roster_only = false;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    auto next = [&](const char* what) -> std::string {
      if (i + 1 >= argc) {
        std::cerr << "missing value for " << what << "\n";
        std::exit(2);
      }
      return argv[++i];
    };
    if (a == "--config") config_path = next("--config");
    else if (a == "--minutes") minutes = std::stod(next("--minutes"));
    else if (a == "--data") data_dir = next("--data");
    else if (a == "--roster") roster_only = true;
    else if (a == "--quiet") fomo::log::quiet() = true;
    else {
      std::cerr << "usage: fomo-engine [--config file] [--minutes N] [--data dir] [--roster] [--quiet]\n";
      return 2;
    }
  }
  try {
    fomo::Config cfg = fomo::Config::load_file(config_path);
    if (cfg.str("mode", "paper") == "live") {
      std::cerr << "Live mode is not available in this build. Paper trading only (see docs/ROADMAP.md).\n";
      return 3;
    }
    fomo::CurlHttp http;
    // Rate-limit buckets measured on 2026-11-19 (see research notes).
    http.add_bucket("api.dexscreener.com", "dexscreener", cfg.integer("gap_dexscreener_ms", 250));
    http.add_bucket("api.geckoterminal.com", "geckoterminal", cfg.integer("gap_geckoterminal_ms", 20000));
    http.add_bucket("api.rugcheck.xyz", "rugcheck", cfg.integer("gap_rugcheck_ms", 4500));
    http.add_bucket("lite-api.jup.ag/swap", "jup-quote", cfg.integer("gap_jup_quote_ms", 700));
    http.add_bucket("lite-api.jup.ag/tokens", "jup-meta", cfg.integer("gap_jup_meta_ms", 13000));
    http.add_bucket("lite-api.jup.ag/ultra", "jup-meta", cfg.integer("gap_jup_meta_ms", 13000));
    http.add_bucket("lite-api.jup.ag/price", "jup-meta", cfg.integer("gap_jup_meta_ms", 13000));
    http.add_bucket("api.mainnet-beta.solana.com", "solana-rpc", cfg.integer("gap_rpc_ms", 150));
    fomo::Orchestrator engine(cfg, http, data_dir);
    if (roster_only) {
      std::cout << "FOMO Family - 20 agents\n" << engine.roster();
      return 0;
    }
    engine.run(minutes);
  } catch (const std::exception& e) {
    std::cerr << "fatal: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
