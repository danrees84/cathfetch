#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

"""Build an offline JSON index, or reproduce the bundled XML from public USFX.

Uses Python's standard library at build time; Cathfetch's lookup is native C++.
The complete 1899 text is public domain. Strong numbers and layout are discarded,
not verse text. Input hashes pin the exact source used by this project.
"""
import argparse
import hashlib
import json
from pathlib import Path
import xml.etree.ElementTree as ET
import zipfile


def digest(data):
    return hashlib.sha256(data).hexdigest()


def convert(archive, metadata):
    with zipfile.ZipFile(archive) as source:
        data = source.read('engDRA_usfx.xml')
    if digest(data) != metadata['source_sha256']:
        raise ValueError('USFX source hash differs from the recorded edition')
    root = ET.fromstring(data)
    result = ET.Element('bible')
    for book in root.findall('book'):
        identifier = book.attrib['id']
        target = ET.SubElement(result, 'b', id=identifier, n=metadata['books'][identifier]['name'])
        chapter = None
        verse = None
        parts = []

        def visit(node):
            nonlocal chapter, verse, parts
            if node.tag == 'c':
                chapter = ET.SubElement(target, 'c', n=node.attrib['id'])
            elif node.tag == 'v':
                if verse is not None:
                    raise ValueError('Unclosed source verse')
                verse, parts = node.attrib['id'], []
            elif node.tag == 've':
                if verse is None or chapter is None:
                    raise ValueError('Unexpected source verse end')
                ET.SubElement(chapter, 'v', n=verse).text = ' '.join(''.join(parts).split())
                verse, parts = None, []
            if verse is not None and node.text:
                parts.append(node.text)
            for child in node:
                visit(child)
                if verse is not None and child.tail:
                    parts.append(child.tail)

        visit(book)
        if verse is not None:
            raise ValueError('Unclosed final verse')
    data = ET.tostring(result, encoding='utf-8', xml_declaration=True)
    if digest(data) != metadata['xml_sha256']:
        raise ValueError('Converted XML differs from the bundled source')
    return data


def index(xml, metadata):
    data = xml.read_bytes()
    if digest(data) != metadata['xml_sha256']:
        raise ValueError('Bible XML hash differs from recorded provenance')
    result = dict(metadata, books={})
    result['versification'] = json.loads((xml.parent / 'en_dra_versification.json').read_text())
    for book in ET.fromstring(data):
        identifier = book.attrib['id']
        if identifier in result['books']:
            raise ValueError('Duplicate book')
        entry = dict(metadata['books'][identifier], chapters={})
        for chapter in book:
            number = chapter.attrib['n']
            if number in entry['chapters']:
                raise ValueError('Duplicate chapter')
            verses = {}
            for verse in chapter:
                n = verse.attrib['n']
                if n in verses or not n.isdigit() or int(n) < 1 or not verse.text:
                    raise ValueError('Invalid or duplicate verse')
                verses[n] = verse.text
            entry['chapters'][number] = verses
        result['books'][identifier] = entry
    if result['books'].keys() != metadata['books'].keys() or len(result['books']) != 73:
        raise ValueError('Expected all 73 Catholic books')
    return json.dumps(result, ensure_ascii=False, separators=(',', ':')).encode('utf-8') + b'\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=['index', 'convert'])
    parser.add_argument('input', type=Path)
    parser.add_argument('metadata', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    metadata = json.loads(args.metadata.read_text())
    operation = {'index': index, 'convert': convert}[args.mode]
    data = operation(args.input, metadata)
    args.output.write_bytes(data)


if __name__ == '__main__':
    main()
