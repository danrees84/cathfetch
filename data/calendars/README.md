# Horarium / Cathfetch — England and Wales calendar overlays (draft)

**As-of:** 11 October 2026  
**Status:** independently structured nominal fixed-date facts; **not a complete resolved liturgical calendar**.  
**Base:** `general_roman_fixed.json` (the previously prepared 222-entry General Roman Calendar, transcription review still pending).

## Files and inheritance

```
general_roman_fixed.json
    └── europe.json
          └── england_wales.json
                ├── england.json
                └── wales.json
```

Use `england` *or* `wales` as the top layer, **not both**. Load every ancestor once. Each layer contains only differences, so existing universal celebrations are not duplicated.

## Data operations

- **`additions`**: new fixed nominal celebrations (`id`, month, day, name, rank).
- **`overrides`**: partial `changes` to the named inherited entry. Changes are merged by key; do not replace the entire entry.
- **`date_reassignments`**: change the nominal calendar date of an inherited entry **for this jurisdiction**; this does not move it in its parent calendar.
- **`sources` / `source_id`**: links back to the Liturgy Office, including the 2020 Holy See decree for Newman.

**Overlay application** is not precedence resolution. Resolve calendar candidates first; then independently apply universal liturgical precedence, optional memorial selection, jurisdiction-specific transfers and year-dependent impediments. No colours, Mass readings, copyrighted text or biography are stored here.

## Key examples

- **Europe**: the five feast dates of Europe's six co-patron saints are rank overrides on General Roman Calendar entries.
- **1 March**: St David is a *feast* in England and a *solemnity* in Wales. He is added in `england_wales.json` at rank feast; `wales.json` overrides that shared entry to solemnity.
- **9 October**: St John Henry Newman is a *feast in both England and Wales* under the 6 August 2020 decree (prot. 353/19), though his General Roman Calendar rank is optional memorial. That decree also assigns St Denis and St John Leonardi to **10 October** in both national calendars. This is a fixed national-date reassignment, not a generic precedence transfer. The underlying General Roman Calendar date remains 9 October.
- **23/24 April**: St George is a solemnity in England; St Adalbert is assigned to 24 April in England, but keeps 23 April in the universal/Welsh layers. A separate precedence engine still needs to handle years in which St George is impeded.
- **3 November**: St Winefride is shared by England and Wales, so she is added only once.
- **23 July in Wales**: St Bridget of Sweden is a European feast, while St Philip Evans and St John Lloyd are Welsh memorials on the same fixed date. Keep both candidates; resolve via precedence.

## Sources

- Universal baseline: https://www.liturgyoffice.org.uk/Calendar/Info/General-Roman-Calendar.pdf
- Liturgy Office national summary, England: https://www.liturgyoffice.org.uk/Calendar/National/England1.shtml
- Liturgy Office national summary, Wales: https://www.liturgyoffice.org.uk/Calendar/National/Wales1.shtml
- Further descriptions and some additional rank labels, Wales: https://www.liturgyoffice.org.uk/Calendar/National/Wales2.shtml
- Regional patrons: https://www.liturgyoffice.org.uk/Calendar/Info/background2.shtml
- Holy See decree for England and Wales, St John Henry Newman and October 10 reassignments: https://www.liturgyoffice.org.uk/Calendar/National/October/SJHN-CDW-Decrees.pdf
- 2026 cross-check: https://www.liturgyoffice.org.uk/Calendar/2026/Calendar-2026.pdf

## Known limits and review tasks

1. The inherited 222-entry general dataset remains a first-pass transcription with a manual audit outstanding.
2. National summary pages may lag liturgical decrees or use older names (e.g., "Bl John Henry Newman"). The 2020 decree and the 2026 calendar take precedence where the summary is stale.
3. The Wales detailed page labels a handful of days "Memorial" where the summary rank column is blank; these have been recorded as `memorial`, with provenance, but warrant a final review against current approved calendar texts.
4. **Year-specific transfers** (e.g., St George impeded by Holy Week or an Easter Sunday), feasts of the Lord, movable solemnities and obligations are out of scope, and should be coded separately, with a proper precedence table.
5. No diocesan, religious-order or parish-local feasts are included.
6. This is independently structured factual information, not a reproduction of copyrighted biographies, Missal texts or a scraped publisher database. Source pages and decree links identify provenance, not permission to redistribute their text.
7. The files are data proposals, not yet wired into the C++ calendar engine; no claims of runtime/precedence correctness are made.

## Validation

All JSON files were parsed successfully; all override/reassignment targets resolve to IDs in an inherited layer; all source references resolve within their files; added IDs are unique, ranks from the allowed vocabulary, and recorded month/day values within basic ranges. Validation does **not** verify every day's official celebration or legal status.

## Suggested next steps

First review entries and ranks, then implement merge operations and a simple candidate lookup. Only after that introduce precedence and date transfers. Keep the fixed facts separate from the C++ rules.
