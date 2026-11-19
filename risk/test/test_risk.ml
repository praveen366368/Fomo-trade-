open Risk

let base = {
  id = "1"; size_usd = 1.0; open_positions = 0; deployed_usd = 0.0;
  day_pnl_usd = 0.0; trades_last_hour = 0; balance_usd = 100.0;
  score = 80.0; liquidity_usd = 20000.0; already_holding = false;
}

let failures = ref 0

let expect name got want =
  if got <> want then begin
    incr failures;
    Printf.printf "FAIL %s: got %S want %S\n" name got want
  end

let show = function Approve -> "approve" | Deny r -> "deny:" ^ r
let l = default_limits

let () =
  expect "happy path" (show (decide l base)) "approve";
  expect "size cap" (show (decide l { base with size_usd = 1.5 })) "deny:size_above_cap";
  expect "zero size" (show (decide l { base with size_usd = 0.0 })) "deny:size_not_positive";
  expect "holding" (show (decide l { base with already_holding = true })) "deny:already_holding_this_coin";
  expect "max open" (show (decide l { base with open_positions = 25 })) "deny:max_open_positions";
  expect "deployed" (show (decide l { base with deployed_usd = 24.5 })) "deny:max_deployed_capital";
  expect "daily loss" (show (decide l { base with day_pnl_usd = -15.0 })) "deny:daily_loss_limit";
  expect "trades/hour" (show (decide l { base with trades_last_hour = 30 })) "deny:max_trades_per_hour";
  expect "balance" (show (decide l { base with balance_usd = 2.5 })) "deny:low_balance";
  expect "score" (show (decide l { base with score = 59.9 })) "deny:score_below_min";
  expect "liquidity" (show (decide l { base with liquidity_usd = 4999.0 })) "deny:liquidity_below_min";
  (* protocol *)
  expect "ping" (reply l "PING") "PONG";
  expect "ok line"
    (reply l "CHECK id=9 size=1 open=2 deployed=2 day_pnl=-1 trades_hour=3 balance=90 score=75 liq=9000 holding=0")
    "OK id=9";
  expect "deny line"
    (reply l "CHECK id=10 size=1 open=2 deployed=2 day_pnl=-20 trades_hour=3 balance=90 score=75 liq=9000")
    "DENY id=10 reason=daily_loss_limit";
  expect "bad field" (reply l "CHECK id=11 size=abc") "ERR missing_or_bad_field";
  expect "unknown" (reply l "BUY everything") "ERR unknown_command";
  let custom = limits_of_args [ "--max-open"; "3"; "--daily-loss"; "5" ] in
  expect "args open" (show (decide custom { base with open_positions = 3 })) "deny:max_open_positions";
  expect "args loss" (show (decide custom { base with day_pnl_usd = -5.0 })) "deny:daily_loss_limit";
  if !failures > 0 then (Printf.printf "%d risk test(s) failed\n" !failures; exit 1)
  else print_endline "risk: all tests passed"
