# Tests für changelog.py über die Kommandozeile, wie der Workflow es aufruft.
#
#   python3 .github/scripts/test_changelog.py

import os
import subprocess
import sys
import tempfile
import unittest

SCRIPT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "changelog.py")

CHANGELOG = """# Changelog

Kopftext.

## Unveröffentlicht
- Neu: Etwas.

- Verbessert: Anderes.

## 1.0.0 – 2026-11-02
- Erste Version.
"""


class ChangelogTest(unittest.TestCase):
    def setUp(self):
        fd, self.path = tempfile.mkstemp(suffix=".md")
        os.close(fd)
        self.addCleanup(os.remove, self.path)

    def write(self, text):
        with open(self.path, "w", encoding="utf-8") as f:
            f.write(text)

    def read(self):
        with open(self.path, encoding="utf-8") as f:
            return f.read()

    def run_script(self, *args):
        return subprocess.run(
            [sys.executable, SCRIPT, *args, self.path],
            capture_output=True, text=True, encoding="utf-8",
        )

    def test_notes_gibt_den_unveroeffentlicht_abschnitt_aus(self):
        self.write(CHANGELOG)
        result = self.run_script("notes")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "- Neu: Etwas.\n\n- Verbessert: Anderes.\n")

    def test_notes_macht_relative_links_absolut(self):
        self.write("## Unveröffentlicht\n"
                   "- Siehe [SETUP.md](docs/SETUP.md#auto-align), [Web](https://x.org/a) und [Log](#log).\n")
        result = self.run_script("notes", "--link-base", "https://github.com/o/r/blob/v1.1.0/")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout,
                         "- Siehe [SETUP.md](https://github.com/o/r/blob/v1.1.0/docs/SETUP.md#auto-align),"
                         " [Web](https://x.org/a) und [Log](#log).\n")

    def test_notes_scheitert_ohne_unveroeffentlicht_abschnitt(self):
        self.write("# Changelog\n\n## 1.0.0 – 2026-11-02\n- Erste Version.\n")
        result = self.run_script("notes")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("No '## Unveröffentlicht' section", result.stderr)
        self.assertEqual(result.stdout, "")

    def test_notes_scheitert_bei_leerem_abschnitt(self):
        self.write("# Changelog\n\n## Unveröffentlicht\n\n  \n## 1.0.0 – 2026-11-02\n- Erste Version.\n")
        result = self.run_script("notes")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("'## Unveröffentlicht' section is empty", result.stderr)
        self.assertEqual(result.stdout, "")

    def test_release_benennt_den_abschnitt_um(self):
        self.write(CHANGELOG)
        result = self.run_script("release", "1.1.0", "2026-11-13")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.read(), """# Changelog

Kopftext.

## 1.1.0 – 2026-11-13
- Neu: Etwas.

- Verbessert: Anderes.

## 1.0.0 – 2026-11-02
- Erste Version.
""")


if __name__ == "__main__":
    unittest.main()
