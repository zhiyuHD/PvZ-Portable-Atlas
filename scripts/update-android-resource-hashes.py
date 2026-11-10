#!/usr/bin/env python3
"""Update Android's expected hashes from the resource ZIP being released."""
import argparse
import hashlib
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("archive", nargs="?", type=Path, default=ROOT / "pvz-portable-resources.zip")
args = parser.parse_args()

with zipfile.ZipFile(args.archive) as archive:
    names = sorted(entry.filename for entry in archive.infolist() if not entry.is_dir())
    if "main.pak" not in names or not any(name.startswith("properties/") for name in names):
        parser.error("ZIP must contain main.pak and properties/ at its root")
    lines = ["# Expected SHA-256 hashes for this app's resource release."]
    for name in names:
        if name != "main.pak" and not name.startswith("properties/"):
            continue
        if ".." in Path(name).parts or "\\" in name or "\n" in name or "\r" in name:
            parser.error(f"Invalid resource path: {name!r}")
        with archive.open(name) as resource:
            digest = hashlib.file_digest(resource, "sha256").hexdigest()
        lines.append(f"{digest}  {name}")

manifest = ROOT / "android/app/src/main/assets/resource-hashes.sha256"
manifest.parent.mkdir(parents=True, exist_ok=True)
manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"Updated {manifest} ({len(lines) - 1} files)")
