#!/bin/sh
set -eu
python3 - <<'PY'
import os
import re
import subprocess
import unicodedata

def run(*args, width=120):
    return subprocess.check_output(['./build/cathfetch', *args], text=True,
        env=dict(os.environ, COLUMNS=str(width)))

duplicate = subprocess.run(['./build/cathfetch', '--full-verses', '--full-verses'],
                           text=True, capture_output=True)
assert duplicate.returncode and 'Duplicate option' in duplicate.stderr
for width in [120, 100, 80, 60, 40]:
    for cross in [[], ['--cross']]:
        for date, options in [('25122026', []), ('08122026', []), ('19102026', []), ('04042026', ['--mass=vigil'])]:
            text = run('--date='+date, '--full-verses', *options, *cross, width=width)
            assert 'Douay–Rheims' in text
            plain = re.sub(r'\x1b\[[0-?]*[ -/]*[@-~]', '', text)
            for line in plain.splitlines():
                cells = sum(0 if unicodedata.combining(c) else
                    2 if unicodedata.east_asian_width(c) in ('W', 'F') else 1 for c in line)
                assert cells <= width, (width, line)
            if cross and width >= 55:
                # Verse rows below the artwork must retain the text column.
                assert all(not line or line.startswith(' '*23) for line in plain.splitlines()[8:])
christmas = run('--date=25122026', '--full-verses')
assert 'In the beginning was the Word' in christmas
assert 'How beautiful upon the mountains' in christmas
assert 'Whole verses shown' in christmas
assert 'Alternative: John 1:1-5, 9-14 (shorter)' in christmas
assert 'Text unavailable' not in christmas
assert 'Text unavailable' not in run('--date=08122026', '--full-verses')
assert 'Word was God' not in run('--date=25122026', '--readings')
assert 'Word was God' not in run('--date=25122026')
assert christmas == run('--date=25122026', '--readings', '--full-verses')
assert christmas == run('--date=25122026', '--full-verses', '--readings')
assert 'No daytime Mass' in run('--date=04042026', '--full-verses')
assert 'assignment not yet verified' in run('--date=02032026', '--calendar=wales', '--full-verses')
assert 'Text unavailable:' in run('--date=20102026', '--full-verses')
print('Full-verse CLI, alternatives and wrapping tests passed')
PY
