"""Liest und schreibt den Abschnitt "## Unveröffentlicht" in CHANGELOG.md.

Wird vom Release-Lauf in .github/workflows/build.yml aufgerufen:

  changelog.py notes [--footer DATEI] [--link-base URL] CHANGELOG.md
      gibt den Text des Abschnitts aus (die Release Notes), mit --footer
      gefolgt vom Inhalt der Datei; relative Links werden mit --link-base
      absolut, damit sie im GitHub-Release funktionieren
  changelog.py release <Version> <Datum> CHANGELOG.md
      benennt den Abschnitt in "## <Version> – <Datum>" um

Fehlt der Abschnitt, scheitern beide mit Exit-Code 1, notes auch bei leerem
Abschnitt. Der Workflow ruft deshalb notes vor release auf.
"""

import argparse
import re
import sys

UNRELEASED = "## Unveröffentlicht"

# Markdown-Link mit relativem Ziel: kein Schema (http:, mailto:), kein Anker
# auf derselben Seite, kein absoluter Pfad
RELATIVE_LINK = re.compile(r"\]\((?![a-zA-Z][a-zA-Z0-9+.-]*:|#|/)")


class ChangelogError(Exception):
    pass


def unreleased_section(lines):
    """Zeilenbereich [start, end) des Abschnittstexts unter der Überschrift."""
    try:
        start = lines.index(UNRELEASED + "\n") + 1
    except ValueError:
        raise ChangelogError(f"No '{UNRELEASED}' section in the changelog.")
    end = start
    while end < len(lines) and not lines[end].startswith("## "):
        end += 1
    return start, end


def notes(lines, link_base=None, footer=None):
    start, end = unreleased_section(lines)
    text = "".join(lines[start:end]).strip()
    if not text:
        raise ChangelogError(f"The '{UNRELEASED}' section is empty.")
    if footer:
        text += "\n\n" + footer.strip()
    if link_base:
        text = RELATIVE_LINK.sub(lambda m: "](" + link_base, text)
    return text + "\n"


def release(lines, version, date):
    """Benennt den Abschnitt in lines um (in place)."""
    start, _ = unreleased_section(lines)
    lines[start - 1] = f"## {version} – {date}\n"


def main():
    parser = argparse.ArgumentParser(description="Read or release the unreleased changelog section.")
    commands = parser.add_subparsers(dest="command", required=True)
    notes_cmd = commands.add_parser("notes", help="print the section text")
    notes_cmd.add_argument("--footer", help="file appended after the section text")
    notes_cmd.add_argument("--link-base", help="prefix for relative links, e.g. https://github.com/o/r/blob/v1.2.0/")
    release_cmd = commands.add_parser("release", help="rename the section to '## <version> – <date>'")
    release_cmd.add_argument("version")
    release_cmd.add_argument("date")
    for cmd in (notes_cmd, release_cmd):
        cmd.add_argument("path")
    args = parser.parse_args()

    with open(args.path, encoding="utf-8") as f:
        lines = f.readlines()
    try:
        if args.command == "notes":
            footer = None
            if args.footer:
                with open(args.footer, encoding="utf-8") as f:
                    footer = f.read()
            sys.stdout.write(notes(lines, args.link_base, footer))
        else:
            release(lines, args.version, args.date)
            with open(args.path, "w", encoding="utf-8") as f:
                f.writelines(lines)
    except ChangelogError as e:
        print(e, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
