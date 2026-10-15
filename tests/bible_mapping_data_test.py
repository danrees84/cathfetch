#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

"""Offline tests for the versification data import, without downloading a Bible."""
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('bible_versification', ROOT / 'tools/bible_versification.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class VersificationImportTests(unittest.TestCase):
    def test_tobit_review_evidence_tracks_the_runtime_provisions(self):
        metadata = json.loads((ROOT / 'data/bibles/en_dra_metadata.json').read_text())
        audit = json.loads((ROOT / 'data/bibles/tobit_correspondence_audit.json').read_text())
        reviewed = set()
        for passage in audit['passages']:
            key = passage['key']
            self.assertNotIn(key, reviewed)
            reviewed.add(key)
            self.assertTrue(passage['note'])
            self.assertTrue(any(url.startswith('https://bible.usccb.org/') for url in passage['sources']))
            self.assertTrue(any(url.startswith('https://ebible.org/engDRA/') for url in passage['sources']))
            if passage['status'] == 'mapped':
                self.assertEqual(metadata['passage_mappings'][key], passage['native_reference'])
                self.assertNotIn(key, metadata['passage_diagnostics'])
            else:
                self.assertEqual(passage['status'], 'textual_difference')
                self.assertIsNone(passage['native_reference'])
                self.assertIn(key, metadata['passage_diagnostics'])
                self.assertNotIn(key, metadata['passage_mappings'])
        provisions = {key for key in metadata['passage_mappings'] if key.startswith('TOB')}
        provisions.update(key for key in metadata['passage_diagnostics'] if key.startswith('TOB'))
        self.assertEqual(reviewed, provisions)

    def derive(self, table):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'table.txt'
            source.write_text(table)
            xml = Path(directory) / 'bible.xml'
            xml.write_text('<bible><b id="GEN"><c n="1"><v n="1">a</v><v n="2">b</v>'
                           '</c></b><b id="PSA"><c n="97"><v n="1">a</v><v n="2">b</v>'
                           '</c><c n="98"><v n="1">c</v></c></b></bible>')
            metadata = {'versification_source': {'sha256': hashlib.sha256(source.read_bytes()).hexdigest()},
                        'books': {'GEN': {'aliases': ['Gen']}, 'PSA': {'aliases': ['Psa']}}}
            return module.derive(source, metadata, xml)

    def test_merged_titles_do_not_add_another_psalm(self):
        data = self.derive('$Psa.98\tEnglish KJV\tHebrew\tLatin\tGreek\n'
                           'OneToOne\tPsa.98:Title\tAbsent [=Psa.98:1]\tAbsent [=Psa.98:1]\tAbsent [=Psa.98:1]\n'
                           'OneToOne\tPsa.98:1-2\tPsa.98:1-2\tPsa.97:1-2\tPsa.97:1-2\n')
        self.assertEqual(data['maps']['hebrew']['PSA:98:1'], [['PSA', 97, 1]])
        self.assertEqual(data['maps']['greek']['PSA:97:2'], [['PSA', 97, 2]])

    def test_known_absence_and_merge_are_distinct(self):
        data = self.derive('$Gen.1\tEnglish KJV\tHebrew\tLatin\n'
                           'TextMayBeMissing\tGen.1:1\tGen.1:1\tAbsent\n'
                           'MergedPrevVerse\tGen.1:2\tGen.1:2\tAbsent [=Gen.1:1]\n')
        self.assertEqual(data['maps']['hebrew']['GEN:1:1'], [])
        self.assertIn('hebrew:GEN:1:1', data['omitted_verses'])
        self.assertEqual(data['maps']['hebrew']['GEN:1:2'], [['GEN', 1, 1]])
        self.assertNotIn('hebrew:GEN:1:2', data['omitted_verses'])

    def test_splits_retain_all_whole_target_verses(self):
        data = self.derive('$Gen.1\tEnglish KJV\tHebrew\tLatin\n'
                           'SubdividedVerse\tGen.1:1\tGen.1:1\tGen.1:1-2\n')
        self.assertEqual(data['maps']['hebrew']['GEN:1:1'], [['GEN', 1, 1], ['GEN', 1, 2]])
        self.assertEqual(data['evidence']['hebrew:GEN:1:1'], [2])

    def test_incompatible_latin_edition_blocks_conversion(self):
        data = self.derive('$Gen.1\tEnglish KJV\tHebrew\tLatin\n'
                           'TEST: If...\tGen.1:2=Last\tGen.1:2=Last\tGen.1:3=Last\n'
                           'OneToOne\tGen.1:1\tGen.1:1\tGen.1:2\n')
        self.assertEqual(data['maps']['hebrew']['GEN:1:1'], [])
        self.assertNotIn('hebrew:GEN:1:1', data['omitted_verses'])

    def test_conflicting_profiles_are_not_unioned(self):
        data = self.derive('$Gen.1\tEnglish KJV\tHebrew\tLatin\n'
                           'OneToOne\tGen.1:1\tGen.1:1\tGen.1:1\n'
                           '$Gen.1\tEnglish KJV\tHebrew\tLatin\n'
                           'OneToOne\tGen.1:1\tGen.1:1\tGen.1:2\n')
        self.assertEqual(data['maps']['hebrew']['GEN:1:1'], [])
        self.assertIn('hebrew:GEN:1:1', data['conflicting_profiles'])


if __name__ == '__main__':
    unittest.main()
