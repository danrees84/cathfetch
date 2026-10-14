# Cathfetch lectionary engine status

As of 14 October 2026, Cathfetch has **807 reference sets and 214 explicit memorial policies**. Ordinary Time weeks 1–34 are populated in both weekday cycles. The calendar and lectionary remain separate; `--readings` remains opt-in. **Complete verification against the England and Wales Lectionary introduced in Advent 2024 is blocked by the absence of an accessible complete current-edition citation index.** No supplied set claims that verification. Reference availability must not be read as certification.

## Offline full-verse display

`--full-verses` now implies reading selection and displays the selected passages using a locally bundled, complete 73-book Douay–Rheims (1899) text. `--readings` retains the requested compact template. Calendar selection and Mass forms remain unchanged. The scripture lookup is a separate native C++ component; an offline build-time index is generated from the XML using Python’s standard library. No network access or Python runtime is used by Cathfetch.

The originally suggested GitHub XML has only 66 books, so the bundled normalised XML uses the complete public-domain copy of the same edition from eBible.org. See [Bible provenance, commands and limitations](data/bibles/README.md). Partial-verse references display whole verses with an explicit notice; shorter forms remain reference alternatives.

The mapping implementation uses source-backed whole-verse correspondences, including psalm splits/merges, titles and differing verse divisions. **The independent follow-up audit supersedes the earlier claim that all stored passages were safely mapped.** Tobit has substantial recension differences; an in-range address is not proof of correspondence. Unlisted Tobit addresses now require an explicit map. Of **2,665 primary non-acclamation citations, 2,659 return text and six explicitly report an unverified correspondence**. The affected Tobit readings/canticles are in Ordinary Time weeks 9 and 25, cycle I. Two other passages contain a cited verse absent from Douay–Rheims (Genesis 49:32 and Sirach 3:19); available text is shown with an omission notice. All 119 stored alternative non-acclamation citations are checked separately.

Regression coverage includes literal text/address checks, every stored citation and selected daily passages for all 10,228 date/calendar combinations in 2024–2030. An explicit unavailable correspondence is tested as such, never counted as verified text. Final validation passes: `make -j4 all test`, both audit freshness checks, and byte-for-byte current-ordo report reproduction from the cached official PDF. The scripture suite performs 35,471 checks; its 10,228-date sweep finds 31 date/calendar selections with guarded Tobit text correspondences. All 119 alternative references resolve. All twelve C++ suites, three CLI suites and 15 Python audit/import tests pass, including terminal widths 120/100/80/60/40.

## Independent audit update — 14 October 2026

Added three national default sets after consulting Welsh diocesan directions and national/parish sources:

- St David in England: Philippians, Psalm 1 and Matthew, with the published Luke alternative. The truncated old psalm citation is repaired using St Gregory's Catholic parish headings for 1 March 2025.
- St David in Wales: the diocesan ordo's suggested Isaiah 61:1–3,10–11 before the national Philippians provision, retaining three readings for the solemnity. Transfers retain this identity and set.
- All Saints of Wales: the diocesan ordo directs use of the All Saints readings. Under Lectionary Introduction 84(c), the feast default has two readings; Revelation is selected and 1 John retained as an alternative first reading.

These are explicit accommodated selections, **not uniquely mandatory Common sets or fully verified 2024-edition assignments**. [National audit evidence and remaining gap](data/lectionary/national_audit.json) records exact source scope and access limitations. The indexed diocesan PDF supplied the directions; its direct download currently returns HTTP 403. The Six Welsh Martyrs remain missing: the diocesan ordo points to a proper set without giving its citations. A third-party transcription is insufficient authority to substitute an invented Common set.

The new [current diocesan ordo audit](data/lectionary/current_ordo_audit.json) checks all 807 records against 328 complete citation blocks extracted from the official Hallam 2025–26 ordo. It finds 289 partial two-anchor comparisons, 13 single-anchor review leads and 505 records without a matching block. Across the recorded comparisons, 707 slots match literally after book/punctuation normalisation, 252 require collation and 209 are unsupplied. Matching first-reading/Gospel anchors locates evidence; it does not verify dates, omitted subverses, options or a complete current-edition set. The report retains exact source citations and PDF page/line locators. No Scripture or liturgical prose is stored.

Confirmed corrections include Holy Tuesday John 13:21–33,36–38; Ordinary Time week 12 Tuesday Matthew 7:6,12–14 in both cycles; week 20 Wednesday II Matthew 20:1–16; week 8 Friday Mark 11:11–25 in both cycles; the missing Matthew 10:5a on Advent week 1 Saturday; and Psalm 118:111 in week 19 Tuesday II. Their previous values and source evidence remain in [the correction ledger](data/lectionary/scripture_corrections.json). Other differences remain review items, since the ordo itself contains clipped/inconsistent entries.

The year diagnostic exposes the partial current-ordo audit scope through `verification_scope` and its evidence through `verification_sources`. Every supplied set still has `verification=published_table`; none is promoted to `current_edition`. The compact terminal display is unchanged.

```sh
# Reproduce citation-only extraction from an authorised copy of the public PDF:
pdftotext -layout HALLAM-ORDO-2025-26.pdf /tmp/hallam-layout.txt
python3 tools/current_lectionary_audit.py extract /tmp/hallam-layout.txt /tmp/hallam-facts.json
python3 tools/current_lectionary_audit.py compare /tmp/hallam-facts.json /tmp/current-ordo-audit.json
python3 tools/current_lectionary_audit.py check data/lectionary/current_ordo_audit.json
```

## Earlier data milestone

Added 204 sets to the original 600: 168 Ordinary Time weekday sets for weeks 21–34, six partial mandatory memorial provisions and 30 general, European or national feast sets. Added ten memorial policies. Calendar calculations, precedence and terminal layout are unchanged.

The new weekday sets contain the first reading, responsorial psalm/canticle and Gospel in I/II. The sources do not supply their Gospel acclamations; those remain absent, explicitly qualified in each entry. Hyderabad and French bishops’ public reference guides provide these assignments. Their citation conventions differ: Hyderabad generally uses Hebrew psalm numbers, while the French guides use Septuagint numbers and sometimes different biblical verse numbering. These conventions are retained and disclosed, never silently converted into England and Wales numbering.

The Hallam 2025–26 ordo explicitly identifies the first reading as proper for Timothy and Titus and Barnabas, and the Gospel as proper for the Immaculate Heart, John the Baptist’s Passion, Our Lady of Sorrows and the Guardian Angels. These six partial sets replace only those weekday slots. Martha, Mary and Lazarus’s existing Gospel provision is also corroborated. Other Marian memorials retain weekday readings under explicit policies; optional Common selections are not misrepresented as mandatory proper readings.

New feast coverage includes the apostles, Lawrence, Marian feasts, European patrons, George in England, Patrick in England/Wales, Augustine of Canterbury and Gregory the Great in England, English Martyrs, Walsingham, Newman, John Fisher and Thomas More, and Thomas Becket. The same identity may remain a memorial outside the territory where it is a feast. Its national feast set does not replace that territory’s weekday default. George’s selected Common set is explicitly accommodated, not uniquely mandatory.

## Universalis reconciliation — 14 October 2026

At the user’s request, [Universalis’s public Birmingham calendar](https://universalis.com/europe.england.birmingham/calendar-2026.htm) has been reconciled with Cathfetch’s England calendar. The [stored report](data/lectionary/universalis_reconciliation_2026.json) contains all 365 dates and citation headings from the nine accessible Mass pages, 13–21 October 2026. No Scripture text is stored. The scraper uses the Python standard library, makes sequential requests one second apart, checks returned dates/URLs and never works around the date limit.

The source independently places the First Sunday of Advent on **29 November 2026**. All 365 dates agree with Cathfetch’s liturgical-year grouping: 332 dates in the year ending 2026 and 33 in the year ending 2027. All nine available Mass pages independently agree with A/II. Cathfetch supplies B/I after Advent; those future Mass-page labels were not accessible for independent Universalis verification. There are 317 normalised-name or temporal-week matches and 48 name/local-proper review items; those review items are not automatically errors.

Birmingham includes diocesan celebrations absent from the national selection, including Wulstan, Chad, the cathedral dedication, Dominic Barberi, Edmund Campion and Birinus. It also supplies an anniversary of dedication for consecrated churches. These have not been imported into England’s national calendar. Naming variants and optional-memorial choices also require interpretation; the report preserves them instead of forcing identity matches.

All nine first readings and Gospels agree after expanding abbreviations. The report separately identifies psalm/verse differences, missing acclamation slots and memorial reading alternatives. **Do not merge repeated roles:** Universalis can publish both feria and memorial sets on one page. The parser retains all citation headings in order; the comparison of the first published set does not declare the later set mandatory or certify all options.

One confirmed correction was made: the second reading for Ordinary Time Sunday 29 A ends at **1 Thessalonians 1:5b**, corroborated both by Universalis’s 18 October Mass page and the official Hallam ordo. The record and independent audit ledger retain that evidence. Other differences remain review items: for example, Universalis omits Psalm 100 verse 4 on 19 October while Hallam’s compressed range reads 1–5. This is not sufficient evidence to silently impose a global correction. Current-edition verification remains pending.

Universalis [explicitly limits public reading pages](https://universalis.com/n-otherdates.htm) to yesterday, today and the week ahead. The requested 19 October page is available, but 29 November redirects to that notice. The full public yearly calendar is accessible; **a complete year of Mass references cannot currently be scraped from these public dated pages**. No paid access was purchased or bypassed. The earlier Universalis collection did not resolve the national reading gaps; the subsequent diocesan-source work above supplies explicit defaults for St David and All Saints of Wales.

```sh
# Snapshot collected on 14 October 2026; use the current public window for a later fetch.
python3 tools/universalis_audit.py fetch --year 2026 --mass-start 2026-10-13 --output /tmp/universalis-facts.json
python3 tools/universalis_audit.py reconcile /tmp/universalis-facts.json --calendar england --output /tmp/universalis-report.json
python3 tests/universalis_audit_test.py
```

The year diagnostic now includes the explicit ending `liturgical_year` alongside A/B/C and I/II. Six Python tests cover citation-only extraction, retained options, access notices, date/name comparisons, subverse differences and the complete source-year reconciliation against the live engine. These tests are included in `make test`.

## Coverage measured, not inferred

The [coverage report](data/lectionary/coverage_2024_2030.tsv) contains 28 complete civil-year sweeps: 2024–2030 in all four calendars, totalling 10,228 date/calendar pairs. There are 10,194 available provisional sets, six missing assignments, 28 Holy Saturdays without a daytime Mass and no unresolved calendar days. There are 101 selections of a permitted alternative/default accommodated set, and 5,300 days with alternatives available. These last two metrics overlap with availability; they are not extra days.

For 2026:

| Calendar | Days with references | Current-edition verified | Provisional | Missing | No daytime Mass | Selected permitted alternative |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| GRC | 364 | 0 | 364 | 0 | 1 | 2 |
| Europe | 364 | 0 | 364 | 0 | 1 | 2 |
| England | 364 | 0 | 364 | 0 | 1 | 3 |
| Wales | 364 | 0 | 364 | 0 | 1 | 3 |

The remaining observed-day gap is **Six Welsh Martyrs and their Companions**, Wales, 25 October when not impeded. It accounts for all six missing assignments in the 2024–2030 sweep. Holy Saturday's absent daytime Mass is a separate status; an explicit Easter Vigil request is supported. Reference availability is independent of the six guarded Tobit full-text correspondences.

## Independent audit: exact scope

The [audit ledger](data/lectionary/reference_audit.json) accounts for all 807 records, including all original 600. **It is an audit triage ledger, not a completed independent textual audit.** The French bishops’ 2023–2025 guides provide partial independent passage comparisons for 619 records; 188 lack a usable comparison. Of the original 600, 453 have partial comparisons and 147 remain without a usable independent comparison.

Across the selected comparisons, 1,173 slots match after book-abbreviation and punctuation normalisation, 798 require collation, and 423 are absent or ambiguous in the comparison source. These numbers are neither verified assignments nor confirmed errors. Differences include source verse/psalm numbering, optional forms, and transcription defects in the comparison sources themselves. No verse numbering is converted by the comparison. No reference is changed merely to improve the match count.

The ledger records an example date, independent URL/page, stored citation, comparison citation and result for each comparable slot. Dates are selected through Cathfetch; this is **not** independent proof of the date-selection rules. Only daytime sets are compared, and a first-reading or Gospel match is required to avoid treating a foreign national schedule or a different Mass form as the same provision. Unmatched records, acclamations, alternatives and precise subverses remain openly pending. The ledger cannot certify the 2024 England and Wales edition or all options from daily French ordos.

`python3 tools/lectionary_audit.py` checks identity/source/citation freshness and prints the audit totals. It uses stored citation facts only, needs no network access or third-party Python packages, and cannot confer verification. Re-collate evidence before changing an audited citation; the freshness check deliberately fails on stale comparisons.

The year diagnostic now separates `status=available` from `verification=published_table|current_edition|none`. It also exposes selected permitted alternatives, available option counts, provenance and verification scope. Its summary counts dates with references, verified/provisional dates, alternatives, missing readings, no daytime Mass and unresolved calendar days. `current_edition` requires explicit verification sources and a scope note at load time. No current dataset entry uses that status. At the user’s request, the terminal uses a compact `Readings:` heading followed by semicolon-separated reading references and inline alternatives. It omits acclamations, cycle/Mass-form labels, verification notices and alternative-set counts. Verification and Mass-form metadata remain available through the engine and year diagnostic; missing-reading and no-daytime-Mass messages remain explicit.

## Sources and access limits

- [Liturgy Office 2024–25 index](https://www.liturgyoffice.org.uk/Calendar/2025/index.shtml), supplied by the user: confirms Sunday C/weekday I and links monthly calendars to the existing shared reference pages. Those links do not establish that every shared citation was collated for the new edition. In particular, [OT5](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT5.shtml), [OT6](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT6.shtml) and [OT7](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT7.shtml) have navigation but no reading tables.
- [Liturgy Office current Lectionary information](https://www.liturgyoffice.org.uk/Resources/Lectionary/) and [FAQ](https://www.liturgyoffice.org.uk/Resources/Lectionary/LM-FAQ.shtml): ESV–CE and Abbey Psalms and Canticles from Advent 2024; continuing Sunday selections and additional national/saints provisions do not supply a complete citation index.
- [Hallam 2025–26 ordo](https://hallam-diocese.com/wp-content/uploads/sites/9/2023/12/HALLAM-ORDO-2025-26.pdf): current national feast and mandatory memorial-slot evidence, with page locators in the data. It omits acclamations/options and contains some clipped or inconsistent text. Diocesan propers are excluded; suspicious weekday paragraphs were not imported as facts.
- [Hyderabad 2024–25](https://www.archdioceseofhyderabad.org/downloads/ordo/Ordo-2024-2025-information.pdf) and [2025–26](https://www.archdioceseofhyderabad.org/downloads/ordo/Ordo-2025-26-mass-readings.pdf): public official weekday reference guides. Some direct downloads failed; the accessible indexed PDF text was used. These are not the England and Wales edition.
- French bishops’ public guides: [2023 A](https://liturgie.catholique.fr/wp-content/uploads/sites/11/2022/12/30-ORDO-2022_2023-Annee-A.pdf), [2024 B](https://liturgie.catholique.fr/wp-content/uploads/sites/11/2022/12/31-ORDO-2023-2024-Annee-bissextile-B.doc.pdf), [2025 C](https://catechese.catholique.fr/wp-content/uploads/sites/15/2024/02/32-ORDO-2024-2025-ANNE%CC%81E-C.doc.pdf): citation facts for missing general feasts, some weekdays and independent comparison evidence. French national dates and Mass forms are not imported indiscriminately.
- [Lectionary Introduction](https://www.liturgyoffice.org.uk/Resources/Rites/LM-Introduction.pdf), especially 66–70, 82–84, 93 and 96: cycles, annual weekday Gospel, proper versus accommodated memorial readings, feasts and seasonal arrangements. The composed week 26 Monday I set records both independent source locations and the annual-Gospel rule.
- Existing [Sunday](https://www.liturgyoffice.org.uk/Calendar/Sunday/AdventSunday.shtml), [weekday](https://www.liturgyoffice.org.uk/Calendar/Weekday/Advent.shtml), [solemnity](https://www.liturgyoffice.org.uk/Calendar/Sunday/OT2Solemnities.shtml) and [feasts of the Lord](https://www.liturgyoffice.org.uk/Calendar/Sunday/Feasts.shtml) tables remain the sources of the original 600. Earlier independently checked corrections are retained in their source notes.
- User-supplied [Lectionary.com full year](https://lectionary.com/catholic/full-year): inspected, but no current England/Wales authority or complete daily coverage established. No data was imported from this third-party table. The displayed St Joseph date of 20 March 2026 also requires correction/clarification before using it as a date authority.

The remaining blockers are a complete authoritative current-edition citation index/current volumes, the Welsh Martyrs proper, and independently verified Tobit correspondences across textual recensions. The public sources checked cannot establish complete exact-verse, acclamation and option verification, nor the remaining Welsh Martyrs proper. No copyrighted Scripture, Missal texts, biographies or publisher database has been imported. No full ecclesiastical or historical accuracy is claimed.

## API and existing policies

`readingCycles(date)` returns the liturgical year by its ending civil year, Sunday A/B/C and weekday I/II. Both labels change at Advent; 28–29 November 2026 changes A/II to B/I. Seasonal weekday sets are annual except explicit conditional provisions. Ordinary Time uses I/II.

`loadLectionary()` validates identities, selectors, calendars, citations, forms, alternatives, provenance and ambiguous defaults atomically. `resolveMassReadings(LiturgicalDay, data, calendarName, mass)` consumes the resolved calendar identity; transfers and national schedules retain that identity’s readings. It does not resolve precedence or move celebrations. Partial proper memorial sets require an available weekday set, retaining both sources and the weaker verification status. There is one resolver.

The default is daytime Mass. Christmas has Vigil/Night/Dawn/Day; Easter has Vigil/Day; Pentecost has simple and extended Vigil/Day. Holy Thursday defaults to evening and Good Friday to its Passion liturgy, explicitly labelled. This date-only engine is not an anticipated-Mass/first-Vespers scheduler. Easter Vigil retains all seven Old Testament reading/psalm pairs; pastoral omissions are not automated. All Souls’ suggested set is accommodated. Optional memorials retain the calendar’s temporal default; the CLI does not choose optional saints. Common selections and all possible ritual/votive forms remain incomplete. Lectionary citations remain display strings; the separate Scripture lookup parses supported addresses with explicit correspondence policies.

## Build and validation

```sh
make -j4 all test
python3 tools/lectionary_audit.py
./build/cathfetch --date=25102026 --readings --cross
./build/cathfetch --date=28112026 --readings
./build/cathfetch --date=02032026 --calendar=wales --readings
./build/lectionary_year 2026 england > /tmp/lectionary-2026.tsv
```

The application is rebuilt in `build/cathfetch`. All twelve C++ test executables, all three CLI suites and the six Universalis Python tests pass. The lectionary test performs **14,612 date/calendar pairs** (2008, 2022, 2024–2030 and 2038, all four calendars), including reversed assignment order: 14,563 available, nine explicit gaps and 40 Holy Saturdays. The calendar regression performs 450,238 checks across 64,284 date/calendar pairs, each resolved in both candidate orders. Reading and calendar CLI tests include 120, 100, 80, 60 and 40 columns, with/without the cross and redirected output. The 168 late Ordinary Time selectors, independent literal reference cases, proper-slot/weekday combinations, national versus general ranks and verification-evidence validation have dedicated checks.

These are implementation, coverage and selected reference checks, **not independent verification of every reading over complete liturgical years**. Completing that final audit requires the authoritative material described above.
