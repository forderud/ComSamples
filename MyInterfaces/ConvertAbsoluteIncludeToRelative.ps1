# Get rid of absolute path in TLI include
$tlh_input = $args[0]
$tlh_output = $args[1]
$tli_file = $args[2]

$pattern = "#include `".+\\$tli_file`""
$replacement = "#include `"$tli_file`""

(Get-Content $tlh_input) -replace $pattern, $replacement | Out-File -encoding utf8 $tlh_output
