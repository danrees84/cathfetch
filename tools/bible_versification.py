#!/usr/bin/env python3
"""Derive whole-verse correspondences from the pinned CC BY STEPBible table.

Usage: bible_versification.py TVTMS.txt metadata.json en_dra.xml output.json
Only the main traditional columns are used, never NETS/Brenton/Greek2 variants.
Unrepresentable target verses are explicit unavailable entries, not identity
fallbacks. Edition-specific corrections live separately in the metadata.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

SCHEMES = {'hebrew': 'Hebrew', 'greek': 'Greek', 'english': 'English'}
ROW_TYPES = {'OneToOne', 'SubdividedVerse', 'MergedPrevVerse',
             'MergedNextVerse', 'TextMayBeMissing'}


def derive(source, metadata, xml):
    if hashlib.sha256(source.read_bytes()).hexdigest() != metadata['versification_source']['sha256']:
        raise ValueError('Versification source hash differs from provenance')
    aliases = {re.sub('[^a-z0-9]', '', a.lower()): bid
               for bid, book in metadata['books'].items() for a in book['aliases']}
    aliases.update({'psa': 'PSA', 'ecc': 'ECC', 'sng': 'SNG', 'jol': 'JOL',
                    'ezk': 'EZK', 'nam': 'NAM', 'php': 'PHP', 'mrk': 'MRK',
                    'jhn': 'JHN', 'act': 'ACT', '1ki': '1KI', '2ki': '2KI',
                    '1sa': '1SA', '2sa': '2SA', '1ch': '1CH', '2ch': '2CH'})
    native = {b.get('id'): {int(c.get('n')): max(int(v.get('n')) for v in c)
                           for c in b} for b in ET.parse(xml).getroot()}

    def addresses(value, scheme="latin"):
        value = value.strip().replace('–', '-')
        if value.startswith('Absent'):
            recipient = re.search(r'\[=([^]]+)\]', value)
            return addresses(recipient[1], scheme) if recipient else None
        value = re.sub(r'\s*\[.*', '', value)
        if ',' in value or ';' in value:
            selected = []
            book = chapter = None
            for part in re.split(r'[,;]\s*', value):
                if '.' not in part:
                    if book is None:
                        return None
                    part = book + '.' + (part if ':' in part else chapter + ':' + part)
                heading = re.match(r'([^.]*)\.([0-9]+):', part)
                if not heading:
                    return None
                book, chapter = heading.groups()
                parsed = addresses(part, scheme)
                if parsed is None:
                    return None
                selected.extend(parsed)
            return selected
        # .1/.2 and a/b are parts of a whole verse in this table.
        value = re.sub(r'(\d)\.[12](?=$|[,-])', r'\1', value)
        match = re.fullmatch(r'([1-3]?[A-Za-z0-9]+)\.(\d+):(\d+)(?:[!a-z]+)?'
                             r'(?:-(?:(\d+):)?(\d+)(?:[!a-z]+)?)?', value)
        if not match or match[1].lower() not in aliases:
            return None
        book, chapter, verse = aliases[match[1].lower()], int(match[2]), int(match[3])
        last_chapter = int(match[4]) if match[4] else chapter
        last_verse = int(match[5]) if match[5] else verse
        if last_chapter < chapter or last_chapter - chapter > 1:
            return None
        if chapter != last_chapter:
            length = ends.get(scheme, {}).get(f'{book}:{chapter}', native.get(book, {}).get(chapter, 0))
            if verse > length or last_verse > 500:
                return None
            return ([(book, chapter, n) for n in range(verse, length + 1)] +
                    [(book, last_chapter, n) for n in range(1, last_verse + 1)])
        if last_verse < verse or last_verse - verse > 500:
            return None
        return [(book, chapter, n) for n in range(verse, last_verse + 1)]

    def column(headers, label):
        return next((i for i, name in enumerate(headers)
                     if name.rstrip('*') == label or
                     (label == 'English' and name == 'English KJV')), None)

    sections = []
    section = None
    for line_number, line in enumerate(source.read_text(encoding='utf-8-sig').splitlines(), 1):
        if '2) EXPANDED VERSION' in line:
            break
        cells = line.split('\t')
        if cells[0].startswith('$'):
            section = {'headers': cells, 'rows': [], 'tests': []}
            sections.append(section)
        elif section is not None:
            if cells[0] == 'BIBLES':
                section['headers'] = cells
            elif cells[0].startswith('TEST:') or any(x in cells[0] for x in SCHEMES.values()):
                section['tests'].append(cells)
            elif cells[0] in ROW_TYPES:
                section['rows'].append((line_number, cells))

    maps = collections.defaultdict(dict)
    evidence = collections.defaultdict(set)
    ends = collections.defaultdict(dict)
    rejected = []
    omitted = set()
    conflicts = set()
    for section in sections:
        headers = section['headers']
        latin = column(headers, 'Latin')
        if latin is None:
            continue
        valid = True
        for test in section['tests']:
            if test[0].startswith('TEST:') and latin < len(test):
                for b, c, v in re.findall(r'([1-3]?[A-Za-z0-9]+)\.(\d+):(\d+)=Last', test[latin]):
                    bid = aliases.get(b.lower())
                    if bid in native and native[bid].get(int(c)) != int(v):
                        valid = False
        for scheme, label in SCHEMES.items():
            index = column(headers, label)
            for test in section['tests']:
                values = []
                if test[0].startswith('TEST:') and index is not None and index < len(test):
                    values = [test[index]]
                elif label in [v.strip().rstrip('*') for v in test[0].split('+')]:
                    values = test[1:]
                for value in values:
                    for b, c, v in re.findall(r'([1-3]?[A-Za-z0-9]+)\.(\d+):(\d+)=Last', value):
                        if b.lower() in aliases:
                            ends[scheme][aliases[b.lower()] + ':' + c] = int(v)
            if index is None:
                continue
            local = collections.defaultdict(set)
            for line_number, row in section['rows']:
                if max(index, latin) >= len(row):
                    continue
                # A title already merged in both traditions adds no new
                # verse correspondence. Some table title annotations retain
                # the unswitched psalm chapter; never union those into the body.
                if row[index].startswith('Absent') and row[latin].startswith('Absent'):
                    continue
                src, dst = addresses(row[index], scheme), addresses(row[latin])
                if not src:
                    # Retain rejected rows for review. References outside the
                    # Catholic canon (e.g. Psalm 151) are not lookup addresses.
                    if row[index].strip():
                        rejected.append([line_number, scheme, row[index], row[latin]])
                    continue
                if valid and row[latin].strip() == 'Absent':
                    omitted.update(scheme + ':' + ':'.join(map(str, address)) for address in src)
                if not valid or not dst or any(b not in native or c not in native[b] or
                                                v > native[b][c] for b, c, v in dst):
                    dst = []
                if len(src) == len(dst):
                    pairs = zip(src, [[v] for v in dst])
                elif len(src) == 1:
                    pairs = [(src[0], dst)]
                elif len(dst) <= 1:
                    pairs = [(v, dst) for v in src]
                else:
                    pairs = [(v, []) for v in src]
                for address, targets in pairs:
                    key = ':'.join(map(str, address))
                    local[key].update(targets)
                    evidence[scheme + ':' + key].add(line_number)
            for key, targets in local.items():
                converted = [list(v) for v in sorted(targets)]
                if key in maps[scheme] and maps[scheme][key] != converted:
                    conflicts.add(scheme + ':' + key)
                maps[scheme][key] = [] if scheme + ':' + key in conflicts else converted
    return {'schema_version': 1, 'source': metadata['versification_source'],
            'profiles': 'Main Hebrew/Greek/English to Latin; whole verses; Latin Last tests checked against bundled edition.',
            'maps': {s: dict(sorted(m.items())) for s, m in maps.items()},
            'chapter_ends': dict(ends), 'rejected_rows': rejected, 'omitted_verses': sorted(omitted),
            'conflicting_profiles': sorted(conflicts),
            'evidence': {k: sorted(v) for k, v in sorted(evidence.items())}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ['source', 'metadata', 'xml', 'output']:
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    result = derive(args.source, json.loads(args.metadata.read_text()), args.xml)
    args.output.write_text(json.dumps(result, ensure_ascii=False, separators=(',', ':')) + '\n')


if __name__ == '__main__':
    main()
