# Tests

Run the complete suite from the project directory:

```sh
make -j4 test
```

For a smaller run while working on a particular part:

| Command | Checks |
| --- | --- |
| `make test-calendar` | Dates, seasons, precedence, transfers and regional calendars. |
| `make test-readings` | Reading selection, Bible lookup and summary validation. |
| `make test-cli` | Arguments, terminal layout, redirected output and installation. |
| `make test-data` | Data-tool tests and generated-summary freshness. |

A group stops when a check fails. Running one group does not replace the full
suite before a release.

## C++ suites

There are nine executables. The smaller examples use tables of dates and
expected results; the broader tests also run complete years.

| File | Purpose |
| --- | --- |
| `calendar_engine_test.cpp` | Published dates, overlays, invalid data and a 44-year consistency sweep in each calendar. |
| `liturgical_precedence_test.cpp` | Precedence classes, movable dates and Advent formatting. |
| `movable_observances_test.cpp` | The candidates supplied to the resolver, including named temporal baselines. |
| `observance_resolver_test.cpp` | Observed names, ranks and seasons, national dates and competing candidates. |
| `solemnity_occurrence_test.cpp` | Lord/saint solemnity conflicts and the patronal exception. |
| `lectionary_test.cpp` | Reading cycles, Mass forms, alternatives and complete-year reading selection. |
| `scripture_test.cpp` | Passage parsing, verse numbering and available Bible text. |
| `scripture_summary_test.cpp` | Quotations, reading membership, malformed records and full-year summary coverage. |
| `terminal_render_test.cpp` | ANSI/Unicode widths, wrapping and cross layouts. |

The shell and Python CLI tests run the actual executable. The Python data-tool
tests use small fixtures to check the audit and mapping scripts.
No test needs a live website or GitHub account. When adding a C++ suite, add
its executable to the appropriate test group in the Makefile.

Literal examples catch specific mistakes; sweeps catch missing days, repeated
celebrations and ordering problems. Both are useful. A successful sweep does not
independently verify every date or reading against a Church publication.

The former print-only fixed-celebration example was removed. The old JSON test's
Lord/saint feast assertions now sit with the fixed precedence cases. Advent
formatting and temporal-day examples were merged into the precedence and
resolver suites rather than kept as separate executables.
