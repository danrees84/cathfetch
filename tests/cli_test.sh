#!/bin/sh
set -eu
app=./build/cathfetch
tmp_dir=$(mktemp -d /tmp/cathfetch-cli.XXXXXX)
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM
expect_failure() {
    if "$app" "$@" >"$tmp_dir/stdout" 2>"$tmp_dir/stderr"; then
        echo "Unexpected CLI success: $*" >&2
        exit 1
    fi
    test -s "$tmp_dir/stderr"
}
expect_failure --date=29022025
expect_failure --date=31132026
expect_failure --date=140026
expect_failure --date=abcdefgh
expect_failure --date=01010000
expect_failure --calendar=unknown
expect_failure --calendar=grc --calendar=wales
expect_failure --date=14102026 --date=15102026
expect_failure --cross --cross
expect_failure --unknown
expect_failure --date=24062038
"$app" --date=03042026 >"$tmp_dir/output"
rg -q 'Good Friday' "$tmp_dir/output"
if rg -q 'Fixed-date candidate|Ordinary Time' "$tmp_dir/output"; then exit 1; fi
"$app" --date=02032026 --calendar=wales --cross >"$tmp_dir/output"
rg -q 'St David' "$tmp_dir/output"
rg -q '███' "$tmp_dir/output"
"$app" --date=16082026 --calendar=england >"$tmp_dir/output"
rg -q 'Assumption' "$tmp_dir/output"
"$app" --date=07062026 --calendar=grc >"$tmp_dir/output"
rg -q '10th Sunday in Ordinary Time' "$tmp_dir/output"
"$app" --date=07062026 --calendar=wales >"$tmp_dir/output"
rg -q 'Body and Blood' "$tmp_dir/output"
"$app" --date=10082026 >"$tmp_dir/output"
# The resolved martyr's red ANSI code must reach the terminal.
python3 - "$tmp_dir/output" <<'PY'
import pathlib, sys
assert '\x1b[31m' in pathlib.Path(sys.argv[1]).read_text()
PY
"$app" >"$tmp_dir/output"
test -s "$tmp_dir/output"
if ./build/calendar_year invalid >"$tmp_dir/output" 2>"$tmp_dir/stderr"; then exit 1; fi
if ./build/calendar_year 2026 invalid >"$tmp_dir/output" 2>"$tmp_dir/stderr"; then exit 1; fi
echo 'CLI tests passed'
