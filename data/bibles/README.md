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

## Numbering limitations — material, not exceptional

A complete Bible text does not establish a complete mapping from Cathfetch’s mixed-source lectionary citations. **Full-text lookup currently has partial coverage.** The regression sweep of 2,655 stored default non-acclamation citations resolves 1,684; 971 are explicitly unavailable. These are citation-record counts, not unique passages or calendar-day coverage.

- Most psalm references remain unmapped. The four checked Vulgate psalms (1, 2, 97 and 116) and a specific Hebrew Psalm 100 verse map are supported. Chapter subtraction alone is insufficient: Douay Psalm 99 verse 1 is a title, and verse 2 combines the text commonly numbered as Hebrew Psalm 100 verses 1–2. This mapping displays whole Douay verses with an explicit note.
- `psalm_numbering` on a reading slot records `lxx`, `hebrew` or `unspecified`. Provenance establishes the newly added French/Hyderabad conventions; Christmas Day’s and the Immaculate Conception’s older Liturgy Office Psalm 97 are explicitly identified (the latter table prints the response “Sing a new song to the Lord”). Other legacy tables are not silently assigned a convention. Checked dual-number forms such as `98(97)` are recognised, but do not bypass unverified verse alignment.
- Some Old Testament books and passages have substantial numbering differences, particularly Sirach, Tobit, Esther and Daniel. Conservative metadata guards keep uncollated passages unavailable. Specific verified Daniel 3 mappings distinguish the inserted prayer/canticle from modern references to the king seeing the three men in the furnace. They do not apply one blanket offset to the chapter.
- Additional numeric guards identify differing Hebrew/English/Latin verse addresses using the [STEPBible TVTMS table](https://github.com/STEPBible/STEPBible-Data/tree/master/Versification), created by STEPBible/Tyndale House Cambridge, **CC BY 4.0** ([licence](https://creativecommons.org/licenses/by/4.0/)). The precise source URL and hash are in `guard_source`. The adaptation expands simple numeric differences into conservative unavailable-verse guards; it does not claim to implement TVTMS’s complete conditional reversification algorithm. Complex cases retain book-level guards. Partial/merged verse differences are not inferred as exact text cuts.

To reproduce these numeric guards from the pinned TVTMS text:

```sh
python3 tools/bible_data.py guards /path/to/TVTMS.txt data/bibles/en_dra_metadata.json /tmp/en_dra_metadata.json
cmp data/bibles/en_dra_metadata.json /tmp/en_dra_metadata.json
```

Further mappings need explicit provenance and literal passage tests. Do not remove guards merely to increase coverage. Text lookup does not certify the underlying lectionary assignment, and the Douay–Rheims wording is not the ESV–CE/Abbey text used in the current England and Wales Lectionary. Existing assignment-verification qualifications remain in the engine and diagnostic data.
