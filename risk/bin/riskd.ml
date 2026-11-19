(* riskd: reads one request per line on stdin, writes one reply per line. *)

let () =
  let limits = Risk.limits_of_args (List.tl (Array.to_list Sys.argv)) in
  try
    while true do
      let line = input_line stdin in
      print_string (Risk.reply limits line);
      print_char '\n';
      flush stdout
    done
  with End_of_file -> ()
