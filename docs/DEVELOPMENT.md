# Building and running Cathfetch

Cathfetch uses C++20, GNU Make, Python 3 for data tooling and tests, and the
nlohmann/json headers. The calendar, Lectionary and Scripture modules do not
depend on terminal rendering.

```sh
make -j4 all
make -j4 test
./build/cathfetch --help
./build/cathfetch --date=30112025 --summary
```

`make` builds only the application and its Bible index. Use `make tools` for
optional year diagnostics and the summary catalogue. `make test-calendar`,
`make test-readings`, `make test-cli` and `make test-data` run individual groups;
`make test` still runs all of them. See the [test guide](../tests/README.md) and
[tool guide](../tools/README.md).

The executable can be called from any working directory. A development build
finds the project's `data/` directory relative to its own location, and the
generated Bible index alongside the executable in `build/`. Keep this directory
layout when running an uninstalled build. Linux's `/proc/self/exe` identifies the
executable, including when it is invoked through a symbolic link.

## Installation and packaging

```sh
make install PREFIX="$HOME/.local"
```

This installs `bin/cathfetch` and `share/cathfetch/` under the chosen prefix.
The generated Bible index is installed at `share/cathfetch/bibles/en_dra.json`.
Source data, licensing information and provenance accompany it. Installed data
is located relative to the executable, so the prefix can be relocated together.

Packagers can stage the same layout without writing to the system:

```sh
make install PREFIX=/usr DESTDIR=/tmp/cathfetch-package
```

For a custom data location, set `CATHFETCH_DATA_DIR` to a complete data directory
containing `calendars/`, `lectionary/`, `summaries/` and `bibles/`, including the
generated Bible index if using Scripture text or summaries. An absolute path is
recommended. An explicit override never falls back to bundled data when a file
is missing. Explicit filenames passed to engine loaders retain their normal
filesystem semantics; tests and other applications can supply their own files.

## Reading the terminal code

`main.cpp` parses options and calls the existing engines. In
`terminal_display.cpp`, `appendCalendar`, `appendReadings` and `appendSummary`
build their sections using `InformationRows`, a small wrapper around the
existing word-wrapping function. `informationRows` puts the sections in order;
`renderLiturgicalDay` places the cross beside or above the completed rows.
There are no calendar calculations in these display helpers.

## Summary validation

The CLI's singular `loadScriptureSummary` validates the selected record against
its appointed reading and bundled Bible, including quotation, identity, cycle,
Mass form, provenance and editorial fields. It still parses the JSON document,
checks its schema/policy and rejects duplicate identities across the catalogue.
Unused records do not undergo verse lookup or commentary validation on a daily
invocation. No prevalidated cache or second copy of the summary data is needed.

The plural `loadScriptureSummaries` and default `parseScriptureSummaries` retain
exhaustive validation for tests, catalogue exports and year diagnostics. Run
`make test` after data changes; `make summary-review` regenerates the editorial
collection, and the test suite checks that exports are current.

Technical validation does not constitute human editorial review or authoritative
current-edition Lectionary collation. Actual review statuses remain in the data
and generated collection. See [the summary status](../SCRIPTURE_SUMMARY_STATUS.md)
for outstanding content work.
