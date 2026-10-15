# Licensing

Copyright (C) 2026 Daniel Rees.

Cathfetch's original source code, build scripts, tests, documentation and
original Scripture commentary are distributed under the GNU General Public
License as published by the Free Software Foundation, either version 3 of the
Licence, or (at your option) any later version. The SPDX identifier is
`GPL-3.0-or-later`.

Cathfetch is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See [LICENSE](LICENSE) for the complete terms.

Original code files carry an SPDX notice. The full GPLv3 text also accompanies
installed copies. The choice of later GPL versions is granted by the notices
above and in source files; the GPLv3 licence text itself remains unmodified.

## Separately sourced material

The project licence does not change the status or terms of separately sourced
material, including quotations reproduced in documentation or summary records.

- **Bible text:** the bundled Douay–Rheims American Edition of 1899 is identified
  as public domain by its source, eBible.org. Cathfetch's GPL notice does not
  impose new restrictions on that public-domain text. See the [Bible guide](data/bibles/README.md)
  and [source metadata](data/bibles/en_dra_metadata.json).
- **Verse correspondences:** STEPBible/Tyndale House Cambridge's TVTMS source is
  licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
  Cathfetch's adapted mapping data retains its attribution, exact source
  identity and adaptation notes in the [Bible guide](data/bibles/README.md),
  [metadata](data/bibles/en_dra_metadata.json) and
  [mapping file](data/bibles/en_dra_versification.json). Preserve those notices
  when redistributing that material.
- **Calendar and Lectionary references:** source citations and correction
  evidence are retained in the [calendar data](data/calendars/README.md) and
  [Lectionary data](data/lectionary/README.md). The project licence does not
  assert ownership of factual dates or Scripture references, or grant rights
  to the publishers' underlying liturgical publications.
- **nlohmann/json:** this external header dependency is separately licensed
  under the [MIT licence](licenses/nlohmann-json-MIT.txt).
  A copy of its copyright and permission notice accompanies the repository and
  installed binaries in `licenses/nlohmann-json-MIT.txt`. Preserve these notices
  when redistributing the dependency or copies incorporated into binaries.

The project includes neither the ESV–CE nor Abbey Lectionary texts. Editorial
review and liturgical verification status are separate from licensing and are
recorded in the [summary status document](SCRIPTURE_SUMMARY_STATUS.md).
