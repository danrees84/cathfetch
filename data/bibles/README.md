# Offline Douay–Rheims text

`en_dra.xml` contains all 73 books of the Douay–Rheims American Edition of 1899, obtained from [eBible.org](https://ebible.org/engDRA/). Its [copyright statement](https://ebible.org/engDRA/copyright.htm) identifies the text as public domain. The XML has 35,811 numbered source verses and occupies about 5.1 MB. The downloaded USFX was normalised into the simple `<bible><b id="..."><c n="..."><v n="...">` structure. Source verse numbers and text are retained; Strong-number annotations and layout are removed and whitespace is normalised. This is a pinned electronic transcription, not an independently proofread edition.

The user-supplied [GitHub `en_dra.xml`](https://github.com/thiagobodruk/bible/blob/c5179e5f736a44237c0746858c5be33824f29db3/xml/en_dra.xml) was inspected first. It contains only 66 books, lacks the deuterocanonical books, and omits Daniel’s Greek additions. It is therefore **not** the bundled text. The complete copy from eBible.org is the same translation/edition and avoids silently losing Catholic readings.

## Provenance and reproducibility

The source is [engDRA_usfx.zip](https://ebible.org/Scriptures/engDRA_usfx.zip), retrieved 14 October 2026. `en_dra_metadata.json` records the SHA-256 of its `engDRA_usfx.xml` member and the resulting bundled XML, as well as the translation, book aliases and conservative verse-number mapping policies. The exact recorded archive member is required to reproduce this snapshot; a changed upstream download is rejected.

```sh
python3 tools/bible_data.py convert /path/to/engDRA_usfx.zip data/bibles/en_dra_metadata.json /tmp/en_dra.xml
cmp data/bibles/en_dra.xml /tmp/en_dra.xml
```

The book IDs use USFM codes and modern English book identities. For example, modern `1 Kings` maps to `1KI`, rather than treating it as the historical Douay title for `1 Samuel`. Ambiguous bare names such as `Kings` are rejected. Book aliases are structured metadata, not scattered resolver conditionals.

`make all` builds `build/en_dra.json` from the XML and metadata, validating the recorded hash, the 73-book coverage and unique chapter/verse addresses. Python 3’s standard library is used at build time; no Python packages or XML library are required. **Runtime lookup is C++20**, using the project’s existing nlohmann JSON dependency. It performs no network calls and does not invoke Python. `loadBible(filename)` accepts an index location for reuse by another client. Like the existing calendar data, the CLI’s default paths are relative to the project directory; distribute the generated index with the application if packaging it elsewhere.

## Lookup and display policy

```sh
./build/cathfetch --date=25122026 --full-verses
./build/cathfetch --date=25122026 --full-verses --cross
./build/cathfetch --date=25122026 --mass=night --full-verses
```

`--full-verses` implies reading selection. It uses the same resolved day, calendar, Mass form and default passages as `--readings`. The latter keeps its compact reference-only output. Full mode labels the translation once, prints each selected passage with chapter/verse numbers, and wraps through the existing terminal renderer. Short forms remain reference alternatives; they are not printed as duplicate full passages. Acclamations are omitted, consistently with compact mode.

The parser supports book aliases, comma/dot-separated selections, semicolon chapter changes, cross-chapter ranges and subverse suffixes. **Subverse cuts are not reconstructed.** A reference such as `21b` displays the entire numbered verse and explicitly says so. Repeated selections such as `3a, 3b–4` coalesce the overlapping whole verse. A malformed, missing or unverified mapping produces `Text unavailable: ...` for that passage, atomically; no truncated or unrelated passage is substituted.

## Verse correspondence and measured coverage

All **2,655 stored primary non-acclamation citations** now return text. Two passages contain a cited verse absent from this translation: Genesis 49:32 and Sirach 3:19. Their available verses are displayed with an explicit omission notice; the missing text is never invented. These counts describe stored citations, not unique passages or independently verified lectionary assignments.

The mappings use the main Hebrew, Greek and Latin traditions in the [STEPBible TVTMS table](https://github.com/STEPBible/STEPBible-Data/tree/master/Versification), created by STEPBible/Tyndale House Cambridge, **CC BY 4.0** ([licence](https://creativecommons.org/licenses/by/4.0/)). The precise source URL and SHA-256 are recorded in `versification_source` in the metadata. `en_dra_versification.json` records whole-verse correspondences, source line evidence, chapter ends, known absences and rejected/conditional rows. Unlisted ordinary addresses retain their numbers. Unrepresentable known correspondences fail explicitly. This is a targeted adaptation, not TVTMS's complete conditional reversification algorithm or a general conversion service for every biblical edition.

Source Hebrew numbering applies to the Old Testament, and main Greek numbering to the deuterocanonical books and New Testament. Psalms require an explicit `lxx` or `hebrew` convention, or recognised dual numbering. Mapping handles titles, split and combined psalms, and whole-verse unions where divisions differ. It never reconstructs an exact subverse cut. Lettered psalms 9B, 113A and 113B in the older tables are recognised.

Edition/source exceptions remain structured metadata, with provenance in `exception_sources`:

- Douay Psalm 99 verse 1 is a title, while verse 2 combines the text commonly numbered as Hebrew Psalm 100 verses 1–2.
- Some older Liturgy Office Psalm 115/147 citations use LXX chapter labels with unsplit Hebrew verse numbers. Exact passage overrides are scoped to that convention; they do not alter Hebrew citations or French native LXX verse citations.
- Traditional Daniel 3 canticle/prayer citations retain their Catholic numbers. Modern furnace narrative citations map around the Greek additions. Daniel 13–14 and appended Esther 11–16 retain their native Catholic chapter numbers.
- The bundled Douay text combines verses in 1 Thessalonians 4 and 2 Thessalonians 2; subsequent verses shift accordingly.

To reproduce the derived table from the exact pinned TVTMS source:

```sh
python3 tools/bible_versification.py /path/to/TVTMS.txt data/bibles/en_dra_metadata.json data/bibles/en_dra.xml /tmp/en_dra_versification.json
cmp data/bibles/en_dra_versification.json /tmp/en_dra_versification.json
```

The scripture regression test checks all 2,655 primary and 116 alternative non-acclamation citations, literal passage identities and all selected daily passages across 2024–2030 in all four calendars. CLI tests retain wrapping checks at 120/100/80/60/40 columns. Corrections to malformed reference-table citations are recorded in [the correction ledger](../lectionary/scripture_corrections.json); the independent comparison ledger remains conservatively qualified.

The chapter-only Isaiah 12 alternative retains its source qualification that exact verses were not specified; a whole-chapter lookup does not certify that the whole chapter is appointed.

Text lookup does not certify the underlying lectionary assignment. Douay–Rheims wording is not the ESV–CE/Abbey text used in the current England and Wales Lectionary. Current-edition verification and the documented missing national assignments remain separate limitations. The pinned electronic Bible transcription is not independently proofread. Unstored or ambiguous references can still return an explicit unavailable result.
