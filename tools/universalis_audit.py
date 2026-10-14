#!/usr/bin/env python3
"""Collect public Universalis citation facts and reconcile them with Cathfetch.

No Scripture text is saved. Reading pages outside the public window are recorded
as unavailable; redirects are not worked around. Birmingham is a diocesan source,
not an implicit addition to Cathfetch's national calendars. Standard library only.
"""

import argparse
import csv
import datetime as dt
from html.parser import HTMLParser
import io
import json
from pathlib import Path
import re
import subprocess
import time
import unicodedata
from urllib.request import Request, urlopen


def clean(value):
    return ' '.join(value.split())


class FactParser(HTMLParser):
    """Keep calendar rows and citation headings, discarding reading bodies."""

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.table = None
        self.rows = []
        self.row = None
        self.cell = None
        self.calendar = []
        self.readings = []
        self.metadata = {}
        self.captures = []
        self.stack = []
        self.year_cycle = None

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag not in {'br', 'hr', 'img', 'input', 'link', 'meta', 'wbr'}:
            self.stack.append(tag)
        if attrs.get('id') in {'datename', 'feastname', 'calendar-heading'}:
            self.captures.append((len(self.stack), attrs['id'], []))
        if tag == 'table' and (attrs.get('id') == 'yearly-calendar' or
                               attrs.get('class') == 'each'):
            self.table = attrs.get('id', 'each')
            self.rows = []
        if self.table and tag == 'tr':
            self.row = {'cells': [], 'links': [], 'ranks': []}
        if self.row is not None and tag in {'td', 'th'}:
            self.cell = []
        if self.row is not None and tag == 'a' and 'href' in attrs:
            self.row['links'].append(attrs['href'])
        if self.row is not None and tag == 'span':
            self.row['ranks'].extend(c for c in attrs.get('class', '').split()
                                     if c.startswith('rank-'))
        if tag == 'br' and self.cell is not None:
            self.cell.append('\n')

    def handle_data(self, data):
        for _, _, parts in self.captures:
            parts.append(data)
        if self.cell is not None:
            self.cell.append(data)
        match = re.search(r'Year:\s*([ABC])\s*\((II?)\)', data)
        if match:
            self.year_cycle = {'sunday': match[1], 'weekday': match[2]}

    def handle_endtag(self, tag):
        if self.table and tag in {'td', 'th'} and self.cell is not None:
            self.row['cells'].append(''.join(self.cell))
            self.cell = None
        if self.table and tag == 'tr' and self.row is not None:
            self.rows.append(self.row)
            self.row = None
        if self.table and tag == 'table':
            if self.table == 'yearly-calendar':
                for row in self.rows:
                    dates = [re.search(r'/(\d{8})/today\.htm$', url) for url in row['links']]
                    dates = [m[1] for m in dates if m]
                    if len(dates) != 1 or len(row['cells']) < 2:
                        continue
                    date = dt.datetime.strptime(dates[0], '%Y%m%d').date()
                    names = [clean(s) for s in row['cells'][1].split('\n') if clean(s)]
                    self.calendar.append({'date': date.isoformat(), 'observances': names,
                                          'rank_classes': row['ranks']})
            else:
                cells = [clean(cell) for row in self.rows for cell in row['cells']]
                roles = {'First reading': 'first', 'Responsorial Psalm': 'psalm',
                         'Second reading': 'second', 'Gospel Acclamation': 'acclamation',
                         'Gospel': 'gospel'}
                if len(cells) == 2 and cells[0] in roles:
                    self.readings.append({'role': roles[cells[0]], 'reference': cells[1]})
            self.table = None
        if tag in self.stack:
            depth = len(self.stack) - self.stack[::-1].index(tag)
            for n, key, parts in self.captures[:]:
                if n >= depth:
                    self.metadata[key] = clean(''.join(parts))
                    self.captures.remove((n, key, parts))
            self.stack = self.stack[:depth - 1]


def parse(html):
    parser = FactParser()
    parser.feed(html)
    parser.close()
    return parser


def fetch(url):
    request = Request(url, headers={'User-Agent': 'Cathfetch citation audit (references only)'})
    with urlopen(request, timeout=30) as response:
        return response.geturl(), response.read().decode('utf-8')


def collect(args):
    if not re.fullmatch(r'[a-z.]+', args.source_calendar):
        raise ValueError('Invalid source calendar')
    base = 'https://universalis.com/' + args.source_calendar + '/'
    url = base + f'calendar-{args.year}.htm'
    actual, html = fetch(url)
    parsed = parse(html)
    expected = 366 if dt.date(args.year, 12, 31).timetuple().tm_yday == 366 else 365
    if len(parsed.calendar) != expected or len({r['date'] for r in parsed.calendar}) != expected:
        raise ValueError('Year calendar is incomplete or duplicated')
    if any(not r['date'].startswith(str(args.year) + '-') for r in parsed.calendar):
        raise ValueError('Server returned a different calendar year')
    facts = {'source_calendar': args.source_calendar, 'year': args.year,
             'retrieved_on': dt.datetime.now(dt.timezone.utc).date().isoformat(),
             'calendar_source': actual, 'calendar': parsed.calendar, 'mass_days': []}
    for offset in range(args.mass_days):
        date = args.mass_start + dt.timedelta(days=offset)
        url = base + date.strftime('%Y%m%d') + '/mass.htm'
        time.sleep(1)
        try:
            actual, html = fetch(url)
            page = parse(html)
            date_label = date.strftime('%A ') + str(date.day) + date.strftime(' %B %Y')
            accessible = actual == url and page.metadata.get('datename', '').startswith(date_label)
            facts['mass_days'].append({'date': date.isoformat(), 'source': url,
                'actual_url': actual, 'status': 'available' if accessible and page.readings else 'unavailable',
                'source_cycles': page.year_cycle if accessible else None,
                'observance': page.metadata.get('feastname', '') if accessible else '',
                'readings': page.readings if accessible else []})
        except (OSError, ValueError) as error:
            facts['mass_days'].append({'date': date.isoformat(), 'source': url,
                                      'status': 'unavailable', 'error': str(error), 'readings': []})
    return facts


def engine_rows(program, year, calendar):
    result = subprocess.run([str(program), str(year), calendar], check=True,
                            text=True, capture_output=True)
    return {r['date']: r for r in csv.DictReader(io.StringIO(result.stdout), delimiter='\t')}


def name_key(name):
    name = ''.join(c for c in unicodedata.normalize('NFKD', name.lower())
                   if not unicodedata.combining(c))
    name = re.sub(r'\b(saints?|sts?)\b', 'saint', name)
    name = re.sub(r'\b(solemnity|feast|the|and|of|church)\b', '', name)
    return re.sub(r'[^a-z0-9]', '', name)


def temporal_words(name):
    for number, word in enumerate(['first', 'second', 'third', 'fourth', 'fifth', 'sixth', 'seventh'], 1):
        name = re.sub(r'\b' + word + r'\b', str(number), name, flags=re.IGNORECASE)
    return name.replace('Eastertide', 'Easter')


def compare_names(source, observed):
    if name_key(source) == name_key(observed):
        return 'normalised_name_match'
    # Temporal weekday/Sunday designations have different English word order.
    def temporal_identity(name):
        name = temporal_words(name)
        number = re.search(r'week\s+(\d+)|(\d+)(?:st|nd|rd|th)?\s+(?:Sunday|week)', name, re.IGNORECASE)
        weekday = re.search(r'\b(Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday)\b', name)
        season = next((s for s in ['Ordinary Time', 'Advent', 'Lent', 'Easter'] if s in name), None)
        if number and weekday and season:
            return next(n for n in number.groups() if n), weekday[0], season
        return None
    identity = temporal_identity(source)
    if identity and identity == temporal_identity(observed):
        return 'temporal_week_match'
    return 'name_or_local_proper_review'


def citation_key(reference):
    abbreviations = {'eph': 'ephesians', 'gal': 'galatians', 'isa': 'isaiah',
                     'thess': 'thessalonians', 'lk': 'luke', 'matt': 'matthew'}
    reference = reference.lower().replace('–', '-').replace('—', '-')
    reference = re.sub(r'\b[a-z]+\b', lambda m: abbreviations.get(m[0], m[0]), reference)
    return re.sub(r'\s+', '', reference.replace('.', ','))


def reconcile(facts, calendar):
    root = Path(__file__).resolve().parents[1]
    year = facts['year']
    days = engine_rows(root / 'build/calendar_year', year, calendar)
    readings = engine_rows(root / 'build/lectionary_year', year, calendar)
    advent = [r['date'] for r in facts['calendar']
              if any('1st Sunday of Advent' in name for name in r['observances'])]
    if len(advent) != 1:
        raise ValueError('Source calendar does not identify one First Sunday of Advent')
    records = []
    for source in facts['calendar']:
        date = source['date']
        expected_year = year + (date >= advent[0])
        records.append(dict(source, cathfetch_id=days[date]['id'],
            cathfetch_observance=days[date]['observance'], cathfetch_rank=days[date]['rank'],
            name_comparison=compare_names(source['observances'][0], days[date]['observance']),
            source_liturgical_year=expected_year,
            cathfetch_liturgical_year=int(readings[date]['liturgical_year']),
            liturgical_year_match=expected_year == int(readings[date]['liturgical_year']),
            sunday_cycle=readings[date]['sunday_cycle'],
            weekday_cycle=readings[date]['weekday_cycle']))
    comparisons = []
    for source in facts['mass_days']:
        date = source['date']
        if date not in readings:
            continue
        engine = readings[date]
        cycles = {'sunday': engine['sunday_cycle'], 'weekday': engine['weekday_cycle']}
        stored = dict(re.findall(r'(\w+): (.*?); ', engine['readings']))
        first_slots = {}
        for reading in source['readings']:
            first_slots.setdefault(reading['role'], reading['reference'])
        slot_comparisons = []
        for role, reference in first_slots.items():
            result = 'stored_slot_absent'
            if role in stored:
                result = ('citation_match' if citation_key(reference) == citation_key(stored[role])
                          else 'requires_collation')
            slot_comparisons.append({'role': role, 'source': reference,
                                     'stored': stored.get(role), 'result': result})
        # Preserve every heading, including duplicate roles for feria/memorial or
        # alternative forms. Never flatten these into a falsely mandatory set.
        comparisons.append(dict(source, cathfetch_key=engine['key'], cathfetch_cycles=cycles,
            cycle_match=source.get('source_cycles') == cycles if source['status'] == 'available' else None,
            cathfetch_references=engine['readings'], first_published_set_comparison=slot_comparisons))
    return {'scope': 'Universalis public factual calendar and citation reconciliation. '
            'Birmingham contains diocesan propers; name differences are not automatically national calendar errors. '
            'Reading headings retain their order and may describe multiple options; no automatic import or '
            'current-edition verification follows. Scripture text is excluded. Source Advent dates determine '
            'the liturgical-year grouping; cycle labels come from Cathfetch and are independently compared '
            'only where a public Mass page supplies them.',
            'source_calendar': facts['source_calendar'], 'cathfetch_calendar': calendar,
            'year': year, 'retrieved_on': facts.get('retrieved_on'),
            'calendar_source': facts['calendar_source'],
            'source_advent_start': advent[0], 'dates': records, 'reading_comparisons': comparisons}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    collect_parser = commands.add_parser('fetch')
    collect_parser.add_argument('--year', type=int, required=True)
    collect_parser.add_argument('--source-calendar', default='europe.england.birmingham')
    collect_parser.add_argument('--mass-start', type=dt.date.fromisoformat, required=True)
    collect_parser.add_argument('--mass-days', type=int, default=9, choices=range(0, 10))
    collect_parser.add_argument('--output', type=Path, required=True)
    compare_parser = commands.add_parser('reconcile')
    compare_parser.add_argument('facts', type=Path)
    compare_parser.add_argument('--calendar', choices=['grc', 'europe', 'england', 'wales'], default='england')
    compare_parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = collect(args) if args.command == 'fetch' else reconcile(json.loads(args.facts.read_text()), args.calendar)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n')
    print('Saved citation/calendar facts to', args.output)


if __name__ == '__main__':
    main()
