#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

"""Exercise build-tree, installed and overridden data paths from another cwd."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

project = Path(__file__).resolve().parents[1]
app = project / 'build/cathfetch'
env = {key: value for key, value in os.environ.items() if key != 'CATHFETCH_DATA_DIR'}
env['COLUMNS'] = '100'
# The packaging check starts its own make process; inherited jobserver file
# descriptors are not available through subprocess's normal closed-fd boundary.
env.pop('MAKEFLAGS', None)
env.pop('MFLAGS', None)


def run(binary, cwd, extra_env=None, success=True):
    result = subprocess.run([str(binary), '--date=30112025', '--readings', '--summary'],
                            cwd=cwd, env={**env, **(extra_env or {})},
                            capture_output=True, text=True)
    if success:
        assert result.returncode == 0, result.stderr
        assert not result.stderr
        assert 'Isaiah 2:5' in result.stdout
    else:
        assert result.returncode != 0 and not result.stdout
        assert 'CATHFETCH_DATA_DIR' in result.stderr
    return result.stdout


with tempfile.TemporaryDirectory(prefix='cathfetch-data-') as temporary:
    root = Path(temporary)
    expected = run(app, root)
    # Install into a temporary prefix, including the generated Bible index.
    subprocess.run(['make', '--no-print-directory', 'install', 'PREFIX=/usr',
                    f'DESTDIR={root}'], cwd=project, env=env,
                   check=True, stdout=subprocess.DEVNULL)
    installed = root / 'usr/bin/cathfetch'
    data = root / 'usr/share/cathfetch'
    for notice in ['LICENSE', 'LICENSING.md', 'licenses/nlohmann-json-MIT.txt']:
        assert (data / notice).read_bytes() == (project / notice).read_bytes()
    assert run(installed, root) == expected
    # A standalone copy can use an explicit data root. Paths containing spaces
    # exercise packaging/override handling as well as executable discovery.
    standalone = root / 'standalone app'
    shutil.copy2(app, standalone)
    relocated_data = root / 'data with spaces'
    data.rename(relocated_data)
    assert run(standalone, root, {'CATHFETCH_DATA_DIR': str(relocated_data)}) == expected
    run(installed, root, {'CATHFETCH_DATA_DIR': str(root / 'missing')}, success=False)
    # Existing but incomplete overrides must not borrow files from the build.
    incomplete = root / 'incomplete'
    incomplete.mkdir()
    run(app, root, {'CATHFETCH_DATA_DIR': str(incomplete)}, success=False)
    # Invalid data must still be rejected by the daily selected-entry validator.
    import json
    summary_file = relocated_data / 'summaries/roman_summaries.json'
    document = json.loads(summary_file.read_text())
    for entry in document['entries']:
        if entry['reference'] == 'Isaiah 2:5':
            entry['quotation'] += ' invented'
    summary_file.write_text(json.dumps(document))
    result = subprocess.run([str(standalone), '--date=30112025', '--summary'], cwd=root,
                            env={**env, 'CATHFETCH_DATA_DIR': str(relocated_data)},
                            capture_output=True, text=True)
    assert result.returncode != 0 and not result.stdout
    assert 'quotation' in result.stderr
print('Data discovery: external cwd, staged installation, explicit override and invalid selected data passed.')
