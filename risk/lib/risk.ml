(* FOMO Family risk governor.

   Every buy the engine wants to make is sent here first. This module is a
   pure function from (limits, request) to a decision, so it is easy to test
   and impossible to bypass by accident: the C++ engine fails closed (no
   trade) if this process is missing or answers anything but OK. *)

type limits = {
  max_size_usd : float;          (* biggest single buy, $1 by design *)
  max_open : int;                (* open positions at once *)
  max_deployed_usd : float;      (* total $ sitting in open positions *)
  max_daily_loss_usd : float;    (* stop buying after losing this much today *)
  max_trades_per_hour : int;
  min_balance_reserve_usd : float; (* keep this much SOL for fees/rent *)
  min_score : float;
  min_liquidity_usd : float;
}

let default_limits = {
  max_size_usd = 1.0;
  max_open = 25;
  max_deployed_usd = 25.0;
  max_daily_loss_usd = 15.0;
  max_trades_per_hour = 30;
  min_balance_reserve_usd = 2.0;
  min_score = 60.0;
  min_liquidity_usd = 5000.0;
}

type request = {
  id : string;
  size_usd : float;
  open_positions : int;
  deployed_usd : float;
  day_pnl_usd : float;
  trades_last_hour : int;
  balance_usd : float;
  score : float;
  liquidity_usd : float;
  already_holding : bool;
}

type decision = Approve | Deny of string

(* Order matters only for which reason is reported; any failing rule denies. *)
let decide (l : limits) (r : request) : decision =
  let checks = [
    (r.size_usd <= 0.0, "size_not_positive");
    (r.size_usd > l.max_size_usd +. 1e-9, "size_above_cap");
    (r.already_holding, "already_holding_this_coin");
    (r.open_positions >= l.max_open, "max_open_positions");
    (r.deployed_usd +. r.size_usd > l.max_deployed_usd +. 1e-9, "max_deployed_capital");
    (r.day_pnl_usd <= -. l.max_daily_loss_usd, "daily_loss_limit");
    (r.trades_last_hour >= l.max_trades_per_hour, "max_trades_per_hour");
    (r.balance_usd -. r.size_usd < l.min_balance_reserve_usd, "low_balance");
    (r.score < l.min_score, "score_below_min");
    (r.liquidity_usd < l.min_liquidity_usd, "liquidity_below_min");
  ] in
  match List.find_opt fst checks with
  | Some (_, reason) -> Deny reason
  | None -> Approve

(* ---- line protocol ------------------------------------------------------
   Request : CHECK id=7 size=1 open=3 deployed=3 day_pnl=-1.2 trades_hour=4
                   balance=96.5 score=71 liq=12000 holding=0
   Reply   : OK id=7            or   DENY id=7 reason=daily_loss_limit
   Also    : PING -> PONG ; anything malformed -> ERR <why>              *)

let parse_kv tokens =
  List.filter_map
    (fun t ->
      match String.index_opt t '=' with
      | None -> None
      | Some i ->
          Some (String.sub t 0 i, String.sub t (i + 1) (String.length t - i - 1)))
    tokens

let parse_request (line : string) : (request, string) result =
  let tokens = List.filter (fun s -> s <> "") (String.split_on_char ' ' (String.trim line)) in
  match tokens with
  | "CHECK" :: rest -> (
      let kv = parse_kv rest in
      let str k = List.assoc_opt k kv in
      let flt k = Option.bind (str k) float_of_string_opt in
      let int k = Option.bind (str k) int_of_string_opt in
      match
        ( str "id", flt "size", int "open", flt "deployed", flt "day_pnl",
          int "trades_hour", flt "balance", flt "score", flt "liq" )
      with
      | Some id, Some size_usd, Some open_positions, Some deployed_usd,
        Some day_pnl_usd, Some trades_last_hour, Some balance_usd, Some score,
        Some liquidity_usd ->
          Ok {
            id; size_usd; open_positions; deployed_usd; day_pnl_usd;
            trades_last_hour; balance_usd; score; liquidity_usd;
            already_holding = (str "holding" = Some "1");
          }
      | _ -> Error "missing_or_bad_field")
  | _ -> Error "unknown_command"

let reply (l : limits) (line : string) : string =
  match String.trim line with
  | "PING" -> "PONG"
  | _ -> (
      match parse_request line with
      | Error e -> "ERR " ^ e
      | Ok r -> (
          match decide l r with
          | Approve -> "OK id=" ^ r.id
          | Deny why -> "DENY id=" ^ r.id ^ " reason=" ^ why))

(* Command-line flags override the defaults: --max-open 25 --daily-loss 15 ... *)
let limits_of_args (args : string list) : limits =
  let rec go l = function
    | "--max-size" :: v :: tl -> go { l with max_size_usd = float_of_string v } tl
    | "--max-open" :: v :: tl -> go { l with max_open = int_of_string v } tl
    | "--max-deployed" :: v :: tl -> go { l with max_deployed_usd = float_of_string v } tl
    | "--daily-loss" :: v :: tl -> go { l with max_daily_loss_usd = float_of_string v } tl
    | "--max-trades-hour" :: v :: tl -> go { l with max_trades_per_hour = int_of_string v } tl
    | "--reserve" :: v :: tl -> go { l with min_balance_reserve_usd = float_of_string v } tl
    | "--min-score" :: v :: tl -> go { l with min_score = float_of_string v } tl
    | "--min-liq" :: v :: tl -> go { l with min_liquidity_usd = float_of_string v } tl
    | _ :: tl -> go l tl
    | [] -> l
  in
  go default_limits args
