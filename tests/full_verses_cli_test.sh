#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

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
assert 'Text unavailable' not in run('--date=02032026', '--calendar=wales', '--full-verses')
martyrs = run('--date=25102025', '--calendar=wales', '--full-verses')
assert 'Readings · Douay–Rheims' in martyrs
assert 'assignment not yet verified' not in martyrs and 'Text unavailable' not in martyrs
assert 'Text unavailable:' not in run('--date=20102026', '--full-verses')
assert 'Have mercy on me' in run('--date=18022026', '--full-verses')
assert 'I will take the chalice' in run('--date=02042026', '--mass=evening', '--full-verses')
for date in ['12072025', '31082025']:
    text = run('--date='+date, '--full-verses')
    assert 'Text unavailable' not in text
    assert 'Cited verses absent from the Douay–Rheims text are omitted.' in ' '.join(text.split())
for date in ['08062023', '09062023', '10062023']:
    text = run('--date='+date, '--calendar=england', '--full-verses', width=120)
    assert 'No verified Douay–Rheims correspondence for this Tobit passage' in ' '.join(text.split())
charity = run('--date=05062023', '--calendar=england', '--full-verses')
assert 'Text unavailable' not in charity
assert 'way of truth' in ' '.join(charity.split()) and 'But every day gave' in charity
raphael = run('--date=10062023', '--calendar=england', '--full-verses')
assert 'For I am the angel Raphael' in raphael
assert 'canticle differs in both verse divisions and content' in ' '.join(raphael.split())
print('Full-verse CLI, alternatives and wrapping tests passed')
PY
