# Offline Scripture summaries

`roman_summaries.json` is the canonical editorial dataset. Its 807 records cover the stored reference catalogue, including 780 complete universal-scope records and 27 retained national or partial proper records. Cathfetch's **v1 universal-reading policy remains unchanged**: memorials use the weekday readings, national propers are omitted, and universal feasts retain local scheduling. Archived proper summaries are not enabled by their presence here.

Every entry is an **AI draft, not human-reviewed**. The selection is editorial, not an officially designated best verse. The explanation and liturgical connection together contain 40–70 words. No runtime generation or network access occurs.

## Reading the data

- `reading_set_id`, `occasion`, `cycle` and `mass` identify an existing Lectionary record. The summary engine never calculates a feast date or chooses readings.
- `category` is the actual reading role; `testament` classifies its biblical book. A responsorial canticle can therefore be a New Testament text without being a Gospel selection.
- `source_reading` preserves the appointed primary reference. `book_id`, `chapter`, `verse` and `reference` give the quoted Douay–Rheims address.
- `quotation` is copied from the pinned Bible index, never reconstructed from memory. `translation`, `quotation_source` and `source_xml_sha256` identify its source. The Bible metadata also pins the original downloaded USFX member.
- `explanation` gives biblical context; `liturgical_connection` relates it to the celebration or its place in the reading sequence. `rationale` records the selection's editorial focus.
- `technical_validation=checked` and `quotation_verification=bundled_source_checked` are checked again on loading. They do not mean independent proofreading, ecclesiastical approval or human review.
- `reference_status` must agree with the underlying Lectionary record. All current entries retain `published_table`, not `current_edition`.
- `editorial_status=ai_draft`, with null reviewer/date, records the actual authorship status. A future manual `human_reviewed` decision requires a named reviewer and review date. No tool upgrades this status.

Quotes reproduce the [public-domain Douay–Rheims American Edition of 1899](https://ebible.org/engDRA/copyright.htm), using the existing [73-book source and mapping metadata](../bibles/README.md). They are not the ESV–CE or Abbey texts currently proclaimed in England and Wales. Another licensed translation would need its own Bible index, provenance and correspondence checks; the summary model does not bake in a translation name.

## Authoring and reproducibility

```sh
make all build/summary_catalogue
./build/summary_catalogue > /tmp/cathfetch-eligible-verses.json
make summary-review
python3 tools/summary_review.py check
./build/summary_year 2026 england
make -j4 test
```

The catalogue exposes candidates through the **production C++ Scripture parser**. It is an authoring aid, not a ranking algorithm. Select a verse after considering the complete reading set, write the original commentary in the canonical JSON, then regenerate the review collection. The exporter copies stored editorial work; it does not generate prose or choose verses. Repeating these commands produces the same Markdown and reports from the same inputs.

The Scripture parser now records whether an entire source verse is eligible. Partial boundary verses, non-identity/merged divisions and whole-passage-only correspondences are excluded conservatively. Interior whole verses can remain eligible. The summary loader checks the actual book, address, text, source reading, category, cycle, form, translation and provenance; a numeric match alone is insufficient. These are checks within the documented source-numbering profile, not independent proof of every Lectionary citation or ancient textual correspondence.

The [generated collection](../../docs/scripture-summaries/README.md) lists all records by season and cycle, with provenance and status. Ordinary Time weekday chapters are split into weeks 1–17 and 18–34. `coverage.json` separates source matching, reference certification and human review; `distribution.json` records selection categories; `year-coverage.tsv` measures real selections across 2024–2030 in all four calendars.

`alternative-compatibility.json` checks all 119 stored non-acclamation passage alternatives. The chosen verse remains available with every applicable v1 alternative. Three alternatives to archived partial propers need a separate editorial selection: Martha/Mary/Lazarus (Luke 10), Timothy/Titus (Titus 1) and Our Lady of Sorrows (Luke 2). V1 does not select those propers. The engine continues to use its existing deterministic primary passages; this feature adds no alternative-reading selector.

## Editorial process and limits

Work began with 27 distinct commentaries covering 36 reading sets: Advent A/B/C, both weekday cycles, Christmas, Lent, the Triduum, Easter and principal feasts. The sample was assessed for contextual specificity, literal quotation, restrained language and the separation of grace from moral self-sufficiency. This was **AI self-review**, not human editorial approval.

Subsequent batches completed Sundays, repeated passages, feasts, Psalms and remaining weekdays. Shared biblical explanations are reused where the selected verse legitimately recurs; context-specific connections remain separately stored. Distribution review prompted stronger Psalm selections where their imagery directly illuminates the other readings, such as exile on Lent IV B and the rejected stone on Easter Day. Alternative-passages review prompted selections common to the permitted readings, rather than presenting a default-only verse as universal.

The similarity heuristic flags substantially overlapping commentary for different selections; its empty report does not establish originality, theological quality or editorial approval. Human review remains necessary for all drafts. Complete current-edition reference collation is also outstanding: published tables and partial diocesan-ordo comparisons cannot establish every subverse, option and acclamation. Coverage of these 807 stored records is not a claim to possess the complete Catholic Lectionary, diocesan propers, ritual or votive Masses.
