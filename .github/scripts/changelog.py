"""Liest und schreibt den Abschnitt "## Unveröffentlicht" in CHANGELOG.md.

Wird vom Release-Lauf in .github/workflows/build.yml aufgerufen:

  changelog.py notes CHANGELOG.md
      gibt den Text des Abschnitts aus (die Release Notes)
  changelog.py release <Version> <Datum> CHANGELOG.md
      benennt den Abschnitt in "## <Version> – <Datum>" um

Fehlt der Abschnitt, scheitern beide mit Exit-Code 1, notes auch bei leerem
Abschnitt. Der Workflow ruft deshalb notes vor release auf.
"""

import sys

UNRELEASED = "## Unveröffentlicht"


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


def notes(lines):
    start, end = unreleased_section(lines)
    text = "".join(lines[start:end]).strip()
    if not text:
        raise ChangelogError(f"The '{UNRELEASED}' section is empty.")
    return text + "\n"


def release(lines, version, date):
    start, _ = unreleased_section(lines)
    lines[start - 1] = f"## {version} – {date}\n"
    return lines


def main(argv):
    command, path = argv[1], argv[-1]
    with open(path, encoding="utf-8") as f:
        lines = f.readlines()
    try:
        if command == "notes":
            sys.stdout.write(notes(lines))
        elif command == "release":
            lines = release(lines, argv[2], argv[3])
            with open(path, "w", encoding="utf-8") as f:
                f.writelines(lines)
        else:
            raise ChangelogError(__doc__)
    except ChangelogError as e:
        print(e, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
