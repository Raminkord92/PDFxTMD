"""Regression test for release version consistency (Python 3.9+)."""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def test_release_versions_agree():
    cmake = (ROOT / "cmake/version.cmake").read_text(encoding="utf-8")
    major = int(re.search(r"set\(PROJECT_VERSION_MAJOR\s+(\d+)\)", cmake).group(1))
    minor = int(re.search(r"set\(PROJECT_VERSION_MINOR\s+(\d+)\)", cmake).group(1))
    patch = int(re.search(r"set\(PROJECT_VERSION_PATCH\s+(\d+)\)", cmake).group(1))
    cmake_version = f"{major}.{minor}.{patch}"

    pyproject = (ROOT / "pyproject.toml").read_text(encoding="utf-8")
    pyproject_version = re.search(r'^version\s*=\s*"([^"]+)"', pyproject, re.M).group(1)

    setup = (ROOT / "setup.py").read_text(encoding="utf-8")
    setup_version = re.search(r'version\s*=\s*"([^"]+)"', setup).group(1)

    manifest_version = json.loads((ROOT / "vcpkg.json").read_text(encoding="utf-8"))["version-string"]

    assert cmake_version == pyproject_version == setup_version == manifest_version
    changelog = (ROOT / "CHANGELOG").read_text(encoding="utf-8")
    assert f"## [{cmake_version}]" in changelog
