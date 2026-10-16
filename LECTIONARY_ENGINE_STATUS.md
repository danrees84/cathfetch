# Cathfetch Lectionary engine status

Current state: 16 October 2026. The offline engine resolves every tested v1
reading day, but complete verification against the current England and Wales
Lectionary remains outstanding. Reference availability, exact Bible text matching
and human editorial review are separate measures.

## Selection policy and behaviour

V1 selects **universal readings with local dates for universal celebrations**.
All memorials retain regular weekday readings, including those with mandatory
proper passages in the full Lectionary. National saints' propers are omitted.
Sundays, universal feasts and solemnities, seasonal weekdays and the supported
universal Mass forms remain available. This deliberately simplified policy does
not reproduce every proper-reading provision of an observed Mass.

The terminal displays the full regional celebration. The calendar engine
separately resolves the General Roman reading day using the selected calendar's
scheduling rules, preserving local Epiphany, Ascension, Corpus Christi and
universal holyday dates. The Lectionary resolver consumes that identity; it does
not load calendars or resolve precedence itself.

Sunday cycles A/B/C and weekday cycles I/II follow the liturgical year beginning
at Advent. Transfers retain the observed celebration's reading identity. Within-
reading alternatives and supported alternative reading sets remain explicit.
Unresolved universal assignments produce diagnostics rather than fabricated
passages or an unrelated weekday substitute.

`--readings` prints compact references. `--full-verses` implies reading selection
and prints the bundled Douay–Rheims text. `--summary` independently displays a
stored quotation and commentary using the same reading identity. `--mass` implies
reading display and selects an available form. The default is normally daytime;
Christmas has Vigil/Night/Dawn/Day, Easter has Vigil/Day and Pentecost has simple
and extended Vigil/Day. Holy Thursday defaults to evening. Good Friday's
`passion` form identifies its liturgy, not a Mass. Holy Saturday has no daytime
Mass. This is not an anticipated-Mass or first-Vespers scheduler.

## Current measured coverage

| Measure | Result |
| --- | ---: |
| Stored reading sets | 807 |
| Complete universal-scope sets | 780 |
| Retained national/partial proper sets, inactive in v1 | 27 |
| Historical memorial-policy records, inactive in v1 | 214 |
| Fully current-edition-certified sets | 0 |
| Dates tested in 2024–2030 across four calendars | 10,228 |
| Dates with v1 references and summaries | 10,200 |
| Holy Saturdays without daytime Mass | 28 |
| Missing v1 assignments in that date range | 0 |
| Stored primary non-acclamation citations | 2,665 |
| Primary citations with available offline text | 2,661 |
| Explicitly unavailable complete Tobit correspondences | 4 |
| Alternative non-acclamation citations checked | 119 |

Ordinary Time weeks 1–34 are populated in both weekday cycles. The wider
Lectionary regression covers 14,612 date/calendar pairs in 2008, 2022,
2024–2030 and 2038: 14,572 have assignments and 40 are Holy Saturdays without
daytime Mass. These counts describe tested availability, not independent
verification of every reading or future liturgical decree.

## Sources, audits and remaining work

Each reading record retains source references and source-specific Psalm and
verse conventions. All 807 sets retain `published_table` verification; none is
promoted merely because automated checks pass. Public Liturgy Office tables,
official diocesan ordos and bishops' reference guides provide the existing data.
They do not collectively establish every cut, option, acclamation and numbering
detail of the England and Wales Lectionary introduced in Advent 2024.

- [Reference audit](data/lectionary/reference_audit.json): partial independent
  passage comparisons for 619 records; 188 lack a usable comparison. The latest
  ledger has 1,205 matching slots, 766 requiring collation and 423 unsupplied or
  ambiguous slots. It is an audit triage ledger, not whole-set certification.
- [Current Hallam ordo audit](data/lectionary/current_ordo_audit.json): 289 partial
  two-anchor comparisons, 13 single-anchor leads and 505 sets without a matching
  block. Its 748 literal slot matches, 211 differences and 209 unsupplied slots
  remain partial evidence.
- [Citation precision corrections](data/lectionary/citation_precision_audit.json)
  and [summary reference corrections](data/lectionary/summary_reference_audit.json)
  retain the evidence and limits of targeted corrections.
- [National audit](data/lectionary/national_audit.json): archived national
  provisions remain outside v1. Authoritative Welsh Martyrs proper citations
  remain a deferred data task, not a missing v1 weekday assignment.
- [Universalis reconciliation](data/lectionary/universalis_reconciliation_2026.json)
  records the scope of the earlier England/Birmingham comparison; diocesan
  propers and optional choices are not silently imported into a national layer.

Some Gospel acclamations and Lectionary numbers are absent from the public
reference sources. Ritual/votive forms, all diocesan propers and full Common
selection are outside current coverage. Earlier proper-slot and memorial-policy
behaviour is not active in v1.

The complete 73-book Bible source is a pinned electronic Douay–Rheims (1899)
transcription, not the current ESV–CE/Abbey Lectionary text. Partial-verse cuts
show whole verses with a notice. Four Tobit selections remain unavailable because
of textual-tradition differences; two other citations include an absent verse
and show available text with an omission notice. See the
[Bible guide](data/bibles/README.md) for exact provisions and provenance.
All summary commentary awaits human editorial review; see
[Scripture Summary status](SCRIPTURE_SUMMARY_STATUS.md).

## Build, validation and code

```sh
make -j4 all test
./build/cathfetch --date=25102026 --readings --cross
./build/lectionary_year 2026 england > /tmp/lectionary-2026.tsv
./build/summary_year 2026 england > /tmp/summaries-2026.tsv
python3 tools/lectionary_audit.py
python3 tools/current_lectionary_audit.py check data/lectionary/current_ordo_audit.json
```

The latest full run passes nine C++ suites, five CLI/integration suites and
17 Python unit tests, plus the generated-summary freshness check. The Lectionary
suite performs 44,371 checks, Scripture 35,492 and summaries 11,060. CLI checks
cover 120/100/80/60/40 columns, cross layouts, redirected output, staged
installation, external working directories and data overrides. Calendar and
Lectionary engines remain independent of terminal rendering and Qt.

Data discovery and installation work independently of the working directory;
see [Development](docs/DEVELOPMENT.md). JSON stores factual assignments and
provenance. C++ selects readings and looks up Scripture. Python tools prepare
and audit data; normal application execution is offline C++.

For the earlier implementation milestones, source discussions and historical
measurements, see the [archived audit notes](docs/history/LECTIONARY_AUDIT_HISTORY.md).
Those earlier counts and proper-aware policies do not override this current
status or the current v1 resolver.
