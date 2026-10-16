# Douay–Rheims Bible text

`en_dra.xml` contains all 73 books of the Douay–Rheims American Edition of 1899,
from [eBible.org](https://ebible.org/engDRA/). The provider identifies it as
[public domain](https://ebible.org/engDRA/copyright.htm). It has 35,811 numbered
verses and is about 5.1 MB.

The downloaded USFX was converted to a simpler XML structure:
`<bible><b id="..."><c n="..."><v n="...">`. Verse numbers and wording were kept;
layout and Strong-number annotations were removed, and whitespace was
normalised. The electronic text has not been independently proofread.

## Source and build

The source archive is [engDRA_usfx.zip](https://ebible.org/Scriptures/engDRA_usfx.zip),
retrieved on 14 October 2026. `en_dra_metadata.json` records hashes for its
`engDRA_usfx.xml` member and the converted XML, along with book names, aliases
and numbering information. To reproduce the XML:

```sh
python3 tools/bible_data.py convert /path/to/engDRA_usfx.zip data/bibles/en_dra_metadata.json /tmp/en_dra.xml
cmp data/bibles/en_dra.xml /tmp/en_dra.xml
```

`make all` creates `build/en_dra.json`, checking the hash, 73-book coverage and
unique chapter/verse addresses. Indexing uses Python's standard library. The
running app uses C++20 and nlohmann/json to read the index.

`make install` includes the index and source files. Data is found relative to
the executable, or through `CATHFETCH_DATA_DIR`. Other clients can pass an index
filename to `loadBible(filename)`. See [Development](../../docs/DEVELOPMENT.md)
for the directory layout.

## Displaying readings

```sh
./build/cathfetch --date=25122026 --full-verses
./build/cathfetch --date=25122026 --full-verses --cross
./build/cathfetch --date=25122026 --mass=night --full-verses
```

`--full-verses` uses the same reading selection as `--readings`, then prints the
passages with chapter and verse numbers. Shorter forms remain listed as
alternatives. Gospel acclamations are omitted from both displays.

References may use book aliases, commas or dots for selections, semicolons for
chapter changes, cross-chapter ranges and subverse letters. For a reference
such as `21b`, the app shows the whole numbered verse and says so. Overlapping
selections such as `3a, 3b–4` print each verse once. A passage that cannot be
mapped safely produces `Text unavailable: ...`.

Book IDs use USFM codes and modern book identities. For example, `1 Kings`
means `1KI`; it is not interpreted as the older Douay title for `1 Samuel`.
Ambiguous names such as bare `Kings` are rejected.

## Verse numbering

The mappings are adapted from the [STEPBible TVTMS table](https://github.com/STEPBible/STEPBible-Data/tree/master/Versification),
created by STEPBible/Tyndale House Cambridge and licensed under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). The source URL and hash
are in `versification_source` in the metadata.
`en_dra_versification.json` records the adapted correspondences, source lines,
chapter ends, absent verses and rejected or conditional rows.

The reference profile uses Hebrew numbering for the Old Testament and the main
Greek numbering for the deuterocanonical books and New Testament. Psalms need
an explicit `lxx` or `hebrew` convention, or a recognised dual number. Mappings
cover Psalm titles, splits and combinations, including older labels 9B, 113A
and 113B. They map whole verses, not exact subverse cuts.

Most ordinary addresses keep their numbers unless the book requires an explicit
mapping, as Tobit does. This implements the correspondences needed here, not
the full conditional TVTMS conversion algorithm. Source-backed exceptions are
recorded in `exception_sources`:

- Douay Psalm 99:1 is a title; verse 2 combines Hebrew Psalm 100:1–2.
- Some older Liturgy Office Psalm 115/147 references use LXX chapter numbers
  with unsplit Hebrew verse numbers. Overrides apply only to that convention.
- Daniel's prayers and canticles retain their Catholic numbers. Narrative
  passages map around the additions; Daniel 13–14 and Esther 11–16 retain
  their native numbering.
- Combined verses in 1 Thessalonians 4 and 2 Thessalonians 2 shift later numbers.

To reproduce the mapping file from the exact recorded TVTMS source:

```sh
python3 tools/bible_versification.py /path/to/TVTMS.txt data/bibles/en_dra_metadata.json data/bibles/en_dra.xml /tmp/en_dra_versification.json
cmp data/bibles/en_dra_versification.json /tmp/en_dra_versification.json
```

## Available text and known gaps

Of 2,665 stored primary non-acclamation citations, 2,661 return text. All 119
stored alternatives return text. Two passages cite a verse absent from this
edition, Genesis 49:32 and Sirach 3:19; the app shows the available verses with
an omission notice. The Isaiah 12 chapter-only alternative follows a source
that does not specify exact verses.

### Tobit

The [passage audit](tobit_correspondence_audit.json) compares six difficult
selections with the bundled text and the USCCB modern Catholic Bible. It records
references and findings, without copying modern Scripture text.

Two whole-passage mappings are available: Tobit 1:3;2:1–8 uses native
1:2–3;2:1–8, and 12:1,5–15,20 uses the same native range despite internal verse
differences. Neither mapping permits individual verses elsewhere in Tobit to
be matched by number alone.

Four complete passages remain unavailable because the textual traditions differ:

- Tobit 6:10–11;7:1,9–14;8:4–9
- Tobit 11:5–17
- Tobit 13:2,6–8
- Tobit 13:2–8

The app gives a specific explanation for each. A simple numbering shift cannot
supply all the material in the appointed passages.

Tests check every stored citation and selected daily readings for 2024–2030 in
all four calendars. These text-availability checks are separate from verifying
the Lectionary assignments. See [Lectionary status](../../LECTIONARY_ENGINE_STATUS.md)
and the [citation corrections](../lectionary/scripture_corrections.json).
