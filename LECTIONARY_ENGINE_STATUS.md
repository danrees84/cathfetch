# Cathfetch lectionary engine status

As of 14 October 2026, Cathfetch has a separate, offline, reference-only lectionary engine and an opt-in terminal display. **The every-day, current-edition milestone is not complete.** The implementation deliberately reports missing assignments and distinguishes published reference tables from records collated against the 2024 England and Wales Lectionary. None of the supplied records claims that latter verification.

## Implementation

`readingCycles(date)` returns the liturgical year by its ending civil year, Sunday A/B/C and weekday I/II. Both labels change at the First Sunday of Advent. Ordinary Time weekdays use the appropriate I/II assignment; seasonal weekdays have annual assignments except where a published conditional provision applies. The 28–29 November 2026 transition is A/II to B/I.

`loadLectionary()` loads `data/lectionary/roman_references.json` and validates identities, selectors, calendars, roles, cycles, forms, alternatives, source provenance and overlapping defaults. A malformed file fails as a whole. No selection depends on JSON ordering. `validateLectionary()` is also available to callers constructing datasets directly.

`resolveMassReadings(day, data, calendarName, mass)` consumes the already resolved `LiturgicalDay`. Pass the same calendar identifier used to resolve that day. It does not resolve precedence, move saints or independently schedule movable feasts. Observance IDs carry proper readings through both obligatory transfers and national schedules. The temporal lookup derives weekday/week keys from existing calendar functions, not displayed names. Unknown proper provisions never fall back to an unrelated weekday.

`ReadingSet` holds an ordered list of citations, their roles and conditional alternatives, an optional Lectionary number, calendar applicability, cycle, form, provenance and verification status. It distinguishes proper readings, accommodated suggestions, partial proper memorial readings and alternative-only sets. Full Scripture, psalm responses, liturgical prayers and biographies are absent. Book abbreviations follow the sources, except ambiguous `Eccl` has been expanded to Sirach or Ecclesiastes. References are display citations, not parsed Bible addresses suitable for automatic translation linking.

The result distinguishes available published references, missing data, an unresolved calendar identity and absence of a daytime Mass. `Available` means a reference set exists; **inspect `selected.verification` and the provenance before treating it as a current-edition assignment**. All supplied sets are `PublishedTable`. A partial proper memorial merges its mandatory slot(s) with the weekday set and retains both sources in its notes. If the weekday set is missing, the result remains missing.

## Selection policy

- The default form is the day's Mass; Christmas offers Day, Night, Dawn and Vigil. Pentecost offers Day, simple Vigil and extended Vigil. Easter offers Day and Vigil. John the Baptist, Peter and Paul, and the Assumption include their published Vigil forms.
- Holy Thursday defaults to the explicitly labelled evening Mass. Good Friday selects its Passion liturgy, without calling it a Mass. Holy Saturday has no daytime Mass; an explicit `vigil` request accesses the Easter Vigil. All seven Old Testament readings and their psalms are retained there; pastoral omissions are not automated.
- Forms are selected for the observed liturgical identity, not a calculated civil-time transition. For example, request Christmas's Vigil using the Christmas identity/date and `--mass=vigil`; 24 December is still the Advent daytime identity. This is not a first-Vespers or anticipated-Sunday-Mass scheduler.
- Ordinary memorials retain weekday readings according to explicit data policies. The supplied policies cover non-biblical saints; unverified biblical, Marian and angelic memorial provisions remain gaps. Mother of the Church has a proper set. Martha, Mary and Lazarus supplies the proper Gospel with weekday readings in the other slots. Suppressed celebrations have no influence on readings.
- The calendar's default on optional memorial days remains the temporal day. Optional celebration choices remain in the calendar API; the CLI does not yet select one of those alternatives. Selecting an optional memorial in a future client will require its resolved identity and a verified reading policy.
- Short forms and published alternatives remain attached to their slots. Year A sets for Lent Sundays 3–5 are retained as alternatives in B/C. Conditional Gospels for Lent week 5 Monday and Easter week 4 Monday are selected by cycle. Advent week 1 Monday in A uses the published alternative Isaiah 4 to avoid repeating Sunday's Isaiah 2.
- Holy Family and Baptism use the published cycle-specific pre-Gospel option by default in B/C, retaining the common pre-Gospel readings as alternatives. Feasts of the Lord have three readings on Sunday and two on weekdays, with the other pre-Gospel reading available as an alternative.
- All Souls uses the source's suggested first selection, explicitly marked accommodated, with its other suggested Gospels preserved. It does not claim a mandatory A/B/C scheme or complete coverage of the three Masses and the Common of the Dead.

## Reference coverage and remaining work

The dataset contains **600 reading sets and 204 explicit memorial policies**. It covers all three temporal Sunday cycles, principal general solemnities, both octaves, the available seasonal weekday tables and Ordinary Time weekdays in weeks 1–20 in I/II. It includes the published England/Wales Eternal High Priest readings. General reference assignments apply to all four calendars only where explicitly listed; a European feast cannot inherit a Roman memorial's weekday policy.

The [coverage report](data/lectionary/coverage_2024_2030.tsv) records actual civil-year sweeps, not ecclesiastical certification. For 2026:

| Calendar | Dates with published-table sets | Explicit gaps | No daytime Mass | Unresolved calendar |
| --- | ---: | ---: | ---: | ---: |
| GRC | 275 | 89 | 1 | 0 |
| Europe | 271 | 93 | 1 | 0 |
| England | 265 | 99 | 1 | 0 |
| Wales | 270 | 94 | 1 | 0 |

The public Liturgy Office pages [OT5](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT5.shtml), [OT6](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT6.shtml) and [OT7](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT7.shtml) have navigation for weeks 21–34 but no reading tables. These weeks have not been filled by inference. Missing saints include several apostles, Lawrence, Timothy and Titus, Barnabas, the Passion of John the Baptist and several Marian/angelic memorial provisions. European patrons and national feasts need their verified proper/Common selections. David and George require selections from Commons, including the extra reading for a solemnity; their incomplete public summaries are not presented as full sets. Additional current Vigil provisions and Commons also require curation.

The public tables are not a sufficient independent reference for certifying complete liturgical years against the 2024 edition. The [current-edition FAQ](https://www.liturgyoffice.org.uk/Resources/Lectionary/LM-FAQ.shtml) confirms continuing Sunday selections and additional national/saints provisions, but does not supply a complete citation index. The [Hallam 2025–26 ordo](https://hallam-diocese.com/wp-content/uploads/sites/9/2023/12/HALLAM-ORDO-2025-26.pdf) was consulted as an independent current reference; its diocesan provisions and incomplete extraction must not be generalised into national assignments. Completing the milestone requires a verified current-edition reference index or further individually verified official assignments for the missing areas. Access to Scripture text is unnecessary.

The older tables also contain transcription defects. Independently checked repairs and their second sources are recorded in each affected entry: Acts 4 restored for Easter week 2 Monday; Easter Monday's Acts endpoint corrected to 33; Pentecost C's optional Gospel starts at John 14:15; Easter week 4 Monday's psalm range ends 3–4 rather than 4–4. An invalid Psalm 2 response verse 13 was omitted. A malformed Peter and Paul acclamation citation was omitted rather than guessed. This is not an exhaustive textual audit, and precise psalm verse numbering, responses, optional selections and Lectionary numbers still require current-edition collation.

## Sources and rule verification

- [Lectionary Introduction](https://www.liturgyoffice.org.uk/Resources/Rites/LM-Introduction.pdf), particularly 66–70, 82–84, 93, 96: cycles, memorials, feast readings and seasonal arrangements. Sunday C corresponds to ending years divisible by three; the liturgical cycle begins at Advent. Weekday I is odd and II even.
- [Advent Sundays](https://www.liturgyoffice.org.uk/Calendar/Sunday/AdventSunday.shtml), linked Christmas/Lent/Easter/Ordinary Time Sunday tables, [solemnities](https://www.liturgyoffice.org.uk/Calendar/Sunday/OT2Solemnities.shtml) and [feasts of the Lord](https://www.liturgyoffice.org.uk/Calendar/Sunday/Feasts.shtml): references and permitted forms/options.
- [Advent weekdays](https://www.liturgyoffice.org.uk/Calendar/Weekday/Advent.shtml), linked seasonal tables and [Ordinary Time weekdays](https://www.liturgyoffice.org.uk/Calendar/Weekday/OT1.shtml): annual versus I/II assignments and conditional readings.
- [Current Lectionary information](https://www.liturgyoffice.org.uk/Resources/Lectionary/): the edition introduced at Advent 2024 uses ESV–CE and Abbey Psalms and Canticles. Its copyrighted texts are not included here.
- [USCCB Easter Monday](https://bible.usccb.org/bible/readings/040521.cfm), [Easter week 2 Monday](https://bible.usccb.org/bible/readings/041315.cfm), [Easter week 4 Monday](https://bible.usccb.org/bible/readings/050123.cfm), [Pentecost](https://bible.usccb.org/bible/readings/051516-pentecost-day.cfm) and [Holy Family](https://bible.usccb.org/bible/readings/123016.cfm): independent citation checks and clarification of Sirach. US psalm numbering is not silently substituted for the source convention.

Each JSON assignment has its own source URL and qualification. Contemporary rules are projected throughout the calendar engine's supported date range; this does not reconstruct historical lectionaries or diocesan propers.

## Running and validation

```sh
make -j4 all test
./build/cathfetch --date=30112025 --readings --cross
./build/cathfetch --date=29112026 --readings
./build/cathfetch --date=25122026 --mass=night
./build/cathfetch --date=04042026 --mass=vigil
./build/cathfetch --date=24052026 --mass=vigil-extended
./build/lectionary_year 2026 england > /tmp/lectionary-2026.tsv
```

Calendar-only output is unchanged. Reading output is opt-in, explicitly qualifies the published-table data, and wraps with the existing renderer. Both ordinary and cross displays are tested at 120, 100, 80, 60 and 40 columns.

All eleven C++ test executables and both CLI suites pass. The new test performs 43,953 checks across 14,612 resolved dates (2008, 2022, 2024–2030 and 2038, all four calendars), including reversed assignment order. It reports 10,708 published-table results, 3,864 explicit gaps and 40 Holy Saturdays without a daytime Mass. Existing calendar regressions remain unchanged. These are implementation/coverage checks and selected independent reference cases, **not complete-year independent verification of every citation**.

The new cycle, loader and resolver translation units and the lectionary test also passed AddressSanitizer/UndefinedBehaviorSanitizer checks, linked against the unchanged calendar objects. Leak detection was disabled for that run; the entire existing engine was not rebuilt with instrumentation.
