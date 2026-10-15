# Cathfetch Scripture Summary status

15 October 2026. The offline feature is implemented; the corpus is an editorial draft awaiting human review and full current-edition reference verification.

## Implemented behaviour

```sh
./build/cathfetch --summary
./build/cathfetch --date=30112025 --calendar=england --summary
./build/cathfetch --cross --readings --summary
./build/cathfetch --date=25122026 --mass=night --summary
```

`--summary` is opt-in and independent of `--readings`; existing `--mass` and `--full-verses` implications are preserved. Output shows the exact Scripture quotation, native reference and translation, originating reading and passage, Mass form and original 40–70-word commentary. Review and reference-verification statuses remain in the data and editorial collection; terminal summaries omit the review-status footer. Missing summaries or unavailable reading forms produce a diagnostic, never generated substitutes. Holy Saturday has no daytime summary; the explicit Vigil form uses its assigned readings.

The implementation is a small separate C++ model/loader/validator and identity lookup. It consumes the existing `LectionaryDay`, preserving calendar transfers, cycles and local universal-feast dates. No new calendar or reading-selection mechanism was added. The shared Scripture parser exposes conservative whole-verse eligibility; full-passage display retains its prior behaviour. The terminal reuses existing Unicode/ANSI-aware wrapping and cross layout. Redirected CLI output now omits ANSI colours; terminal output retains them.

## Measured coverage

| Measure | Result |
| --- | ---: |
| Stored reading sets | 807 |
| Complete universal-scope records | 780 |
| Retained national/partial proper records | 27 |
| Sets with stored draft summaries | 807 |
| Distinct biblical explanations | 442 |
| Quote matches against pinned electronic source | 807 |
| Independently proofread quotations | 0 |
| Fully current-edition-certified reading sets | 0 |
| Human-reviewed summaries | 0 |
| Stored passage alternatives compatible with summary | 116 / 119 |
| Incompatible alternatives selected by v1 | 0 |

Selection categories are 188 first readings, 212 Psalms/canticles, 87 second readings/epistles and 320 Gospels. The generated distribution separates seasons and cycles, including explicit Sunday records. These counts describe editorial selections, not quotas or the frequency of dates in a civil year.

All 10,228 date/calendar pairs in 2024–2030 resolve under v1: 10,200 have a summary; 28 Holy Saturdays have no daytime Mass; none has an unexplained missing summary. The same dates include 76 selected permitted/default alternative sets. These measure availability, not factual certification. Named Mass forms, transfers, cycle transitions and leap dates are also tested separately.

See the generated [editorial collection and master index](docs/scripture-summaries/README.md), [coverage](docs/scripture-summaries/coverage.json), [distribution](docs/scripture-summaries/distribution.json), [alternative compatibility](docs/scripture-summaries/alternative-compatibility.json) and [year coverage](docs/scripture-summaries/year-coverage.tsv). They are reproduced with `make summary-review`; `python3 tools/summary_review.py check` rejects stale exports.

## Sources and corrections

The existing complete 73-book Bible is the [Douay–Rheims American Edition of 1899](https://ebible.org/engDRA/copyright.htm), whose provider identifies it as public domain. Exact quotations come from its pinned local transcription; no copyrighted ESV–CE/Abbey text or borrowed commentary was added. Source hashes and [versification provenance](data/bibles/README.md) remain available. The model keeps translation identity and source provenance explicit.

Assignments retain their source-specific references, primarily the [Liturgy Office tables](https://www.liturgyoffice.org.uk/Calendar/Sunday/AdventSunday.shtml), with independent bishops' ordo evidence. The [Liturgy Office FAQ](https://www.liturgyoffice.org.uk/Resources/Lectionary/LM-FAQ.shtml) confirms the Advent 2024 change of translation and unchanged Sunday selections; it does not provide a complete current citation index.

Summary collation identified four apparent source-table errors in five records. John 12:33–50 became 12:44–50; Mark 9:14–25 became 9:14–29 in both weekday cycles; Matthew 20:1–6 became 20:1–16; Numbers 20:1–3 became 20:1–13. The [correction ledger](data/lectionary/summary_reference_audit.json) records French bishops' guide and, where available, USCCB/Hallam corroboration. Full reading-set certification was not inferred. Existing comparison ledgers remain fresh.

## Remaining limitations and review

- All commentary remains an AI draft. Named human review and any ecclesiastical assessment must be real future actions, not statuses conferred by tests.
- Current England and Wales collation remains incomplete. Existing public ordos/tables do not verify every cut, option, Psalm numbering and acclamation in the printed 2024 edition.
- The v1 policy deliberately omits memorial and national propers. Archived summaries do not enable them. Three archived partial-proper alternatives lack a compatible selection; they are outside v1.
- The four previously documented Tobit complete-passage correspondences remain unavailable in full-verse mode. Summaries use a verifiable verse from another appointed reading, rather than inventing a correspondence.
- Eligibility checks are deliberately conservative about partial/merged verses. Source matching does not independently proofread the electronic Bible or prove every textual tradition equivalent.
- This is coverage of the stored daily-reference catalogue, not every diocesan, historical, ritual or votive provision. Runtime data paths remain relative to the project directory, following the existing application convention.

The initial 27-commentary sample and subsequent editorial process are described in the [dataset guide](data/summaries/README.md). No additional v1 policy decision is required; the user's universal-reading policy has been retained.

## Validation and checkpoints

The full C++20 application and diagnostics build in `build/`. `make --jobserver-style=pipe -j4 all test` passes with no compiler warnings: 13 C++ suites, four CLI suites and 17 Python unit tests, followed by the reproducible summary-data/export check. The GNU Make pipe jobserver option avoids temporary FIFO collisions in this sandbox; ordinary `make -j4 all test` is also supported.

- Calendar regression: 450,238 checks over 64,284 dates, including both candidate orders.
- Lectionary regression: 44,371 checks; 14,612 date/calendar pairs, with 14,572 available assignments and 40 Holy Saturdays without daytime Mass.
- Scripture regression: 35,492 checks; 35,811 source verses; 2,661 stored primary citations available and the four documented Tobit selections explicitly unavailable.
- Summary regression: 11,054 checks; all 807 stored sets, all 10,228 date/calendar pairs in the stated range, invalid/duplicate records, altered quotations, partial and merged boundaries, Advent/leap-year boundaries, transfers and Mass forms. Applicable alternative passages retain the selected verse. The 2038 Corpus Christi test correctly uses Sunday Year A and the existing provisional St John transfer.
- Rendering/CLI: 120/100/80/60/40 columns with and without the cross, independent and combined flags, full verses, plain redirected output and real pseudo-terminal colour/width checks.
- Both existing reference-audit freshness tools pass. The independent comparison remains partial. Similarity detection reports no candidates above its threshold; that is not editorial approval.

Checkpoint `458cf8f` contains the tested implementation, complete stored draft corpus, correction evidence, generated collection and regression tests. The documentation follow-up records these results and links the existing Lectionary status to this report. No unrelated user changes were included.
