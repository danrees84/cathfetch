#!/bin/sh
set -eu
python3 - <<'PY'
import os
import re
import subprocess
import unicodedata

app = './build/cathfetch'
def run(*args, width=80):
    return subprocess.check_output([app, *args], text=True, env=dict(os.environ, COLUMNS=str(width)))

for args in [('--readings', '--readings'), ('--mass=day', '--mass=night'), ('--mass=invalid',)]:
    result = subprocess.run([app, *args], text=True, capture_output=True)
    assert result.returncode and result.stderr
for width in [120, 100, 80, 60, 40]:
    for cross in [[], ['--cross']]:
        for date, mass in [('08122026', []), ('04042026', ['--mass=vigil']), ('14102026', []), ('25122026', [])]:
            text = run('--date='+date, '--readings', *mass, *cross, width=width)
            text = re.sub(r'\x1b\[[0-?]*[ -/]*[@-~]', '', text)
            for line in text.splitlines():
                cells = sum(0 if unicodedata.combining(c) else
                            2 if unicodedata.east_asian_width(c) in ('W', 'F') else 1 for c in line)
                assert cells <= width, (width, line)
            assert 'Readings' in text
assert 'Luke 1:26-38' in run('--date=08122026', '--readings', width=120)
christmas = run('--date=25122026', '--readings', width=240)
assert christmas.split('Readings:\n')[1] == ('Isa 52:7-10; Ps 97:1, 2-3a, 3b-4, 5-6; '
    'Heb 1:1-6; John 1:1-18 or John 1:1-5, 9-14 (shorter)\n')
for removed in ['collation pending', 'Sunday B /', 'Day Mass', 'First:', 'Gospel:',
                'Other reading sets', 'Acclamation:']:
    assert removed not in christmas
assert 'Readings:\nIsa 2:1-5;' in run('--date=30112025', '--readings', width=120)
assert 'Matt 1:16, 18-21, 24a' in ' '.join(run('--date=19032026', '--readings').split())
assert 'Acts 10:34a, 37-43' in ' '.join(run('--date=05042026', '--readings').split())
assert 'Luke 2:1-14' in run('--date=25122026', '--mass=night', width=120)
assert 'No daytime Mass' in run('--date=04042026', '--readings', width=120)
assert 'Readings:\nGal 5:18-25;' in run('--date=14102026', '--readings', width=120)
assert 'Isaiah 61:1-3, 10-11' in run('--date=02032026', '--calendar=wales', '--readings', width=120)
assert 'assignment not yet verified' in run('--date=25102025', '--calendar=wales', '--readings', width=120)
assert 'with references;' in subprocess.run(['./build/lectionary_year', '2026', 'grc'],
    text=True, capture_output=True, check=True).stderr
assert 'Readings' not in run('--date=08122026', width=120)
print('Lectionary CLI and wrapping tests passed')
PY
