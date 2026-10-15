# Mass reading references

`roman_references.json` contains 807 reading sets, with references and sources.
It stores citations rather than Scripture text. The sources include Liturgy
Office tables, official diocesan ordos and bishops' reference guides. Complete
collation against the current England and Wales Lectionary is still outstanding.

## What v1 selects

V1 uses universal Sunday and weekday readings, feasts and solemnities. Memorials
use the regular weekday readings, including those with proper passages in the
full Lectionary. National saints' propers are omitted.

Universal celebrations keep their local dates: the calendar engine resolves a
General Roman reading day using the selected calendar's scheduling rules.
The displayed celebration still comes from the full regional calendar.
The Lectionary engine receives the reading day and selects complete sets that
apply to `grc`.

National sets, partial memorial sets and the old `memorials` array remain in the
file for future work. V1 does not use those policies or merge proper readings
into a weekday set. The year reports label this policy `universal_v1`.

## Record format

Schema version 1 has `schema_version`, `edition`, `reference_convention`,
`entries` and `memorials` at the root.

| Field | Meaning |
| --- | --- |
| `id` | Unique reading-set ID. |
| `day_id` | Celebration ID or temporal key. |
| `cycle` | `any`, A/B/C or I/II. |
| `mass` | `day`, `vigil`, `vigil-extended`, `night`, `dawn`, `evening` or `passion`. |
| `calendars` | Calendars to which the record applies. |
| `day_type` | `any`, `sunday` or `weekday`; optional. |
| `alternative_only` | A permitted alternative that is not selected by default. |
| `permitted_alternative` | Marks a selected set that is itself an optional provision. |
| `use` | `proper`, `accommodated` or `weekday`. Accommodated sets are suggested choices. |
| `partial` | An archived set containing only proper memorial slots; excluded by v1. |
| `readings` | Ordered reading slots. |
| `source`, `source_note`, `verification` | Required source and verification details. |
| `lectionary_number` | Number from a source, or null when unknown. |

`passion` identifies Good Friday's liturgy, not a Mass. If two records could
both be the default for the same form, loading fails; file order does not break
the tie.

Each reading slot has a `role`, `reference` and optional `alternatives`. An
alternative has its own `reference` and a `condition`, such as a shorter form.
Usual roles are `first`, `psalm`, `second`, `acclamation` and `gospel`. Palm
Sunday can add `procession_gospel`. Vigils can use `reading_1`–`reading_7`,
`psalm_1`–`psalm_7` and `epistle`.

`psalm_numbering` is `unspecified`, `lxx` or `hebrew`. Keep the source's
convention: the older Liturgy Office and French tables generally use LXX
numbering, while Hyderabad generally uses Hebrew numbering. Mixed and dual
numbers are retained. The separate Bible lookup handles the conversion.
Some sources omit Gospel acclamations; these remain absent.

All supplied records have `verification=published_table`. Use `current_edition`
only after checking the complete references and options against the applicable
edition. That status requires `verification_sources` with HTTPS URLs and a
`verification_note`. Those fields can also document partial checks without
changing the status.

## Temporal keys

Weekdays are numbered Sunday=0, Monday=1, through Saturday=6.

| Pattern | Day |
| --- | --- |
| `temporal:ordinary:W:D` | Ordinary Time week and weekday |
| `temporal:advent:W:D` | Advent week and weekday before the date-specific readings |
| `temporal:advent:december:N` | Weekdays, 17–24 December |
| `temporal:ash_wednesday` | Ash Wednesday |
| `temporal:after_ashes:D` | Thursday–Saturday after Ash Wednesday |
| `temporal:lent:W:D` | Lent weeks 1–5 |
| `temporal:holy_week:D` | Palm Sunday through Holy Saturday |
| `temporal:easter:W:D` | Easter weeks 1–7; Easter Sunday uses its celebration ID |
| `temporal:christmas:december:N` | Christmas octave weekdays |
| `temporal:christmas:january:N` | January weekdays before Epiphany |
| `temporal:christmas:sunday2` | Second Sunday after Christmas |
| `temporal:after_epiphany:N` | 7–12 January for fixed Epiphany, or Monday–Saturday after Sunday Epiphany |

## Changing the data

Use sourced assignments, keep conditional alternatives conditional and record
corrections in `source_note`. Run `make test` after changes and regenerate the
year diagnostic. The summary collection uses these same IDs; see its
[data guide](../summaries/README.md).

The audit files record different checks:

| File | Contents |
| --- | --- |
| [coverage_2024_2030.tsv](coverage_2024_2030.tsv) | Date coverage, verification and alternative selections. |
| [reference_audit.json](reference_audit.json) | Partial independent passage comparisons, with unmatched records listed too. |
| [current_ordo_audit.json](current_ordo_audit.json) | Comparisons with the Hallam 2025–26 ordo. |
| [citation_precision_audit.json](citation_precision_audit.json) | 41 corrected citation limits in 39 sets, plus three retained differences. |
| [summary_reference_audit.json](summary_reference_audit.json) | Five range corrections made while preparing summaries. |
| [universalis_reconciliation_2026.json](universalis_reconciliation_2026.json) | The Birmingham date calendar and nine accessible Mass-page citation sets collected on 14 October 2026. |

```sh
python3 tools/lectionary_audit.py
python3 tools/current_lectionary_audit.py check data/lectionary/current_ordo_audit.json
./build/lectionary_year 2026 england > /tmp/readings-2026.tsv
```

These commands check that the evidence still matches the data. They do not
certify a whole reading set against the current edition. The Universalis report
also retains diocesan differences and repeated reading roles; those are not
imported as national defaults. Its tool respects the site's public date window.
See [Lectionary status](../../LECTIONARY_ENGINE_STATUS.md) for the results and
remaining work.
