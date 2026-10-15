#!/bin/sh
set -eu
app=./build/cathfetch
COLUMNS=120
export COLUMNS
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
# Help is a standalone information request: it must work outside the project
# directory, without calendar/Bible data, and write only to stdout.
python3 - <<'PYOPTIONS'
from pathlib import Path
import subprocess
import tempfile

app = str(Path('./build/cathfetch').resolve())
def run(*arguments, **kwargs):
    return subprocess.run([app, *arguments], text=True, capture_output=True, **kwargs)

with tempfile.TemporaryDirectory(prefix='cathfetch-help-') as empty_directory:
    help_output = run('--help', cwd=empty_directory)
assert help_output.returncode == 0 and not help_output.stderr
assert help_output.stdout.startswith('Usage: cathfetch [OPTIONS]\n')
assert '\x1b' not in help_output.stdout
assert all(len(line) <= 80 for line in help_output.stdout.splitlines())
assert run('-h').stdout == help_output.stdout
assert run('--help', '--date=invalid').returncode == 0
for option in ['--help', '--date', '--calendar', '--cross', '--readings', '--full-verses', '--mass']:
    assert option in help_output.stdout, option
for form in ['day', 'vigil', 'vigil-extended', 'night', 'dawn', 'evening', 'passion']:
    assert form in help_output.stdout, form

equals = run('--date=25122026', '--calendar=wales', '--mass=night', '--full-verses')
spaces = run('--date', '25122026', '--calendar', 'wales', '--mass', 'night', '--full-verses')
assert equals.returncode == spaces.returncode == 0
assert equals.stdout == spaces.stdout and not spaces.stderr
ordinary = run('--date=08122026')
assert run('--date=08122026', '--').stdout == ordinary.stdout

for arguments in [('--date',), ('--calendar',), ('--mass',), ('--date=',),
                  ('--calendar=',), ('--mass=',), ('--date', '--cross'),
                  ('--mass', '--readings')]:
    result = run(*arguments)
    assert result.returncode == 1 and not result.stdout
    assert result.stderr.startswith('cathfetch: Missing value for ')
    assert "Try 'cathfetch --help'" in result.stderr
for arguments in [('--', '--help'), ('unexpected',), ('--cross=yes',),
                  ('--help=yes',), ('--unknown',)]:
    result = run(*arguments)
    assert result.returncode == 1 and not result.stdout
    assert result.stderr.startswith('cathfetch: ')
    assert "Try 'cathfetch --help'" in result.stderr
print('Help, option syntax and diagnostic tests passed')
PYOPTIONS
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
import os
import pty
import subprocess

def output(date, *options):
    master, slave = pty.openpty()
    try:
        subprocess.run(['./build/cathfetch', '--date=' + date, *options], stdout=slave, check=True)
    finally:
        os.close(slave)
    contents = b''
    try:
        while True:
            block = os.read(master, 4096)
            if not block: break
            contents += block
    except OSError:
        pass # Linux signals the closed slave with EIO.
    finally:
        os.close(master)
    return contents.decode().splitlines()

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
# Use a real pseudo-terminal to verify ioctl width detection. COLUMNS is set
# deliberately wrong, so these checks would fail if it overrode TIOCGWINSZ.
python3 - <<'PYLAYOUT'
import errno
import fcntl
import os
import pty
import re
import struct
import subprocess
import termios
import unicodedata

ansi = re.compile(r'\x1b\[[0-?]*[ -/]*[@-~]')

def visible_width(text):
    return sum(0 if unicodedata.combining(char) else
               2 if unicodedata.east_asian_width(char) in ('W', 'F') else 1
               for char in ansi.sub('', text))

def check(output, width, cross):
    rows = ansi.sub('', output).splitlines()
    assert rows and all(visible_width(row) <= width for row in rows), (width, rows)
    if cross and width >= 55:
        artwork = ['          ███'] * 8
        artwork[2] = '      ███████████'
        assert all(row.startswith(art) for row, art in zip(rows, artwork))
        assert rows[1].endswith('Tuesday, 8 December 2026')
        # Strip artwork and its padding by cells, independently of the renderer.
        text_rows = [row[len(art):].lstrip() for row, art in zip(rows[1:], artwork[1:])]
        assert ' '.join(text_rows).strip().endswith('· Solemnity')
        continuation = [row for row in rows[4:] if row.strip(' █')]
        for row in continuation:
            # Continuations follow artwork (13 cells) and twelve spaces.
            assert row.startswith('          ███' + ' ' * 12)
    elif cross:
        assert '███████████' in rows[2] and rows[8] == ''
        assert rows[9] == 'Tuesday, 8 December 2026'
    # Normalise wrapped whitespace and decoration to check the complete name.
    name_text = re.sub(r'[█●]', ' ', ' '.join(rows))
    name_text = ' '.join(name_text.split())
    assert 'The Immaculate Conception of the Blessed Virgin Mary · Solemnity' in name_text

for width in (120, 100, 80, 60, 40):
    for cross in (False, True):
        args = ['./build/cathfetch', '--date=08122026'] + (['--cross'] if cross else [])
        environment = dict(os.environ, COLUMNS='17')
        master, slave = pty.openpty()
        try:
            fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack('HHHH', 24, width, 0, 0))
            process = subprocess.Popen(args, stdout=slave, stderr=subprocess.PIPE, env=environment)
            os.close(slave)
            slave = None
            output = bytearray()
            while True:
                try:
                    chunk = os.read(master, 4096)
                    if not chunk:
                        break
                    output.extend(chunk)
                except OSError as error:
                    if error.errno == errno.EIO:
                        break
                    raise
            assert process.wait(timeout=10) == 0, process.stderr.read().decode()
            check(output.decode('utf-8'), width, cross)
        finally:
            os.close(master)
            if slave is not None:
                os.close(slave)
        # Redirected output uses a valid explicit COLUMNS value.
        redirected = subprocess.check_output(args, env=dict(os.environ, COLUMNS=str(width)), text=True)
        check(redirected, width, cross)

args = ['./build/cathfetch', '--date=08122026', '--cross']
no_columns = dict(os.environ)
no_columns.pop('COLUMNS', None)
fallback = subprocess.check_output(args, env=no_columns, text=True)
check(fallback, 80, True)
for invalid in ('', '0', '-1', 'abc', '40oops', '999999999999999999999999999999999'):
    assert subprocess.check_output(args, env=dict(os.environ, COLUMNS=invalid), text=True) == fallback
print('Terminal width and redirected-output tests passed')
PYLAYOUT
echo 'CLI tests passed'
