# Calendar data

These files hold fixed dates, names, ranks and colours. The calendar engine
combines them with movable celebrations and the rules for precedence and
transfers.

The data is a snapshot from 14 October 2026. The same definitions are used for
every accepted year, including dates before these rules came into force.

## Calendar layers

```text
general_roman_fixed.json
    └── europe.json
          └── england_wales.json
                ├── england.json
                └── wales.json
```

Choose `grc`, `europe`, `england` or `wales`. Each layer inherits the preceding
ones. The universal file has 222 fixed celebrations. Europe adds the five feast
dates of its six co-patrons; England and Wales then add their shared and national
entries.

## Fields and changes

Each celebration has a stable ID, month and day, name and rank. All Souls has no
ordinary rank, so its `rank` is null and its precedence is set separately.
In C++, `rankSourceCalendar` records which layer assigned the rank. Moving a
universal solemnity to a national observance date does not make it a proper
solemnity.

| Field | Use |
| --- | --- |
| `additions` | Add a celebration. Its ID must be new. |
| `overrides` / `changes` | Change an inherited entry without changing its ID. |
| `date_reassignments` | Change the nominal date in this calendar. |
| `feast_of_lord` | Identify a general feast of the Lord, at precedence level 5. |
| `precedence` | Set an exceptional precedence, as for Christmas, Epiphany or All Souls. |
| `names_temporal_baseline` | Name an existing temporal day without changing its precedence. |
| `date_rule` | Apply annual scheduling, such as a national Sunday observance. |
| `transfer_rule` | Apply a particular transfer rule when a celebration is impeded. |
| `subject`, `principal_patron` | Resolve Lord/saint solemnity conflicts and preserve patronal exceptions. |
| `colour` | Set the celebration's colour. |
| `source_page`, `amendment`, `source_id`, `rule_source` | Record the source for an entry or rule. |

Colours follow [GIRM 346](https://www.vatican.va/roman_curia/congregations/ccdds/documents/rc_con_ccdds_doc_20030317_ordinamento-messale_en.html).
They are stored explicitly, including white for the Conversion of Paul, Chair
of Peter, St John on 27 December and church dedications.

An overlay is validated before its changes are accepted. Invalid dates,
duplicate additions or change targets, unknown IDs and unknown fields are
rejected. Daily selection still considers competing entries; loading the file
does not decide which celebration wins.

## National entries

Some differences between the supplied calendars:

- St David is a feast in England and a solemnity in Wales; St George is a
  solemnity in England.
- Newman is a national feast under the [2020 decree](https://www.liturgyoffice.org.uk/Calendar/National/October/SJHN-CDW-Decrees.pdf).
  Denis and Leonardi move to 10 October in those calendars. Adalbert moves to
  24 April in England.
- Walsingham is a feast in England under the [2024 decision](https://www.cbcew.org.uk/our-lady-of-walsingham-now-a-feast-in-dioceses-of-england/).
- Elizabeth of Hungary is optional alongside Hilda and Hugh in England, but
  remains obligatory in Wales.
- The Welsh memorials of Asaph, John Jones, Philip Evans and John Lloyd, David
  Lewis, and John Roberts are optional. This follows the
  [national summary](https://www.liturgyoffice.org.uk/Calendar/National/Wales1.shtml)
  and 2026 calendar; older descriptive pages have conflicting labels.

The [universal baseline](https://www.liturgyoffice.org.uk/Calendar/Info/General-Roman-Calendar.pdf)
predates several amendments. Later decrees are recorded in the affected entries.
Keep IDs stable when updating names or designations: Irenaeus becoming a Doctor
of the Church, for example, does not require a new ID.

For movable dates, transfer rules, tested coverage and the provisional 2038
St John transfer, see [Calendar status](../../CALENDAR_ENGINE_STATUS.md).
