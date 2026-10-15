# Cathfetch

A small, native Catholic terminal application, loosely inspired by Fastfetch.
Cathfetch displays the liturgical day, with optional Mass reading references,
offline Scripture text and a short Scripture summary.

Written in C++20 and developed on Arch Linux, it is also a project for learning
modern C++. Its calendar, Lectionary and Scripture engines are independent of
the terminal interface, with future reuse in Horarium in mind.

**Development preview:** the application works offline, but Lectionary
current-edition verification and human editorial review of the summaries remain
incomplete. See [coverage and limitations](#coverage-and-limitations).

## Build and run

Requirements: Linux, a C++20 compiler, GNU Make, Python 3 and the
[nlohmann/json headers](https://archlinux.org/packages/extra/any/nlohmann-json/).
Python is used for build-time data preparation and tests; normal application
execution uses C++ and requires no network connection or AI service.

On Arch Linux, install the dependencies:

```sh
sudo pacman -S --needed gcc make python nlohmann-json git
```

From the project directory:

```sh
make -j4 all
./build/cathfetch
./build/cathfetch --help
```

The build prepares the offline Bible index from the bundled source. Keep the
`build/` and `data/` directories in their project layout when running an
uninstalled copy. The executable can be called from any working directory.

## Usage

```sh
# Today's liturgical day, with the cross
./build/cathfetch --cross

# A chosen date and regional calendar
./build/cathfetch --date=08122026 --calendar=england --cross

# Compact Mass reading references
./build/cathfetch --date=30112025 --readings

# A selected Scripture quotation and original commentary
./build/cathfetch --date=30112025 --summary

# Combine the cross, references and summary
./build/cathfetch --date=30112025 --cross --readings --summary

# Full offline verses for the Christmas Night Mass
./build/cathfetch --date=25122026 --mass=night --full-verses
```

For example, `--date=30112025 --readings` produces the following plain output
at a sufficiently wide terminal width:

```text
Sunday, 30 November 2025
● Advent
First Sunday of Advent

Readings:
Isa 2:1-5; Ps 121:1-2, 4b-5, 6-7, 8-9 r. 1; Rom 13:11-14; Matt 24:37-44
```

| Option | Purpose |
| --- | --- |
| `-h`, `--help` | Show usage and available options. |
| `--date=DDMMYYYY` | Choose a date; the default is today in local time. |
| `--calendar=NAME` | Select `grc` (default), `europe`, `england` or `wales`. |
| `--cross` | Show the cross beside or above the information. |
| `--readings` | Show compact Mass reading references. |
| `--summary` | Show a stored quotation and commentary, independently of `--readings`. |
| `--full-verses` | Show offline Douay–Rheims text; implies `--readings`. |
| `--mass=FORM` | Choose an available liturgical form; implies `--readings`. |

Options with values also accept a space, such as `--calendar wales`.
Available form names are `day`, `vigil`, `vigil-extended`, `night`, `dawn`,
`evening` and `passion`. A form must exist for the selected day; `passion`
identifies Good Friday's liturgy. Without `--mass`, the default is normally the
daytime Mass. Holy Saturday has no daytime Mass; use `--mass=vigil` for the
Easter Vigil.

The season and named celebration have their own liturgical colour indicators.
Long text wraps within the available width. With `--cross`, narrow terminals
switch to a stacked layout. Redirected output omits ANSI colours; width falls
back to `COLUMNS`, then 80 columns, when a terminal width is unavailable.

## Install

For a user installation:

```sh
make install PREFIX="$HOME/.local"
~/.local/bin/cathfetch --summary
```

The executable and data are installed under `bin/` and `share/cathfetch/` in
that prefix. Add `~/.local/bin` to your `PATH` to invoke it as `cathfetch`.
Packagers can use `DESTDIR` for a staged installation. `CATHFETCH_DATA_DIR`
overrides the complete data directory; see the [development guide](docs/DEVELOPMENT.md)
for details.

## Coverage and limitations

- **Calendar:** contemporary General Roman Calendar with European, England and
  Wales overlays, movable celebrations, precedence, colours and transfers.
  Accepted dates are 1583–4099, using the contemporary calendar snapshot rather
  than reconstructing historical practice. Full-year regressions cover
  2024–2030 in all four selections; selected boundary years are tested too.
- **V1 reading policy:** universal Sunday and weekday readings, feasts and
  solemnities, retaining local scheduling of universal celebrations. Memorials
  use the regular weekday readings; national saints' propers are omitted.
  The displayed regional celebration can therefore differ from the reading
  selection's context.
- **References:** the stored catalogue contains 807 reading sets. Published
  sources and correction evidence are recorded, but complete verification
  against the current England and Wales Lectionary remains outstanding.
  Source-specific Psalm numbering is preserved.
- **Scripture:** the bundled 73-book Douay–Rheims American Edition of 1899 is a
  public-domain electronic transcription, with source and mapping provenance.
  It is not the translation currently proclaimed in England and Wales. Four
  documented Tobit passage correspondences remain unavailable. Partial-verse
  references display whole verses with an explanatory notice.
- **Summaries:** all 807 stored sets have prewritten editorial selections and
  commentary. These remain AI drafts awaiting human review. Quotations are
  checked against the bundled Bible; technical checks do not constitute
  independent proofreading or theological approval. Missing content produces
  an explicit diagnostic rather than a generated substitute.
- **Scope:** diocesan propers, historical calendars and all ritual or votive
  Masses are outside current coverage. A date-only calendar cannot represent
  every transition within a day. The 2038 anticipation of St John's Nativity
  to 23 June remains a documented provisional transfer.

Detailed evidence, policies and remaining work:

- [Calendar engine status](CALENDAR_ENGINE_STATUS.md)
- [Lectionary engine status](LECTIONARY_ENGINE_STATUS.md)
- [Scripture Summary status](SCRIPTURE_SUMMARY_STATUS.md)
- [Summary editorial collection, coverage and distribution](docs/scripture-summaries/README.md)
- [Bible source and verse correspondences](data/bibles/README.md)

## Tests and diagnostics

```sh
make -j4 test

# Tab-separated year-at-a-glance reports
./build/calendar_year 2026 england > /tmp/calendar-2026.tsv
./build/lectionary_year 2026 england > /tmp/readings-2026.tsv
./build/summary_year 2026 england > /tmp/summaries-2026.tsv

# Reproduce the Markdown editorial collection and coverage reports
make summary-review
```

Tests cover calendar and reading resolution, full-year sweeps, transfers,
Scripture mappings, quotations, invalid data, terminal wrapping and installation.
Passing tests establish software behaviour, not ecclesiastical approval or
complete factual certification of the data.

For the code structure and validation workflow, see [Development](docs/DEVELOPMENT.md).
Corrections should preserve source provenance and keep liturgical rules separate
from factual calendar and reading data.

## Licensing and sources

A project licence has not yet been selected; no root `LICENSE` has been added.
Bundled material retains its own source status and attribution requirements:

- [Bible text and mapping provenance](data/bibles/README.md), including the
  public-domain Scripture source and CC BY 4.0 STEPBible-derived mappings.
- [Calendar definitions and sources](data/calendars/README.md).
- [Lectionary references and source metadata](data/lectionary/README.md).
- [Original summary data and editorial status](data/summaries/README.md).

Cathfetch does not bundle the copyrighted ESV–CE or Abbey Lectionary texts.
