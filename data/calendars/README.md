# Cathfetch calendar data

These files contain the contemporary nominal fixed facts used by the engine. They are a snapshot as of 14 October 2026. The resolver applies the same snapshot to every accepted year; these files do not reconstruct historical calendars.

## Inheritance

```text
general_roman_fixed.json
    └── europe.json
          └── england_wales.json
                ├── england.json
                └── wales.json
```

Select `grc`, `europe`, `england` or `wales`. The engine loads each ancestor once. England and Wales inherit the five feast dates of Europe's six co-patrons, then their shared and separate national definitions. The universal base contains 222 fixed celebrations.

## Data fields and operations

Each fixed entry has a stable ID, nominal month/day, name and rank. All Souls has a null rank and explicit precedence. `rankSourceCalendar` in C++ records the layer that establishes the current rank, so national scheduling alone does not turn a general solemnity into a proper solemnity.

| Field or operation | Purpose |
| --- | --- |
| `additions` | Add an identity absent from ancestors. Duplicate IDs are errors. |
| `overrides` / `changes` | Change inherited rank, name, colour or rule metadata while retaining identity. |
| `date_reassignments` | Permanently change an inherited nominal date for that jurisdiction. |
| `feast_of_lord` | Classify a general feast of the Lord for level 5. |
| `precedence` | Explicit exceptional table class, used for Christmas, Epiphany and All Souls. |
| `names_temporal_baseline` | Give an existing temporal day its celebration identity without changing its class. |
| `date_rule` | Annual national scheduling, separate from displacement by an impediment. |
| `transfer_rule` | A verified exception to the general solemnity transfer search. |
| `colour` | The celebration's colour, independent of the underlying season. |
| `source_page`, `amendment`, `source_id`, `rule_source` | Trace the factual definition or exception to its authority. |

Colours follow [GIRM 346](https://www.vatican.va/roman_curia/congregations/ccdds/documents/rc_con_ccdds_doc_20030317_ordinamento-messale_en.html), with explicit white for the Conversion of Paul, Chair of Peter, John on 27 December and church dedications. Colour is stored as a fact, never inferred from a name by the runtime engine.

Full overlay application is transactional. Invalid dates, duplicate additions, duplicate override/reassignment targets, unknown targets and unknown change fields are rejected. Annual lookup retains candidates; rank and precedence determine which is observed.

## Audited national distinctions

David is a feast in England and a solemnity in Wales. George is a solemnity in England. Newman is a national feast under the [2020 decree](https://www.liturgyoffice.org.uk/Calendar/National/October/SJHN-CDW-Decrees.pdf); Denis and Leonardi are reassigned to 10 October in those calendars. Adalbert is reassigned to 24 April in England. Walsingham is a feast in England under the [2024 decision](https://www.cbcew.org.uk/our-lady-of-walsingham-now-a-feast-in-dioceses-of-england/). Elizabeth of Hungary is optional alongside Hilda and Hugh in England, remaining obligatory in Wales.

The Welsh optional memorials of Asaph, John Jones, Philip Evans and John Lloyd, David Lewis, and John Roberts follow the [national summary](https://www.liturgyoffice.org.uk/Calendar/National/Wales1.shtml) and 2026 calendar. Older descriptive pages contain conflicting memorial labels. Those labels no longer supply obligatory ranks here.

The original [universal baseline](https://www.liturgyoffice.org.uk/Calendar/Info/General-Roman-Calendar.pdf) predates several amendments. Per-entry Holy See decree references record those updates. Existing IDs remain stable when designations change; Irenaeus now includes Doctor of the Church without changing his identity.

Movable celebrations, optional choice policy, seasonal boundaries, impediments and suppression are implemented in C++. See [calendar engine status](../../CALENDAR_ENGINE_STATUS.md) for the full rule authorities, validation results, computational range, date-only conventions and unresolved 2038 Corpus Christi occurrence.
