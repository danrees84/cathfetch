#!/usr/bin/env python3
"""Check user-visible selection, independent flags and terminal cell widths."""
import os
import re
import subprocess
import unicodedata

def run(*args, width=80):
    result = subprocess.run(['./build/cathfetch', *args], capture_output=True, text=True,
                            env={**os.environ, 'COLUMNS': str(width)})
    assert result.returncode == 0, result.stderr
    assert not result.stderr
    assert '\x1b' not in result.stdout, 'Redirected output must be plain text'
    return re.sub(r'\x1b\[[0-?]*[ -/]*[@-~]', '', result.stdout)

for width in [120, 100, 80, 60, 40]:
    for cross in [[], ['--cross']]:
        for readings in [[], ['--readings']]:
            output = run('--date=30112025', '--calendar=england', '--summary', *cross, *readings, width=width)
            assert 'Scripture summary' in output
            for notice in ['AI draft', 'not human-reviewed', 'Lectionary references provisional',
                           'Human-reviewed editorial selection']:
                assert notice not in output
            assert 'Isaiah 2:5' in output and 'Douay–Rheims' in output
            assert ('Readings:' in output) == bool(readings)
            for line in output.splitlines():
                cells = sum(0 if unicodedata.combining(c) else 2 if unicodedata.east_asian_width(c) in 'WF' else 1 for c in line)
                assert cells <= width, (width, cells, line)
            if cross:
                assert '███████████' in output
assert 'Scripture summary' not in run('--date=30112025')
assert 'No daytime Mass' in run('--date=04042026', '--summary')
assert 'Romans 6:4' in run('--date=04042026', '--mass=vigil', '--summary')
assert 'Isaiah 52:7' in run('--date=25122026', '--summary')
assert 'Titus 2:11' in run('--date=25122026', '--mass=night', '--summary')
assert 'Luke 1:38' in run('--date=08042024', '--summary')
assert 'Luke 6:36' in run('--date=02032026', '--calendar=wales', '--summary')
assert 'Deuteronomy 8:3' in run('--date=04062026', '--calendar=grc', '--summary')
assert 'Deuteronomy 8:3' in run('--date=07062026', '--calendar=england', '--summary')
combined = run('--date=25122026', '--summary', '--full-verses')
assert 'Scripture summary' in combined and 'Readings · Douay' in combined
for args in [('--summary', '--summary'), ('--summary=yes',)]:
    result = subprocess.run(['./build/cathfetch', *args], text=True, capture_output=True)
    assert result.returncode == 1 and not result.stdout and result.stderr
print('Summary CLI: independent flags, Mass forms, transfers and 120/100/80/60/40-column redirected rendering passed.')
