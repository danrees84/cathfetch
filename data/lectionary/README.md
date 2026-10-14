# Mass reading references

This dataset stores citation facts only. It includes no Scripture translation, psalm response, Missal prayer or copied biography. See [the engine status](../../LECTIONARY_ENGINE_STATUS.md) for sources, coverage, verified rules and unresolved current-edition differences.

`roman_references.json` contains 804 sets provisionally collated from the Liturgy Office, official diocesan ordos and French bishops’ reference guides. **It is not a verified transcription of the 2024 England and Wales Lectionary.** Psalm and verse conventions are source-specific: older Liturgy Office and French sources generally use Septuagint numbering; Hyderabad generally uses Hebrew numbering. Mixed/dual numbering is retained where supplied. No automatic conversion of psalm numbers or verses is attempted. Unspecified Lectionary numbers remain null.

## Schema version 1

The root contains `schema_version`, `edition`, `reference_convention`, `entries` and `memorials`.

Each entry has a stable `id` and a `day_id` identifying either the calendar's observed celebration or a temporal key. Selectors are `cycle` (`any`, A/B/C, I/II), `mass`, `calendars` and optional `day_type` (`any`, `sunday`, `weekday`). There is no implicit file-order override or regional priority: overlapping defaults are invalid. To replace a general assignment for a territory, remove that territory from its applicability before adding a national record.

Forms are `day`, `vigil`, `vigil-extended`, `night`, `dawn`, `evening` and `passion`. The last describes Good Friday's liturgy, not a Mass. `alternative_only` records permitted sets that must not be selected as the default. `permitted_alternative` identifies a selected set which is itself an optional provision, rather than counting every set with an available option as an alternative selection. `use` distinguishes `proper`, `accommodated` and `weekday`; accommodated sets are suggestions, not uniquely mandatory assignments.

`readings` is an ordered array. Slots have `role`, `reference` and optional `alternatives` containing `reference` plus a human-readable `condition`. Normal roles are `first`, `psalm`, `second`, `acclamation` and `gospel`. Palm Sunday can add `procession_gospel`; Vigils can use `reading_1`…`reading_7`, `psalm_1`…`psalm_7` and `epistle`. Non-biblical acclamation texts are not reproduced; some sources also omit an acclamation citation, which remains absent rather than invented. The source qualifications distinguish such tables from complete current-edition records.

`partial: true` is reserved for genuinely proper memorial slots which replace the corresponding weekday readings. An unresolved weekday set cannot be completed from a partial record alone. `memorials` explicitly assigns each audited identity and calendar its reading policy. Absence of a policy means unverified, not automatically weekday.

Every record requires `source`, `source_note` and `verification`. `published_table` acknowledges a traceable assignment without asserting current-edition collation. `current_edition` is reserved for a record whose complete references and options have actually been checked against the current applicable edition; no supplied record currently makes that claim. That status requires nonempty `verification_sources` (HTTPS URLs) and `verification_note` describing the evidence and scope. These fields may also record partial corroboration without upgrading a provisional set. `lectionary_number` is optional/null unless independently supplied by the cited source.

Temporal keys use Sunday=0, Monday=1…Saturday=6:

| Pattern | Meaning |
| --- | --- |
| `temporal:ordinary:W:D` | Ordinary Time week and weekday |
| `temporal:advent:W:D` | Advent week and weekday before the date-specific provision |
| `temporal:advent:december:N` | 17–24 December weekday |
| `temporal:ash_wednesday` | Ash Wednesday |
| `temporal:after_ashes:D` | Thursday–Saturday after Ash Wednesday |
| `temporal:lent:W:D` | Lent weeks 1–5 |
| `temporal:holy_week:D` | Palm Sunday through Holy Saturday |
| `temporal:easter:W:D` | Easter weeks 1–7; Easter Sunday itself uses its observed ID |
| `temporal:christmas:december:N` | Christmas octave weekdays |
| `temporal:christmas:january:N` | January weekdays before Epiphany |
| `temporal:christmas:sunday2` | Second Sunday after Christmas |
| `temporal:after_epiphany:N` | 7–12 January sequence for fixed Epiphany, or Monday–Saturday sequence for Sunday Epiphany |

Do not add guessed assignments to eliminate coverage gaps. Keep pastoral alternatives conditional, preserve the source's verse conventions and record independent corrections in `source_note`. Run `make test` and regenerate the year diagnostic after changing records. The checked-in coverage TSV records the 2024–2030 sweep for this dataset.

`coverage_2024_2030.tsv` separates availability from current-edition verification and reports alternative selections separately from available options. `reference_audit.json` is a partial independent passage-comparison ledger, including explicit unavailable comparisons for every uncollated record. It is not certification. Run `python3 tools/lectionary_audit.py` to check its identities, sources and compared citations against the current assignments. See the status document for exact counts and remaining access blockers.

`universalis_reconciliation_2026.json` records the user-requested full public Birmingham date calendar and the nine publicly accessible Mass-page citation sets collected on 14 October 2026. It preserves diocesan differences and repeated reading roles without importing them as national defaults. Universalis redirects Mass dates outside its public window to an access notice; the audit tool records unavailable pages and does not bypass this restriction. See `tools/universalis_audit.py --help` and the status document for commands and exact verification scope.
