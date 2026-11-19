#pragma once
// What one $1 trade really costs on Solana. Every number is configurable;
// defaults come from the research notes (see docs/STRATEGY.md).
#include "core/config.hpp"

namespace fomo {

struct FeeModel {
  double base_fee_lamports = 5000;          // per signature
  double priority_fee_lamports = 50000;     // compute-unit price x limit
  double tip_lamports = 0;                  // Jito / landing-service tip (0 in paper)
  double platform_fee_pct = 1.0;            // app/bot fee per side, percent
  double ata_rent_sol = 0.00203928;         // token account rent, refunded on close
  double extra_slippage_pct = 3.0;          // quote -> landed price drift (latency)

  static FeeModel from(const Config& c) {
    FeeModel f;
    f.base_fee_lamports = c.num("fee_base_lamports", f.base_fee_lamports);
    f.priority_fee_lamports = c.num("fee_priority_lamports", f.priority_fee_lamports);
    f.tip_lamports = c.num("fee_tip_lamports", f.tip_lamports);
    f.platform_fee_pct = c.num("fee_platform_pct", f.platform_fee_pct);
    f.ata_rent_sol = c.num("fee_ata_rent_sol", f.ata_rent_sol);
    f.extra_slippage_pct = c.num("fee_extra_slippage_pct", f.extra_slippage_pct);
    return f;
  }

  // Network cost of one transaction, in USD.
  double tx_cost_usd(double sol_usd) const {
    return (base_fee_lamports + priority_fee_lamports + tip_lamports) / 1e9 * sol_usd;
  }
  double platform_fee_usd(double notional_usd) const { return notional_usd * platform_fee_pct / 100.0; }
  double rent_usd(double sol_usd) const { return ata_rent_sol * sol_usd; }
};

}  // namespace fomo
