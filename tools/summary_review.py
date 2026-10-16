#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

"""Validate stored editorial work and reproduce its review collection.

There is no prose generator here. Authors edit canonical JSON; this tool checks
it against the C++ Scripture engine and exports it without editorial changes.
"""
import argparse
import collections
import csv
import hashlib
import itertools
import io
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
CANONICAL = ROOT / 'data/summaries/roman_summaries.json'
DESTINATION = ROOT / 'docs/scripture-summaries'
NT = set('MAT MRK LUK JHN ACT ROM 1CO 2CO GAL EPH PHP COL 1TH 2TH 1TI 2TI TIT PHM HEB JAS 1PE 2PE 1JN 2JN 3JN JUD REV'.split())
OCCASION_NAMES = {}
for calendar_path in sorted((ROOT / 'data/calendars').glob('*.json')):
    calendar = json.loads(calendar_path.read_text())
    for celebration in calendar.get('entries', []) + calendar.get('additions', []):
        OCCASION_NAMES[celebration['id']] = celebration['name']


def season(entry):
    day = entry['day_id']
    if day.startswith('temporal:advent:'):
        return 'Advent'
    if day.startswith(('temporal:christmas:', 'temporal:after_epiphany:')) or day in ('holy_family', 'baptism_of_lord') or day.startswith(('grc-12-25-', 'grc-01-01-', 'grc-01-06-')):
        return 'Christmas'
    if day.startswith(('temporal:lent:', 'temporal:after_ashes:', 'temporal:ash_wednesday')):
        return 'Lent'
    if day.startswith('temporal:holy_week:') or entry['mass'] == 'vigil' and day == 'easter_sunday':
        return 'Holy Week and Triduum'
    if day.startswith('temporal:easter:') or day in ('easter_sunday', 'ascension', 'pentecost'):
        return 'Easter'
    if day.startswith('temporal:ordinary:'):
        return 'Ordinary Time'
    return 'Feasts and propers'


def category(role):
    if role == 'epistle' or role == 'second':
        return 'second'
    if role.startswith('reading_'):
        return 'first'
    if role.startswith('psalm'):
        return 'psalm'
    return role


def sunday(entry):
    return bool(re.match(r'temporal:(advent|lent|easter|ordinary):\d+:0$', entry['day_id'])) or entry.get('day_type') == 'sunday'


def occasion_name(entry):
    """A review heading, not a new calendar or reading selector."""
    day = entry['day_id']
    if day in OCCASION_NAMES:
        return OCCASION_NAMES[day]
    match = re.fullmatch(r'temporal:(advent|lent|easter|ordinary):(\d+):([0-6])', day)
    if match:
        season_name, week, weekday = match.groups()
        weekday_name = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'][int(weekday)]
        period = 'Ordinary Time' if season_name == 'ordinary' else season_name.title()
        return f'{weekday_name}, week {week} of {period}'
    if day.startswith('grc-'):
        return day[10:].replace('-', ' ').capitalize()
    return day.removeprefix('temporal:').replace(':', ' · ').replace('_', ' ').replace('-', ' ').capitalize()


def natural_key(text):
    # Week 2 precedes week 10 in a reading collection, unlike a lexical sort.
    return [(0, int(part)) if part.isdigit() else (1, part) for part in re.split(r'(\d+)', text)]


def load_and_validate():
    data = json.loads(CANONICAL.read_text())
    lectionary = json.loads((ROOT / 'data/lectionary/roman_references.json').read_text())
    assignments = {e['id']: e for e in lectionary['entries']}
    bible = json.loads((ROOT / 'build/en_dra.json').read_text())
    # These candidates come from the production parser, including its guarded
    # versification rules; Python does not reinterpret citation syntax.
    catalogue = {e['id']: e for e in json.loads(subprocess.check_output(
        [str(ROOT / 'build/summary_catalogue')], cwd=ROOT, text=True))}
    assert data['schema_version'] == 1 and data['policy'] == 'universal_v1'
    assert hashlib.sha256((ROOT / 'data/bibles/en_dra.xml').read_bytes()).hexdigest() == bible['xml_sha256']
    seen = set()
    for entry in data['entries']:
        identity = entry['reading_set_id']
        assert identity in assignments and identity not in seen, identity
        seen.add(identity)
        assignment = assignments[identity]
        for field, source in [('occasion', 'day_id'), ('cycle', 'cycle'), ('mass', 'mass'), ('reference_status', 'verification')]:
            assert entry[field] == assignment[source], (identity, field)
        reading = next(r for r in catalogue[identity]['readings'] if r['role'] == entry['category'])
        assert entry['source_reading'] == reading['reference'], identity
        assert any(v['book'] == entry['book_id'] and v['chapter'] == entry['chapter'] and
                   v['verse'] == entry['verse'] and v['quotation'] == entry['quotation']
                   for v in reading['candidates']), identity
        book = bible['books'][entry['book_id']]
        assert entry['reference'] == f"{book['name']} {entry['chapter']}:{entry['verse']}", identity
        assert entry['quotation'] == book['chapters'][str(entry['chapter'])][str(entry['verse'])], identity
        assert entry['testament'] == ('new' if entry['book_id'] in NT else 'old'), identity
        assert entry['translation'] == bible['translation'], identity
        assert entry['quotation_source'] == bible['source'], identity
        assert entry['source_xml_sha256'] == bible['xml_sha256'], identity
        assert entry['technical_validation'] == 'checked', identity
        assert entry['quotation_verification'] == 'bundled_source_checked', identity
        assert entry['editorial_status'] in ('ai_draft', 'human_reviewed'), identity
        if entry['editorial_status'] == 'human_reviewed':
            assert entry['reviewer'] and entry['review_date'], identity
        for field in ['explanation', 'liturgical_connection', 'rationale']:
            assert isinstance(entry[field], str) and entry[field].strip(), (identity, field)
        assert 40 <= len((entry['explanation'] + ' ' + entry['liturgical_connection']).split()) <= 70, identity
        assert all(ord(c) >= 32 for value in entry.values() if isinstance(value, str) for c in value), identity
    # Corrections are reference facts, never substitutions made by the summary
    # selector. Keep their evidence ledger aligned with the canonical readings.
    audit = json.loads((ROOT / 'data/lectionary/summary_reference_audit.json').read_text())
    for correction in audit['corrections']:
        assignment = assignments[correction['id']]
        reading = next(r for r in assignment['readings'] if r['role'] == correction['role'])
        assert reading['reference'] == correction['reference'], correction['id']
        assert correction['sources'] and all(s['url'].startswith('https://') for s in correction['sources'])
    return assignments, {e['reading_set_id']: e for e in data['entries']}


def reports(assignments, summaries):
    missing = sorted(set(assignments) - summaries.keys())
    active = {k for k, e in assignments.items() if 'grc' in e['calendars'] and not e.get('partial')}
    coverage = dict(total_stored_reading_sets=len(assignments), v1_universal_complete_sets=len(active),
        reference_sets_current_edition_verified=sum(e['verification'] == 'current_edition' for e in assignments.values()),
        reference_sets_published_table=sum(e['verification'] == 'published_table' for e in assignments.values()),
        sets_with_summaries=len(summaries), v1_sets_with_summaries=len(active & summaries.keys()),
        distinct_commentaries=len({e['explanation'] for e in summaries.values()}),
        quotations_bundled_source_checked=len(summaries), quotations_independently_proofread=0,
        ai_drafts=sum(e['editorial_status'] == 'ai_draft' for e in summaries.values()),
        human_reviewed=sum(e['editorial_status'] == 'human_reviewed' for e in summaries.values()),
        missing_summaries=missing, v1_missing_summaries=sorted(active - summaries.keys()))
    distribution = collections.defaultdict(collections.Counter)
    for identity, e in summaries.items():
        source = assignments[identity]
        group = season(source) + ' / ' + source['cycle']
        distribution[group][category(e['category'])] += 1
        if sunday(source):
            distribution['Sundays / ' + source['cycle']][category(e['category'])] += 1
            distribution['Sundays testament / ' + source['cycle']][e['testament']] += 1
    # Shared commentary for the same verse is deliberate and listed separately.
    # Similar wording for different verses needs editorial investigation.
    unique = {}
    for e in summaries.values():
        unique.setdefault((e['book_id'], e['chapter'], e['verse'], e['explanation']), e)
    near_duplicates = []
    for a, b in itertools.combinations(unique.values(), 2):
        wa = set(re.findall(r'\w+', a['explanation'].lower()))
        wb = set(re.findall(r'\w+', b['explanation'].lower()))
        score = len(wa & wb) / len(wa | wb)
        if score >= .65:
            near_duplicates.append(dict(first=a['reading_set_id'], second=b['reading_set_id'], similarity=round(score, 3)))
    coverage['near_duplicate_commentary_review'] = near_duplicates
    coverage['scope_note'] = ('Coverage of the stored 807-set catalogue, including archival propers, is not proof of complete '
        'Catholic Lectionary coverage. Default passages are summarised; subverse alternatives are not separate editorial records. '
        'Technical matching is not independent transcription proofreading, current-edition certification or human editorial approval.')
    return coverage, dict(sorted(distribution.items()))


def alternative_compatibility(assignments, summaries):
    catalogue = json.loads(subprocess.check_output([str(ROOT / 'build/summary_catalogue')], cwd=ROOT, text=True))
    rows = []
    for entry in catalogue:
        summary = summaries.get(entry['id'])
        if not summary:
            continue
        assignment = assignments[entry['id']]
        for reading in entry['readings']:
            for option in reading['alternatives']:
                unaffected = reading['role'] != summary['category']
                included = any(v['book'] == summary['book_id'] and v['chapter'] == summary['chapter'] and
                               v['verse'] == summary['verse'] for v in option['candidates'])
                rows.append(dict(reading_set_id=entry['id'], role=reading['role'], alternative=option['reference'],
                    summary_compatible=unaffected or included,
                    archival_only='grc' not in assignment['calendars'] or assignment.get('partial', False)))
    return rows


def generate(assignments, summaries):
    coverage, distribution = reports(assignments, summaries)
    alternatives = alternative_compatibility(assignments, summaries)
    coverage['stored_non_acclamation_passage_alternatives'] = len(alternatives)
    coverage['passage_alternatives_compatible_with_summary'] = sum(r['summary_compatible'] for r in alternatives)
    coverage['v1_incompatible_passage_alternatives'] = sum(not r['summary_compatible'] and not r['archival_only'] for r in alternatives)
    coverage['archival_incompatible_passage_alternatives'] = sum(not r['summary_compatible'] and r['archival_only'] for r in alternatives)
    files = {}
    sections = collections.defaultdict(list)
    for identity, entry in sorted(assignments.items(), key=lambda item: natural_key(item[0])):
        section = season(entry)
        if section == 'Ordinary Time':
            section += ' — Cycle ' + entry['cycle']
            if entry['cycle'] in ['I', 'II']:
                week = int(entry['day_id'].split(':')[2])
                section += ' — Weeks ' + ('1–17' if week <= 17 else '18–34')
        sections[section].append(identity)
    index = ['# Scripture summary review', '',
        'Read the summaries by season and cycle below. Each entry includes its selected verse, '
        'commentary, sources and review status.', '',
        'The summaries were generated with AI assistance. Quotations match '
        'the bundled Douay–Rheims text, which has not been independently proofread. Complete checks '
        'against the current England and Wales Lectionary are still outstanding.', '',
        'V1 uses universal readings with local feast dates. Archived memorial and national proper '
        'sets are included for review, but are not selected by the app.', '',
        'To update this collection from `data/summaries/roman_summaries.json`, run `make summary-review`. '
        'See the [data guide](../../data/summaries/README.md) for editing instructions.', '',
        '| Measure | Count |', '| --- | ---: |']
    measures = [
        ('total_stored_reading_sets', 'Stored reading sets'),
        ('v1_universal_complete_sets', 'Complete universal sets used by v1'),
        ('sets_with_summaries', 'Sets with summaries'),
        ('v1_sets_with_summaries', 'V1 sets with summaries'),
        ('distinct_commentaries', 'Distinct commentaries'),
        ('reference_sets_current_edition_verified', 'Sets verified against the current edition'),
        ('quotations_bundled_source_checked', 'Quotations matched to the bundled Bible'),
        ('quotations_independently_proofread', 'Quotations independently proofread'),
    ]
    for key, label in measures:
        index.append(f"| {label} | {coverage[key]} |")
    index += [f"| Missing summaries | {len(coverage['missing_summaries'])} |", '', '## Collection', '']
    index += [f"The chosen verses also fit {coverage['passage_alternatives_compatible_with_summary']} of the "
        f"{len(alternatives)} stored passage alternatives. "
        f"{coverage['archival_incompatible_passage_alternatives']} archived proper alternatives need another selection; "
        f"{coverage['v1_incompatible_passage_alternatives']} of these conflicts affect v1. "
        'See [the compatibility report](alternative-compatibility.json) for the individual records.', '']
    for group, ids in sorted(sections.items(), key=lambda item: natural_key(item[0])):
        filename = re.sub(r'[^a-z0-9]+', '-', group.lower()).strip('-') + '.md'
        completed = sum(identity in summaries for identity in ids)
        index.append(f'- [{group}]({filename}): {completed}/{len(ids)} reading sets.')
        lines = [f'# {group}', '', 'Generated editorial collection. All quoted addresses use Douay–Rheims numbering.', '']
        # Cycle is a heading, so the long Ordinary Time file remains navigable.
        for cycle in ['A', 'B', 'C', 'I', 'II', 'any']:
            cycle_ids = [identity for identity in ids if assignments[identity]['cycle'] == cycle]
            if not cycle_ids:
                continue
            lines += [f'## Cycle {cycle}', '']
            for identity in cycle_ids:
                a = assignments[identity]
                lines += [f'### {occasion_name(a)}', '', f"Reading set: `{identity}` · Mass form: **{a['mass']}**", '']
                if identity not in summaries:
                    lines += ['**Summary unavailable: editorial selection and commentary not yet completed.**', '']
                else:
                    e = summaries[identity]
                    lines += [f"**{e['reference']}** — {e['translation']}", '', '> ' + e['quotation'], '',
                        e['explanation'] + ' ' + e['liturgical_connection'], '',
                        f"Category: **{e['category']}** · Testament: **{e['testament']}** · Appointed reading: {e['source_reading']}", '',
                        '**Selection rationale:** ' + e['rationale'], '',
                        f"Editorial status: **{e['editorial_status']}** · Technical validation: {e['technical_validation']} · "
                        f"Quotation: {e['quotation_verification']} · Reference status: {e['reference_status']}", '',
                        f"[Quotation source]({e['quotation_source']}) · XML SHA-256: `{e['source_xml_sha256']}`", '']
                lines += [f"[Reading assignment source]({a['source']})", '', a['source_note'], '']
                if 'grc' not in a['calendars'] or a.get('partial'):
                    lines += ['**Archival proper: not selected under v1 universal-reading policy.**', '']
        files[filename] = '\n'.join(lines)
    index += ['', '## Distribution', '', '| Season or Sunday group / cycle | First | Psalm | Second | Gospel |',
              '| --- | ---: | ---: | ---: | ---: |']
    for group, counts in distribution.items():
        if 'testament' not in group:
            index.append(f"| {group} | " + ' | '.join(str(counts.get(k, 0)) for k in ['first', 'psalm', 'second', 'gospel']) + ' |')
    index += ['', 'These counts are per stored reading set, so a shared passage can appear under several '
        'cycles or Mass forms. The Sunday rows include stored alternatives, not just the Sundays in a '
        'particular year. For full figures and possible repeated commentary, see '
        '[coverage](coverage.json) and [distribution](distribution.json).', '']
    files['README.md'] = '\n'.join(index)
    files['coverage.json'] = json.dumps(coverage, ensure_ascii=False, indent=2) + '\n'
    files['distribution.json'] = json.dumps(distribution, ensure_ascii=False, indent=2) + '\n'
    files['alternative-compatibility.json'] = json.dumps(alternatives, ensure_ascii=False, indent=2) + '\n'
    # These are actual date selections from the production resolver. Keeping
    # them separate from stored-set counts prevents false coverage claims.
    yearly = io.StringIO()
    writer = csv.writer(yearly, delimiter='\t', lineterminator='\n')
    writer.writerow(['year', 'calendar', 'days', 'with_summary', 'missing', 'no_daytime_mass',
                     'current_edition_verified', 'human_reviewed', 'selected_permitted_alternative'])
    for name in ['grc', 'europe', 'england', 'wales']:
        for year in range(2024, 2031):
            output = subprocess.check_output([str(ROOT / 'build/summary_year'), str(year), name], cwd=ROOT, text=True)
            rows = list(csv.DictReader(io.StringIO(output), delimiter='\t'))
            writer.writerow([year, name, len(rows), sum(r['summary'] == 'available' for r in rows),
                sum(r['summary'] == 'missing' for r in rows), sum(r['summary'] == 'no_daytime_mass' for r in rows),
                sum(r['reference_verification'] == 'current_edition' for r in rows),
                sum(r['editorial_status'] == 'human_reviewed' for r in rows),
                sum(r['permitted_alternative'] == 'yes' for r in rows)])
    files['year-coverage.tsv'] = yearly.getvalue()
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['export', 'check'])
    args = parser.parse_args()
    assignments, summaries = load_and_validate()
    files = generate(assignments, summaries)
    if args.command == 'export':
        DESTINATION.mkdir(parents=True, exist_ok=True)
        for name, text in files.items():
            (DESTINATION / name).write_text(text)
        # Only remove obsolete generated chapters; never user-authored notes.
        for name in ['ordinary-time.md']:
            obsolete = DESTINATION / name
            if obsolete.exists():
                obsolete.unlink()
    else:
        for name, text in files.items():
            assert (DESTINATION / name).read_text() == text, f'Stale review export: {name}'
    print(f"Summary validation: {len(summaries)}/{len(assignments)} sets; {len(assignments)-len(summaries)} missing; "
          f"{sum(e['editorial_status']=='human_reviewed' for e in summaries.values())} human-reviewed.")


if __name__ == '__main__':
    main()
