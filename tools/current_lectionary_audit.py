#!/usr/bin/env python3
"""Compare citation facts from the public Hallam 2025–26 ordo.

Extract: current_lectionary_audit.py extract ORDO_LAYOUT.txt OUTPUT.json
Compare: current_lectionary_audit.py compare FACTS.json OUTPUT.json
Check: current_lectionary_audit.py check REPORT.json

Input text must come from pdftotext -layout. Only complete, uniquely labelled
reading blocks are retained, with page/line locators. Acclamations, alternatives
and date selection are not certified. No Scripture or liturgical prose is saved.
"""
import argparse
import collections
import hashlib
from functools import cache
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'https://hallam-diocese.com/wp-content/uploads/sites/9/2023/12/HALLAM-ORDO-2025-26.pdf'
SCOPE = ('Partial comparison with a current-edition diocesan ordo, not certification of the printed Lectionary. '
         'Blocks are matched by first-reading and Gospel whole-verse anchors, not dates. '
         'Differences retain exact source citations; no subverses, psalm numbering or options are silently equated. '
         'Clipped/inconsistent ordo entries require review. Acclamations and full alternative sets remain unverified.')


@cache
def metadata():
    return json.loads((ROOT / 'data/bibles/en_dra_metadata.json').read_text())


def patterns():
    books = metadata()
    names = sorted({a for b in books['books'].values() for a in b['aliases']}, key=len, reverse=True)
    book = '(?:' + '|'.join(re.escape(a).replace(r'\ ', r'\s*') for a in names) + ')'
    address = r'[0-9]+(?:\([0-9]+\))?\s*:\s*[0-9]+[a-g]*(?:(?:\s*[-–—,:;.]+\s*)[0-9]+[a-g]*)*'
    return book, re.compile('(' + book + r'\s*' + address + ')', re.I), re.compile('(' + address + ')')


def extract(path):
    book, reference, psalm = patterns()
    rows = []
    for page, content in enumerate(path.read_text().split('\f'), 1):
        lines = content.splitlines()
        for line_number, line in enumerate(lines):
            if not re.search(r'\bI\s+' + book + r'\s*[0-9]+:', line, re.I):
                continue
            block = line
            for following in lines[line_number + 1:line_number + 4]:
                if re.search(r'\b(?:Office:|Mass:|I\s+' + book + r'\s*\d+:)', following):
                    break
                if re.search(r'\bG\s+' + book + r'\s*\d+:', block):
                    break
                block += ' ' + following.strip()
            markers = list(re.finditer(r'\b(II|I|Ps|G)\s+', block))
            readings = []
            for i, marker in enumerate(markers):
                body = block[marker.end():markers[i+1].start() if i+1 < len(markers) else len(block)]
                match = (psalm if marker[1] == 'Ps' else reference).match(body)
                if match:
                    readings.append({'role': {'I': 'first', 'II': 'second', 'Ps': 'psalm', 'G': 'gospel'}[marker[1]],
                                     'reference': ('Ps ' if marker[1] == 'Ps' else '') + match[1].strip()})
            roles = {r['role'] for r in readings}
            if len(roles) == len(readings) and {'first', 'psalm', 'gospel'} <= roles:
                rows.append({'source': SOURCE + f'#page={page}', 'line': line_number + 1, 'readings': readings})
    return {'scope': SCOPE, 'source': SOURCE, 'layout_sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'blocks': rows}


def canonical(reference, numbering='unspecified', anchor=False):
    books = metadata()
    text = reference.strip()
    # Find the book by longest alias; retain numerical verse groups literally.
    matches = []
    for bid, book in books['books'].items():
        for alias in book['aliases']:
            match = re.match(re.escape(alias).replace(r'\ ', r'\s*') + r'\s*(?=[0-9])', text, re.I)
            if match:
                matches.append((match.end(), bid))
    if not matches:
        return None
    end, bid = max(matches)
    text = re.split(r'\s+[rR]\.', text[end:])[0]
    text = text.translate(str.maketrans({'–': '-', '—': '-', '−': '-', '.': ',', ';': ','}))
    dual = re.match(r'(\d+)\s*\((\d+)\)', text)
    if bid == 'PSA' and dual:
        if numbering not in ['hebrew', 'lxx']:
            return None
        text = dual[1 if numbering == 'hebrew' else 2] + text[dual.end():]
    text = re.sub(r'\s+', '', text).rstrip(',')
    if anchor:
        text = re.sub(r'(?<=[0-9])[a-g]+', '', text)
    return bid + ':' + text


def compare(facts, assignments):
    # Whole-verse anchors find candidate evidence; exact comparison below still
    # flags partial endpoints. Choosing the best block never changes a citation.
    anchors = collections.defaultdict(list)
    for block in facts['blocks']:
        slots = {r['role']: r['reference'] for r in block['readings']}
        anchors[(canonical(slots['first'], anchor=True), canonical(slots['gospel'], anchor=True))].append(block)
    rows = []
    for entry in assignments['entries']:
        slots = {r['role']: r for r in entry['readings']}
        key = (canonical(slots.get('first', {}).get('reference', ''), anchor=True),
               canonical(slots.get('gospel', {}).get('reference', ''), anchor=True))
        candidates = anchors.get(key, []) if all(key) and entry['mass'] == 'day' else []
        strong_match = bool(candidates)
        if not candidates and all(key) and entry['mass'] == 'day':
            # A matching first reading with a different Gospel is a review
            # lead, never confirmation. Preserve that distinction in the report.
            candidates = [block for (first, gospel), blocks in anchors.items()
                          if first == key[0] and gospel and gospel.split(':')[0] == key[1].split(':')[0]
                          for block in blocks]
        observations = []
        for block in candidates:
            other = {r['role']: r['reference'] for r in block['readings']}
            comparisons = []
            for role, slot in slots.items():
                independent = other.get(role)
                numbering = slot.get('psalm_numbering', 'unspecified')
                left = canonical(slot['reference'], numbering)
                right = canonical(independent, numbering) if independent else None
                result = ('not_supplied' if independent is None else
                          'literal_citation_match' if left is not None and left == right else 'requires_collation')
                comparisons.append({'role': role, 'stored': slot['reference'], 'independent': independent, 'result': result})
            observations.append({'source': block['source'], 'line': block['line'], 'comparisons': comparisons})
        evidence = max(observations, key=lambda o: sum(c['result'] == 'literal_citation_match'
                                                      for c in o['comparisons']), default=None)
        rows.append({'id': entry['id'], 'assignment_source': entry['source'],
                     'status': ('partial_current_ordo_comparison' if strong_match else 'single_anchor_review') if evidence else 'no_matching_block',
                     'current_edition_verified': False, 'evidence': evidence})
    counts = collections.Counter(row['status'] for row in rows)
    return {'scope': SCOPE, 'source': facts['source'], 'layout_sha256': facts['layout_sha256'],
            'summary': dict(counts), 'entries': rows}


def check(report, assignments):
    expected = {e['id']: e for e in assignments['entries']}
    rows = {r['id']: r for r in report['entries']}
    if len(rows) != len(report['entries']) or rows.keys() != expected.keys():
        raise ValueError('Current-ordo audit identities are stale or duplicated')
    for identifier, row in rows.items():
        entry = expected[identifier]
        if row['assignment_source'] != entry['source'] or row['current_edition_verified']:
            raise ValueError('Unsupported verification claim or stale source: ' + identifier)
        if row['evidence']:
            stored = {c['role']: c['stored'] for c in row['evidence']['comparisons']}
            if stored != {r['role']: r['reference'] for r in entry['readings']}:
                raise ValueError('Current-ordo audit citations are stale: ' + identifier)
    counts = dict(collections.Counter(row['status'] for row in rows.values()))
    if counts != report['summary']:
        raise ValueError('Current-ordo audit summary is stale')
    print(counts)


def check_precision(ledger, assignments):
    """Keep manual collation evidence attached to the exact live citations."""
    entries = {entry['id']: entry for entry in assignments['entries']}
    seen = set()
    for row in ledger['corrections']:
        key = (row['id'], row['role'])
        if key in seen:
            raise ValueError('Duplicate precision correction: ' + str(key))
        seen.add(key)
        entry = entries[row['id']]
        slot = next(r for r in entry['readings'] if r['role'] == row['role'])
        if slot['reference'] != row['reference'] or row['previous'] == row['reference']:
            raise ValueError('Precision correction is stale: ' + str(key))
        sources = row['sources']
        if len(sources) < 2 or len({source['url'] for source in sources}) != len(sources):
            raise ValueError('Independent precision evidence required: ' + str(key))
        if not any(source['url'].startswith(SOURCE + '#page=') for source in sources):
            raise ValueError('Current national ordo evidence required: ' + str(key))
        for source in sources:
            if (canonical(row['reference']) is None or
                    source['url'] not in entry['verification_sources'] or
                    canonical(source['reference']) != canonical(row['reference'])):
                raise ValueError('Precision evidence differs from the citation: ' + str(key))
    for row in ledger['retained_differences']:
        entry = entries[row['id']]
        slot = next(r for r in entry['readings'] if r['role'] == row['role'])
        if slot['reference'] != row['reference']:
            raise ValueError('Retained source difference needs re-collation: ' + row['id'])
    print(f"{len(seen)} collated citation limits; "
          f"{len(ledger['retained_differences'])} retained source differences; no full-set certification")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=['extract', 'compare', 'check'])
    parser.add_argument('input', type=Path)
    parser.add_argument('output', nargs='?', type=Path)
    args = parser.parse_args()
    assignments = json.loads((ROOT / 'data/lectionary/roman_references.json').read_text())
    if args.operation == 'check':
        check(json.loads(args.input.read_text()), assignments)
        precision = ROOT / 'data/lectionary/citation_precision_audit.json'
        check_precision(json.loads(precision.read_text()), assignments)
        return
    if args.output is None:
        parser.error('extract and compare require an output file')
    result = extract(args.input) if args.operation == 'extract' else compare(json.loads(args.input.read_text()), assignments)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n')


if __name__ == '__main__':
    main()
