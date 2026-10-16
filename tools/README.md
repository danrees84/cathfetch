# Developer tools

These tools prepare data or help inspect it. Cathfetch does not run them during
normal use, and `make install` does not install them.

`make` builds the app. Build the C++ diagnostics separately when needed:

```sh
make -j4 tools
./build/calendar_year 2026 england > /tmp/calendar-2026.tsv
./build/lectionary_year 2026 england > /tmp/readings-2026.tsv
./build/summary_year 2026 england > /tmp/summaries-2026.tsv
```

The year tools write one tab-separated row per date. `summary_catalogue` lists
eligible verses for editors; `make summary-review` uses it to regenerate the
Markdown collection and coverage reports.

## Python scripts

| Script | Use |
| --- | --- |
| `bible_data.py` | Convert the source Bible XML or build the offline JSON index. `make` runs the index step automatically. |
| `bible_versification.py` | Reproduce the verse mappings from the recorded STEPBible source. |
| `lectionary_audit.py` | Check that the stored independent-reference ledger still matches the reading data. |
| `current_lectionary_audit.py` | Extract citation facts from ordo text, compare them with the data and check the correction ledger. |
| `summary_review.py` | Validate stored summaries and export or check the review collection. |
| `universalis_audit.py` | Collect and compare the public date calendar and available Mass-page references. |

The Bible and reference-audit scripts do different jobs; they are not alternate
implementations of the calendar or reading resolver. Most work from local source
files. Universalis collection is an explicit network operation, separate from
the offline tests. Use a script's `--help` where provided, and see the
[Bible](../data/bibles/README.md), [reading](../data/lectionary/README.md) and
[summary](../data/summaries/README.md) guides for source files and commands.
