#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

"""Check the stored citation-comparison ledger against the current assignments.

This checks ledger freshness, not ecclesiastical accuracy. No network access or
third-party Python package is needed. Independent evidence is citation-only.
"""

import collections
import json
from pathlib import Path


def main():
    root = Path(__file__).resolve().parents[1]
    data = json.loads((root / 'data/lectionary/roman_references.json').read_text())
    audit = json.loads((root / 'data/lectionary/reference_audit.json').read_text())
    assignments = {entry['id']: entry for entry in data['entries']}
    ledger = {entry['id']: entry for entry in audit['entries']}
    if len(ledger) != len(audit['entries']) or ledger.keys() != assignments.keys():
        raise ValueError('Audit identities are duplicated or out of date')
    statuses = collections.Counter()
    original = collections.Counter()
    comparisons = collections.Counter()
    for identifier, row in ledger.items():
        current = assignments[identifier]
        if row['assignment_source'] != current['source']:
            raise ValueError('Audit source is stale: ' + identifier)
        statuses[row['status']] += 1
        if row['original_600']:
            original[row['status']] += 1
        if row['evidence']:
            current_slots = {r['role']: r['reference'] for r in current['readings']}
            compared_slots = {r['role']: r['stored'] for r in row['evidence']['comparisons']}
            if current_slots != compared_slots:
                raise ValueError('Audit citations are stale: ' + identifier)
            comparisons.update(r['result'] for r in row['evidence']['comparisons'])
    print(f'{len(ledger)} assignments in the audit ledger')
    for label, counts in [('All assignments', statuses), ('Original 600', original),
                          ('Compared slots', comparisons)]:
        print(label + ': ' + ', '.join(f'{key}={value}' for key, value in sorted(counts.items())))
    print('These are partial passage comparisons, not current-edition certification.')
    print(audit['remaining'])


if __name__ == '__main__':
    main()
