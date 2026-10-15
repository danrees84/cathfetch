# Pre-publication review

15 October 2026. Reviewed the repository before its first GitHub publication.
The Git history review covers every commit reachable from local branches and
other refs through `97d7916`: 38 commits, 477 distinct file blobs and 153 trees.
The file blobs total approximately 26 MB across historical versions. The same
patterns were also checked against all 130 current publication files, including
tracked files and non-ignored new files, with no matches.

## Files and history

A local pattern-based scan inspected reachable file contents, commit messages
and paths for private-key headers, common provider tokens, AWS access key IDs,
quoted credential assignments, authenticated URLs and personal home-directory
paths. No matches were found. No credential/configuration directories such as
`.aws/`, `.ssh/`, `.codex/` or `.agents/`, or credential-named files, appeared in
the reachable path inventory. This records the scope and result of the check;
it is not a guarantee that arbitrary secrets are recognisable by those patterns.

The normal Git author/committer name and email remain in commit metadata. There
is one recorded author/committer email identity. Publication of the existing
history includes that metadata. It has not been rewritten or anonymised.

The root `.gitignore` now excludes local credential/assistant directories,
root `.env` variants and Vim swap/backup files. `.env.example` and `.env.sample`
remain eligible for deliberate version control. Existing build-product and
Python-cache exclusions remain in place. None of those local configuration
directories was tracked at review time.

## Documentation reconciled

- The current Lectionary status now reports 807 stored sets, the active universal
  v1 policy, 10,200 assignments across 10,228 tested dates, and 28 Holy Saturdays
  without daytime Mass. Historical proper-aware policies and earlier counts
  are preserved in [archived notes](history/LECTIONARY_AUDIT_HISTORY.md).
- Calendar validation now distinguishes the current full suite from earlier
  focused sanitizer checks. Reading availability is distinct from incomplete
  current-edition reference verification.
- The Bible guide now documents executable-relative data discovery and the
  current four unavailable Tobit correspondences, including the two recovered
  passages. The Lectionary data guide reports the current 807-set count.
- Existing licensing and source-provenance notices remain intact. No reference
  or editorial status was upgraded as part of publication preparation.

This review prepares local files only. Repository creation, remote configuration,
GitHub upload and CI setup are separate steps.

## Checks executed

The application build is current (`make all`). All Markdown local file links
resolve, and 14 ignore-policy cases pass, including the intended `.env` example
exceptions. Both reference-audit freshness checks pass; the summary export check
passes for all 807 sets. Only documentation and ignore rules changed, so the
already passing full application regression suite was not rerun for this review.
