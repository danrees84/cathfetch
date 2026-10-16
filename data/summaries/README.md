# Scripture summaries

`roman_summaries.json` holds the quotations and commentary shown by `--summary`.
There are 807 entries: 780 complete universal reading sets and 27 national or
partial proper sets kept for future work. V1 uses weekday readings for memorials
and omits national propers, so those archived entries are not selected.

Each summary pairs a verse with 40–70 words of commentary. The selections were
made with AI assistance. They are not verses designated by the Church. Everything is stored locally; the app does not generate commentary when it runs.

## Record fields

| Fields | Contents |
| --- | --- |
| `reading_set_id`, `occasion`, `cycle`, `mass` | The existing Lectionary assignment. |
| `category`, `testament` | Which reading supplies the verse, and its Old/New Testament classification. |
| `source_reading` | The full appointed passage containing the verse. |
| `book_id`, `chapter`, `verse`, `reference` | The verse's address in Douay–Rheims. |
| `quotation`, `translation` | The exact text and its translation. |
| `quotation_source`, `source_xml_sha256` | The source URL and hash of the bundled Bible XML. |
| `explanation`, `liturgical_connection` | Biblical context and the connection with the day. |
| `rationale` | Why this verse was selected. |
| `reference_status` | The verification status of the reading assignment. |
| `editorial_status`, `reviewer`, `review_date` | Whether someone has reviewed the commentary, and who. |

A responsorial canticle may be from the New Testament without being a Gospel
reading. Keep the category and testament fields separate.

`technical_validation=checked` and
`quotation_verification=bundled_source_checked` mean the record passes the
software checks and matches the bundled text. Daily loading checks the selected
record; tests and review tools check every record. All entries currently have
`reference_status=published_table` and `editorial_status=ai_draft`. Only set
`human_reviewed` after a real review, with a reviewer and date.

Quotations come from the [public-domain Douay–Rheims edition of 1899](https://ebible.org/engDRA/copyright.htm).
The [Bible guide](../bibles/README.md) explains its source and verse numbering.
Another translation would need its own source text and correspondence checks.

## Editing and exporting

```sh
make all build/summary_catalogue
./build/summary_catalogue > /tmp/cathfetch-eligible-verses.json
make summary-review
python3 tools/summary_review.py check
./build/summary_year 2026 england
make -j4 test
```

The catalogue lists eligible verses using the same C++ parser as the app.
Consider all the appointed readings before choosing one. Write the commentary
in the JSON, then run `make summary-review` to update the Markdown collection.
The exporter copies the stored entries; it does not write commentary or select
verses.

The chosen verse must fit wholly within its reading. Partial boundary verses,
merged verse divisions and mappings verified only for a whole passage are
excluded. The loader checks the quotation, book, address, reading category,
cycle, Mass form, translation and source details.

The [review collection](../../docs/scripture-summaries/README.md) is organised by
season and cycle. Its reports cover:

- `coverage.json`: completeness, verification and possible duplicate commentary.
- `distribution.json`: selections from first readings, Psalms, second readings
  and Gospels.
- `year-coverage.tsv`: resolved dates for 2024–2030 in all four calendars.
- `alternative-compatibility.json`: whether each of the 119 passage alternatives
  still contains the chosen verse.

Every applicable v1 alternative contains its summary verse. Three archived
proper alternatives need another selection: Martha/Mary/Lazarus (Luke 10),
Timothy/Titus (Titus 1) and Our Lady of Sorrows (Luke 2).
