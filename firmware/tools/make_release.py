"""Prepare GitHub release assets for OTA.

Run after a build:
    python firmware/tools/make_release.py

Reads TL_FW_VERSION from src/core/version.h and the built firmware.bin, and
writes release/v<version>/{firmware.bin,manifest.json}. Upload both files to
a GitHub release whose tag is v<version>. The release must NOT be marked as
a pre-release: devices read .../releases/latest/download/manifest.json, and
"latest" skips pre-releases.
"""
import hashlib
import json
import re
import shutil
import sys
from pathlib import Path

FW_DIR = Path(__file__).resolve().parent.parent
ENV = "esp32c3_supermini"


def main() -> int:
    version_h = (FW_DIR / "src" / "core" / "version.h").read_text(encoding="utf-8")
    m = re.search(r'#define\s+TL_FW_VERSION\s+"([^"]+)"', version_h)
    if not m:
        print("TL_FW_VERSION not found in version.h")
        return 1
    version = m.group(1)
    tag = "v" + version

    binary = FW_DIR / ".pio" / "build" / ENV / "firmware.bin"
    if not binary.exists():
        print(f"{binary} not found — build first")
        return 1
    data = binary.read_bytes()

    out = FW_DIR / "release" / tag
    out.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(binary, out / "firmware.bin")
    manifest = {
        "version": version,
        "tag": tag,
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
    }
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    print(f"Release assets for {tag} in {out}:")
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
