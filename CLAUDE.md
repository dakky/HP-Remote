## Agent skills

### Issue tracker

GitHub Issues in dakky/HP-Remote; pro Feature ein Epic mit Sub-Issues. See `docs/agents/issue-tracker.md`.

### Triage labels

Standard-Labels (needs-triage, needs-info, ready-for-agent, ready-for-human, wontfix). See `docs/agents/triage-labels.md`.

### Domain docs

Single-context. See `docs/agents/domain.md`.

## Branches

`main` ist Entwicklungs- und Release-Branch. Feature-Branches (`feature/…`, `fix/…`, `chore/…`) gehen von `main` ab und werden nach `main` gemergt. Releases baut der Workflow `build.yml` per Run workflow aus `main`.
