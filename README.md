# Cathfetch

Cathfetch is a small Catholic terminal app, loosely inspired by Fastfetch. It
shows the day's liturgical season and celebration, with optional Mass reading
references, offline Bible text and a short Scripture summary.

It runs offline on Linux. The calendar supports the General Roman Calendar,
Europe, England and Wales. The reading data and summaries are still being
reviewed; see [Current coverage](#current-coverage).

## Build

You need a C++20 compiler, GNU Make, Python 3 and
[nlohmann/json](https://archlinux.org/packages/extra/any/nlohmann-json/).
On Arch Linux:

```sh
sudo pacman -S --needed gcc make python nlohmann-json git
```

From the project directory:

```sh
make -j4 all
./build/cathfetch
```

The build creates a Bible index from the bundled XML. Python is needed for this
step and for the tests; the app itself is C++.

## Usage

```sh
./build/cathfetch --cross
./build/cathfetch --date=08122026 --calendar=england --cross
./build/cathfetch --date=30112025 --readings
./build/cathfetch --date=30112025 --cross --readings --summary
./build/cathfetch --date=25122026 --mass=night --full-verses
```

For example, `--date=30112025 --readings` shows:

```text
Sunday, 30 November 2025
● Advent
First Sunday of Advent

Readings:
Isa 2:1-5; Ps 121:1-2, 4b-5, 6-7, 8-9 r. 1; Rom 13:11-14; Matt 24:37-44
```

| Option | What it does |
| --- | --- |
| `-h`, `--help` | Show help. |
| `--date=DDMMYYYY` | Choose a date. Defaults to today in local time. |
| `--calendar=NAME` | Choose `grc` (default), `europe`, `england` or `wales`. |
| `--cross` | Show the cross. |
| `--readings` | Show Mass reading references. |
| `--summary` | Show a Scripture quotation and commentary. Works with or without `--readings`. |
| `--full-verses` | Show the readings in Douay–Rheims; implies `--readings`. |
| `--mass=FORM` | Choose a Mass or liturgical form; implies `--readings`. |

You can also write values with a space: `--calendar wales`.

Mass forms are `day`, `vigil`, `vigil-extended`, `night`, `dawn`, `evening` and
`passion`, where available. The default is usually daytime Mass. For example,
Christmas has separate Night, Dawn and Day readings. Holy Saturday has no daytime
Mass; `--mass=vigil` selects the Easter Vigil. Good Friday uses `passion` for its
liturgy.

The coloured dots show the season's colour and the celebration's colour. Text
wraps to the terminal width, and the cross moves above it in narrow windows.
Redirected output has no ANSI colours. When no terminal width is available,
Cathfetch uses `COLUMNS`, or 80 columns if it is unset.

## Install

```sh
make install PREFIX="$HOME/.local"
~/.local/bin/cathfetch --summary
```

Add `~/.local/bin` to your `PATH` to run it as `cathfetch`. Data is installed in
`~/.local/share/cathfetch/`.

Both installed and development builds work from any directory. An uninstalled
build needs the project's `build/` and `data/` layout intact. For custom data
locations or packaging with `DESTDIR`, see [Development](docs/DEVELOPMENT.md).

## Current coverage

Cathfetch uses a contemporary calendar snapshot, including movable celebrations,
precedence and transfers. It accepts dates from 1583 to 4099, but does not
reconstruct historical calendars or predict future decrees. Full-year tests cover
2024–2030 in all four calendars, with further boundary years tested separately.
Diocesan calendars and ritual or votive Masses are outside the current scope.
The proposed transfer of St John's Nativity to 23 June 2038 remains provisional.

V1 uses universal readings, retaining local dates for universal feasts such as
Corpus Christi. Memorials use weekday readings; national saints' proper readings
are omitted. The regional celebration shown on screen can therefore differ from
the occasion used to select readings.

There are 807 stored reading sets. Their sources are recorded, but they have not
all been checked against the current England and Wales Lectionary. Psalm
numbering follows the cited source.

Full verses use the 73-book Douay–Rheims American Edition of 1899. Four Tobit
passages cannot be supplied in full because the textual traditions differ.
Partial-verse references show whole verses with a notice.

The summaries were drafted with AI assistance and still need human review.
Quotations are checked against the bundled Bible. All stored reading sets have
summaries; missing content is reported on screen.

For the details:

- [Calendar status](CALENDAR_ENGINE_STATUS.md)
- [Lectionary status](LECTIONARY_ENGINE_STATUS.md)
- [Summary status](SCRIPTURE_SUMMARY_STATUS.md) and [review collection](docs/scripture-summaries/README.md)
- [Bible text and verse numbering](data/bibles/README.md)

## Licence

Cathfetch's original code, documentation and commentary are licensed under
**GPL-3.0-or-later**. See [LICENSE](LICENSE) for the terms and warranty disclaimer.
[LICENSING.md](LICENSING.md) explains the separate terms for bundled material.

Sources and data notes are in the guides for the [calendar](data/calendars/README.md),
[readings](data/lectionary/README.md), [Bible](data/bibles/README.md) and
[summaries](data/summaries/README.md).
