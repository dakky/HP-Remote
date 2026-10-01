Import("env")
import datetime
import os
import re
import subprocess

project_dir = env.subst("$PROJECT_DIR")
version_file = os.path.join(project_dir, "src", "version.txt")

def git_output(*args):
    try:
        return subprocess.check_output(
            ["git", *args], cwd=project_dir, text=True, stderr=subprocess.DEVNULL
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def legacy_base_version():
    try:
        with open(version_file, "r") as version_handle:
            return version_handle.read().strip()
    except OSError:
        return "0.0.0"


def release_version(tag):
    match = re.fullmatch(r"v(\d+)\.(\d+)\.(\d+)", tag)
    if not match:
        return ""
    return ".".join(match.groups())


def next_patch_version(tag):
    match = re.fullmatch(r"v(\d+)\.(\d+)\.(\d+)", tag)
    if not match:
        return ""
    major, minor, patch = (int(part) for part in match.groups())
    return f"{major}.{minor}.{patch + 1}"


def build_metadata():
    tag_pattern = "v[0-9]*.[0-9]*.[0-9]*"
    tag = git_output("describe", "--tags", "--exact-match", "--match", tag_pattern)
    commit = git_output("rev-parse", "--short=7", "HEAD") or "unknown"
    dirty = bool(git_output("status", "--porcelain"))

    version = release_version(tag)
    if version:
        if dirty:
            version += "-dirty"
        return version, commit

    latest_tag = git_output("describe", "--tags", "--abbrev=0", "--match", tag_pattern)
    next_version = next_patch_version(latest_tag)
    version = f"{next_version or legacy_base_version()}-snapshot.{commit}"
    if dirty:
        version += ".dirty"
    return version, commit


version, commit = build_metadata()
source_date_epoch = os.environ.get("SOURCE_DATE_EPOCH") or git_output("log", "-1", "--format=%ct")
try:
    build_time = datetime.datetime.fromtimestamp(
        int(source_date_epoch), datetime.timezone.utc
    ).strftime("%Y-%m-%dT%H:%M:%SZ")
except (TypeError, ValueError, OverflowError):
    build_time = "unknown"

print(f"HP-Remote firmware version: {version} (commit {commit}, build {build_time})")

# Die Metadaten im generierten Header sind für lokale und CI-Builds identisch,
# sofern sie denselben Commit und SOURCE_DATE_EPOCH verwenden.
header_path = os.path.join(project_dir, "src", "build_info.h")
header_content = (
    "#pragma once\n"
    "// AUTO-GENERIERT von version_build.py – nicht manuell editieren!\n"
    f'#define FW_VERSION "{version}"\n'
    f'#define FW_GIT_COMMIT "{commit}"\n'
    f'#define FW_BUILD "{build_time}"\n'
)

# Nur schreiben, wenn sich der Inhalt geändert hat.
old = ""
if os.path.exists(header_path):
    with open(header_path, "r") as f:
        old = f.read()

if old != header_content:
    with open(header_path, "w") as f:
        f.write(header_content)
    print(f"build_info.h aktualisiert: {version} / {commit} / {build_time}")