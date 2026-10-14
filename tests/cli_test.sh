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
"$app" --date=24062038 >"$tmp_dir/output"
rg -q 'Body and Blood' "$tmp_dir/output"
"$app" --date=23062038 >"$tmp_dir/output"
rg -q 'Nativity of St John.*provisional transfer' "$tmp_dir/output"
"$app" --date=25062038 >"$tmp_dir/output"
rg -q 'Friday.*Ordinary Time' "$tmp_dir/output"
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
# Colours must stay attached to the correct line, including cross indentation.
python3 - <<'PYCOLOUR'
import subprocess

def output(date, *options):
    return subprocess.check_output(
        ['./build/cathfetch', '--date=' + date, *options], text=True).splitlines()

def dot(code):
    return '\x1b[' + code + 'm●\x1b[0m '

lawrence = output('10082026')
assert lawrence[1] == dot('32') + 'Ordinary Time'
assert lawrence[2].startswith(dot('31') + 'St Lawrence')
david = output('02032026', '--calendar=wales', '--cross')
assert any(line.endswith(dot('35') + 'Lent') for line in david)
assert any(dot('37') + 'St David' in line for line in david)
corpus = output('24062038')
assert corpus[1] == dot('32') + 'Ordinary Time'
assert corpus[2].startswith(dot('37') + 'The Most Holy Body and Blood')
weekday = output('25062038')
assert weekday[1] == dot('32') + 'Ordinary Time'
assert '●' not in weekday[2]
christmas = output('25122026')
assert christmas[1] == dot('37') + 'Christmas Time'
assert christmas[2].startswith(dot('37') + 'The Nativity')
good_friday = output('03042026')
assert good_friday[1] == 'Sacred Paschal Triduum'
assert good_friday[2] == dot('31') + 'Good Friday'
PYCOLOUR
"$app" >"$tmp_dir/output"
test -s "$tmp_dir/output"
if ./build/calendar_year invalid >"$tmp_dir/output" 2>"$tmp_dir/stderr"; then exit 1; fi
if ./build/calendar_year 2026 invalid >"$tmp_dir/output" 2>"$tmp_dir/stderr"; then exit 1; fi
echo 'CLI tests passed'
