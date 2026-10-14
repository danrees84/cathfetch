# Cathfetch calendar engine status

As of 14 October 2026, the engine implements the contemporary General Roman Calendar and the supplied European, English and Welsh overlays. Every date in 2024–2030 resolves to an observed day in all four calendars. The engine also diagnoses an unresolved occurrence outside that primary range, described below. This is a contemporary calendar snapshot, not a historical or diocesan ordo.

## Resolution and coverage

`resolveLiturgicalDay(date, celebrations, calendarName)` validates the input, constructs the fixed observance schedule, gathers movable candidates, and selects against one temporal candidate. Rank and precedence remain distinct. General and proper classifications cover all thirteen levels, including proper feasts in England and Wales and All Souls without an ordinary rank.

Lent, Holy Week, the Triduum, both octaves, Easter weekdays and Sundays, Advent, Christmas and Ordinary Time all have explicit temporal designations. Movable candidates include Holy Family, Baptism, Trinity, Corpus Christi, Sacred Heart, Christ the King, Ascension, Pentecost, Easter, Mother of the Church, Immaculate Heart and the national Eternal High Priest feast. Ordinary Time Saturdays retain the permitted Marian memorial as an alternative.

`namesTemporalBaseline` supplies the identity of an existing temporal candidate. It must match that candidate's precedence and must be unique. It cannot promote a celebration to a stronger class. Unspecified equal memorials or feasts are rejected with a diagnostic. Explicit occurrence dispositions govern Mother of the Church, Immaculate Heart and the Eternal High Priest feast when it meets Augustine of Canterbury.

The default on optional memorial days is the temporal weekday. `alternatives` retains the choices in stable order. On privileged weekdays, eligible alternatives have `restrictedCommemoration = true`; obligatory memorials in Lent are reduced accordingly. Alternatives are omitted when a higher celebration excludes them. This API does not select Mass or Office texts.

`LiturgicalDay` exposes the selected identity, rank, precedence, season, colour, alternatives and transfer provenance. `scheduledFrom` records a national annual scheduling difference from the nominal date. `transferredFrom` records displacement by an impediment. Permanent national date reassignments remain changes to the inherited fixed facts.

The terminal uses this result, including its colour. It preserves the existing date, calendar and cross options and reports invalid inputs and unresolved occurrences on stderr. The engine has no terminal or Qt dependency.

## Verified rules and sources

The [Table of Liturgical Days](https://www.liturgyoffice.org.uk/Calendar/Info/Days.shtml) governs the thirteen classes. The [Universal Norms](https://www.liturgyoffice.org.uk/Calendar/Info/GNLY.pdf), especially paragraphs 5, 14–16, 19, 22–39 and 60, govern seasons, restricted memorials, ordinary Sunday impediments and eligible transfer destinations. General transfers seek the nearest day outside classes 1–8, preferring the following day when equidistant. The dedicated dispositions below override that search.

| Rule or fact | Authority |
| --- | --- |
| Joseph before Holy Week; Annunciation after the Easter Octave | [2008 Calendar Notes](https://liturgyoffice.org.uk/Calendar/Info/Ordo2008.pdf), [solemnity rubrics](https://www.liturgyoffice.org.uk/Calendar/Sunday/OT2Solemnities.shtml) |
| George after the Easter Octave, with Catherine of Siena blocking the first Monday in 2019 | [Bishops' Conference explanation](https://liturgyoffice.org/News/calendar-2018-2019/) |
| John the Baptist anticipated when it coincides with Sacred Heart | [2022 Calendar Notes](https://www.liturgyoffice.org.uk/Calendar/2022/Ordo-2022.pdf) |
| Epiphany on 6 January, except adjacent Sunday for Saturday or Monday; Ascension on Thursday; Corpus Christi on Sunday; national scheduling for Peter and Paul, Assumption and All Saints | [England and Wales decision effective Advent 2017](https://liturgyoffice.org/News/holydays/) |
| All Souls following nationally scheduled All Saints | [2025 Calendar Notes](https://www.liturgyoffice.org.uk/Calendar/2025/Ordo-2025.pdf) |
| Mother of the Church and its priority over another memorial | [2018 decree](https://www.vatican.va/roman_curia/congregations/ccdds/documents/rc_con_ccdds_doc_20180211_decreto-mater-ecclesiae_en.html), [notification of 24 March 2018](https://press.vatican.va/content/salastampa/en/bollettino/pubblico/2018/03/27/180327b.pdf) |
| Immaculate Heart collision reduces both obligatory memorials to optional | CDW 2671/98/L, identified in the [USCCB 2026 calendar, miscellaneous note 6b](https://www.usccb.org/resources/2026cal.pdf) |
| Eternal High Priest, Thursday after Pentecost, proper to England and Wales; takes the 27 May occurrence with Augustine | [Liturgy Office May proper](https://www.liturgyoffice.org.uk/Calendar/Sanctoral/May.shtml), [2021 Calendar Notes](https://www.liturgyoffice.org.uk/Calendar/2021/Ordo-2021.pdf) |
| Newman national feast and October 10 reassignments | [2020 decree](https://www.liturgyoffice.org.uk/Calendar/National/October/SJHN-CDW-Decrees.pdf) |
| National ranks and additions, including England's November 17 choices | [England summary](https://www.liturgyoffice.org.uk/Calendar/National/England1.shtml), [Wales summary](https://www.liturgyoffice.org.uk/Calendar/National/Wales1.shtml) |
| Walsingham feast in England | [Bishops' Conference announcement](https://www.cbcew.org.uk/our-lady-of-walsingham-now-a-feast-in-dioceses-of-england/) |
| Observance colours | [General Instruction of the Roman Missal, 346](https://www.vatican.va/roman_curia/congregations/ccdds/documents/rc_con_ccdds_doc_20030317_ordinamento-messale_en.html) |

The 222 fixed universal entries retain their baseline page or amendment references. The audit reviewed dates, ranks, identities, Lord-feast classifications and inheritance against the [baseline calendar](https://www.liturgyoffice.org.uk/Calendar/Info/General-Roman-Calendar.pdf) and the recorded Holy See amendments. Those amendments include Magdalene, Martha/Mary/Lazarus, the three Doctors added in 2021, Paul VI, Faustina, Loreto, John XXIII, John Paul II, Teresa of Calcutta and Newman. Irenaeus's designation now includes Doctor of the Church, with its [2022 decree](https://press.vatican.va/content/salastampa/en/bollettino/pubblico/2022/01/21/220121b.html) recorded in the entry.

The five Welsh entries previously inferred as obligatory from older descriptive pages are optional in this snapshot: Asaph, John Jones, Philip Evans and John Lloyd, David Lewis, and John Roberts. The national summary and [2026 published calendar](https://www.liturgyoffice.org.uk/Calendar/2026/Calendar-2026.pdf) govern their classification. Published annual calendars can contain inconsistent labels; the explicit Holy See memorial collision dispositions take precedence over a conflicting annual label.

## Validation and use

Run from the repository root:

```sh
make -j4 all test
./build/cathfetch --date=02032026 --calendar=wales --cross
./build/calendar_year 2026 england > /tmp/cathfetch-2026-england.tsv
```

The existing nlohmann JSON dependency remains required. No new runtime library was added. `calendar_year` produces one TSV record per date, including alternatives, scheduling, transfers and diagnostics.

The suite contains the six original test executables, two new C++ test executables, and CLI integration checks. The comprehensive test covers 44 complete years in each calendar: 1818, 1900, 1943 and every year from 2000 through 2040. Its 64,284 dates are each resolved in both forward and reversed definition order. Checks include unique annual identities, alternative ordering, colours, week ranges, transfer destinations, duplicate definitions, invalid inputs and transactional overlay failure. Published dates and ordo examples provide independent reference cases. The four explicitly unresolved projected occurrences below are asserted as diagnostics rather than accepted as observances. The final forced rebuild passed all eight C++ test executables and the CLI checks without compiler warnings; the comprehensive C++ test passed 385,938 checks. The same 385,938 checks also passed under AddressSanitizer and UndefinedBehaviourSanitizer. LeakSanitizer was disabled because the execution environment uses tracing, which it does not support. Four generated 2026 TSV diagnostics in `build/calendar-2026-*.tsv` each contain 365 resolved dates without diagnostics.

## Limits and remaining authority questions

- The computational input range is Gregorian 1583–4099. All years use the current snapshot of fixed facts and national rules. Earlier dates do not reconstruct the calendar then in force; future dates cannot anticipate decrees. The primary contemporary validation range is 2024–2030.
- On 24 June 2038, Thursday Corpus Christi coincides with the Nativity of John the Baptist in `grc` and `europe`, both class 3. The same occurs in the modern-rule projection of 1943. No verified disposition for this occurrence is encoded. These four tested results have `unresolvedReason` and both `conflictingObservances`, with no selected ID, colour or rank. Other dates in those years remain usable. The terminal returns an error on the unresolved day; the year utility records it. A future authoritative disposition is required before claiming complete coverage of that boundary year.
- The 2022 anticipation disposition for John the Baptist is applied when the same Sacred Heart occurrence recurs. It does not establish a general rule for other equal solemnities. Equal fixed solemnities without a disposition are rejected rather than scheduled by ID order.
- Date-only season labels treat Holy Thursday as Lent, Friday and Saturday as Triduum, and Easter Sunday as Easter Time with Triduum precedence. Holy Thursday and Holy Saturday have no single resolved colour because their evening celebrations differ. There is no Vespers, vigil, time-of-day or concurrence calculation.
- Violet is the deterministic All Souls colour; permitted alternatives such as black, rose on the appropriate Sundays and festive vestments are not separately modelled.
- Diocesan, parish, church dedication, religious-order and other local propers are absent. `europe` adds regional patrons and uses base scheduling; it does not represent every European nation's calendar. Arbitrary local calendar extensions can require additional authoritative dispositions.
- Holy-day obligations, readings and devotional or pastoral substitutions are outside this engine. Data consists of independently structured facts and source links, without copied liturgical texts or biographies.

No user design decision is needed for the implemented contemporary calendars. The unresolved Corpus Christi occurrence needs ecclesiastical authority rather than an arbitrary software tie-break.
