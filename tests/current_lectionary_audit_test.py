#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

import copy
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('current_audit', ROOT / 'tools/current_lectionary_audit.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class CurrentAuditTests(unittest.TestCase):
    def test_dual_psalm_conventions_are_explicit(self):
        self.assertEqual(audit.canonical('Ps 51(50):3-4', 'lxx'), 'PSA:50:3-4')
        self.assertEqual(audit.canonical('Ps 51(50):3-4', 'hebrew'), 'PSA:51:3-4')
        self.assertIsNone(audit.canonical('Ps 51(50):3-4'))

    def test_partial_endpoints_are_not_certified_by_anchor(self):
        self.assertEqual(audit.canonical('Mt 5:1-12a', anchor=True), audit.canonical('Matthew 5:1–12', anchor=True))
        self.assertNotEqual(audit.canonical('Mt 5:1-12a'), audit.canonical('Matthew 5:1–12'))

    def test_extract_keeps_only_complete_single_reading_blocks(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'ordo.txt'
            path.write_text('Mass: I Isa 2:1-5 Ps 122(121):1-4a, 8-9 II Rom 13:11-14a G Mt 24:37-44\n'
                            'Office: text which must not be stored\n\f'
                            'Mass: I Acts 1:1-11\nOffice: text\n'
                            'Mass: I Acts 2:1-11 Ps 104(103):1-2 G Jn 20:19-23\n')
            facts = audit.extract(path)
        self.assertEqual(len(facts['blocks']), 2)
        self.assertEqual(facts['blocks'][1]['source'], audit.SOURCE + '#page=2')
        self.assertNotIn('text which must', json.dumps(facts))
        self.assertEqual(facts['blocks'][0]['readings'][2]['reference'], 'Rom 13:11-14a')

    def test_report_is_fresh_and_never_claims_full_verification(self):
        assignments = json.loads((ROOT / 'data/lectionary/roman_references.json').read_text())
        report = json.loads((ROOT / 'data/lectionary/current_ordo_audit.json').read_text())
        audit.check(report, assignments)
        changed = copy.deepcopy(assignments)
        matched = next(r for r in report['entries'] if r['evidence'])
        entry = next(e for e in changed['entries'] if e['id'] == matched['id'])
        entry['readings'][0]['reference'] = 'John 1:1'
        with self.assertRaises(ValueError):
            audit.check(report, changed)

    def test_collated_limits_keep_exact_evidence_and_reject_stale_citations(self):
        assignments = json.loads((ROOT / 'data/lectionary/roman_references.json').read_text())
        ledger = json.loads((ROOT / 'data/lectionary/citation_precision_audit.json').read_text())
        audit.check_precision(ledger, assignments)
        changed = copy.deepcopy(ledger)
        changed['corrections'][0]['sources'][1]['reference'] = 'Acts 10:34, 37-43'
        with self.assertRaises(ValueError):
            audit.check_precision(changed, assignments)
        changed = copy.deepcopy(ledger)
        changed['retained_differences'][0]['reference'] = 'Eph 1:16-23'
        with self.assertRaises(ValueError):
            audit.check_precision(changed, assignments)


if __name__ == '__main__':
    unittest.main()
