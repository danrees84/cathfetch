# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

"""Synthetic parser tests and independently sourced Advent/date regression cases."""
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / 'tools'))
from universalis_audit import parse, compare_names, citation_key, reconcile


class UniversalisAuditTest(unittest.TestCase):
    def test_calendar_options_and_rank(self):
        page = parse('''<table id="yearly-calendar"><tr><th>October</th></tr>
            <tr><td><a href="https://universalis.com/europe.england.birmingham/20261019/today.htm">Mon 19</a></td>
            <td>Monday of week 29 in Ordinary Time<br>or A local saint</td></tr>
            <tr><td><a href="https://universalis.com/europe.england.birmingham/20261020/today.htm">Tue 20</a></td>
            <td><span class="rank-feast">A local feast</span> Feast</td></tr></table>''')
        self.assertEqual(page.calendar[0]['date'], '2026-10-19')
        self.assertEqual(len(page.calendar[0]['observances']), 2)
        self.assertEqual(page.calendar[1]['rank_classes'], ['rank-feast'])

    def test_mass_citations_only_and_duplicate_roles(self):
        page = parse('''<div id="datename">Monday 19 October 2026</div>
            <span id="feastname">A &amp; B</span><p>Year: A(II).</p>
            <table class="each"><tr><th>First reading</th></tr><tr><th>Ephesians 2:1-10</th></tr></table>
            <p>This synthetic reading body must never be retained.</p>
            <table class="each"><tr><th>Gospel</th><th>Luke 12:13-21</th></tr></table>
            <table class="each"><tr><th>First reading</th><th>Romans 8:22-27</th></tr></table>''')
        self.assertEqual(page.year_cycle, {'sunday': 'A', 'weekday': 'II'})
        self.assertEqual(page.metadata['feastname'], 'A & B')
        self.assertEqual([r['role'] for r in page.readings], ['first', 'gospel', 'first'])
        self.assertNotIn('synthetic reading body', json.dumps(page.readings))

    def test_access_notice_has_no_readings(self):
        self.assertEqual(parse('<h1>Other dates</h1><p>The week ahead.</p>').readings, [])

    def test_temporal_names_and_negative_cases(self):
        self.assertEqual(compare_names('Monday of week 29 in Ordinary Time',
            'Monday of the 29th Week in Ordinary Time'), 'temporal_week_match')
        self.assertEqual(compare_names('Monday of the 2nd week of Eastertide',
            'Monday of the Second Week of Easter'), 'temporal_week_match')
        for name in ['Tuesday of the 29th Week in Ordinary Time',
                     'Monday of the 28th Week in Ordinary Time', 'A diocesan feast']:
            self.assertEqual(compare_names('Monday of week 29 in Ordinary Time', name),
                             'name_or_local_proper_review')

    def test_citations_never_hide_subverse_differences(self):
        self.assertEqual(citation_key('Eph 2:1–10'), citation_key('Ephesians 2:1-10'))
        self.assertNotEqual(citation_key('1 Thess 1:1-5'), citation_key('1 Thessalonians 1:1-5b'))

    def test_public_source_advent_and_cycles(self):
        report = json.loads((ROOT / 'data/lectionary/universalis_reconciliation_2026.json').read_text())
        facts = {k: report[k] for k in ['year', 'source_calendar', 'calendar_source', 'retrieved_on']}
        facts['calendar'] = [{k: day[k] for k in ['date', 'observances', 'rank_classes']}
                             for day in report['dates']]
        facts['mass_days'] = [{k: day[k] for k in ['date', 'source', 'actual_url', 'status',
                              'source_cycles', 'observance', 'readings']}
                             for day in report['reading_comparisons']]
        live = reconcile(facts, 'england')
        self.assertEqual(live, report, 'Reconciliation report must agree with the current engine')
        self.assertEqual(report['source_advent_start'], '2026-11-29')
        self.assertEqual(len(report['dates']), 365)
        self.assertEqual(len({d['date'] for d in report['dates']}), 365)
        self.assertTrue(all(d['liturgical_year_match'] for d in report['dates']))
        days = {d['date']: d for d in report['dates']}
        self.assertEqual(days['2026-11-28']['source_liturgical_year'], 2026)
        self.assertEqual(days['2026-11-29']['source_liturgical_year'], 2027)
        self.assertEqual(days['2026-11-29']['sunday_cycle'], 'B')
        readings = {d['date']: d for d in report['reading_comparisons']}
        self.assertTrue(readings['2026-10-19']['cycle_match'])
        self.assertIn({'role': 'gospel', 'reference': 'Luke 12:13-21'}, readings['2026-10-19']['readings'])


if __name__ == '__main__':
    unittest.main()
