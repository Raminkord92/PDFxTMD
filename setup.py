from __future__ import annotations

import os
import platform
import shutil
import shlex
import subprocess
import sys
from pathlib import Path

import pybind11
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    """A setuptools extension whose sources are built by CMake."""

    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = str(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    """Build the Python module through PDFxTMD's native CMake targets.

    This deliberately avoids linking libPDFxTMDLib.a by hand. CMake remains
    the single source of truth for native dependencies such as Zstandard,
    oneDNN, and OpenMP.
    """

    def build_extension(self, ext: CMakeExtension) -> None:
        ext_fullpath = Path(self.get_ext_fullpath(ext.name)).resolve()
        extdir = ext_fullpath.parent
        cfg = "Debug" if self.debug else "Release"
        build_temp = Path(self.build_temp) / ext.name
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir.mkdir(parents=True, exist_ok=True)

        enable_dpd = os.environ.get("PDFXTMD_ENABLE_DPD", "ON").strip().upper()
        if enable_dpd in {"1", "TRUE", "YES", "ON"}:
            enable_dpd = "ON"
        elif enable_dpd in {"0", "FALSE", "NO", "OFF"}:
            enable_dpd = "OFF"
        else:
            raise RuntimeError(
                "PDFXTMD_ENABLE_DPD must be one of ON/OFF, TRUE/FALSE, YES/NO, or 1/0"
            )

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DPDFXTMD_PYTHON_OUTPUT_DIR={extdir}",
            f"-DPython3_EXECUTABLE={sys.executable}",
            f"-Dpybind11_DIR={pybind11.get_cmake_dir()}",
            "-DENABLE_BUILDING_WRAPPERS=ON",
            "-DENABLE_BUILDING_EXAMPLES=OFF",
            "-DENABLE_DOCUMENTATION=OFF",
            "-DENABLE_FORMATTING=OFF",
            f"-DENABLE_DPD={enable_dpd}",
        ]

        # Let users pass normal CMake settings, including a vcpkg toolchain:
        #   CMAKE_ARGS='-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake' pip install .
        extra_cmake_args = shlex.split(os.environ.get("CMAKE_ARGS", ""))
        cmake_args.extend(extra_cmake_args)

        # GitHub Actions exposes vcpkg through VCPKG_ROOT. Add its toolchain
        # as a subprocess argument rather than interpolating a Windows path
        # through cibuildwheel's shell environment parser.
        has_toolchain = any(
            argument.startswith("-DCMAKE_TOOLCHAIN_FILE=")
            for argument in extra_cmake_args
        )
        vcpkg_root = os.environ.get("VCPKG_ROOT")
        if not has_toolchain and vcpkg_root:
            toolchain = Path(vcpkg_root) / "scripts" / "buildsystems" / "vcpkg.cmake"
            if toolchain.is_file():
                cmake_args.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")

        vcpkg_triplet = os.environ.get("VCPKG_TARGET_TRIPLET")
        if vcpkg_triplet:
            cmake_args.append(f"-DVCPKG_TARGET_TRIPLET={vcpkg_triplet}")

        if sys.platform == "darwin":
            macos_target = os.environ.get("MACOSX_DEPLOYMENT_TARGET")
            if macos_target:
                cmake_args.append(f"-DCMAKE_OSX_DEPLOYMENT_TARGET={macos_target}")

        generator = os.environ.get("CMAKE_GENERATOR", "")
        cmake_args.extend(
            [
                f"-DCMAKE_BUILD_TYPE={cfg}",
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}{os.sep}",
                f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}{os.sep}",
            ]
        )

        # If no generator was chosen explicitly, CMake may select Visual Studio
        # on Windows. Match that generator to the active Python architecture.
        if sys.platform == "win32" and not generator:
            machine = platform.machine().lower()
            if machine in {"amd64", "x86_64"}:
                cmake_args.extend(["-A", "x64"])
            elif machine in {"x86", "i386", "i686"}:
                cmake_args.extend(["-A", "Win32"])
            elif machine in {"arm64", "aarch64"}:
                cmake_args.extend(["-A", "ARM64"])

        subprocess.check_call(
            ["cmake", "-S", ext.sourcedir, "-B", str(build_temp), *cmake_args]
        )

        build_args = ["--config", cfg, "--target", "pdfxtmd"]
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            build_args.extend(["--parallel", str(os.cpu_count() or 2)])

        subprocess.check_call(["cmake", "--build", str(build_temp), *build_args])

        # Never let setuptools create a wheel that silently omits the compiled
        # Python extension. Multi-config generators such as Visual Studio can
        # otherwise place the .pyd in a configuration subdirectory.
        if not ext_fullpath.is_file():
            patterns = ("pdfxtmd*.pyd",) if sys.platform == "win32" else ("pdfxtmd*.so",)
            candidates = [
                candidate
                for root in (extdir, build_temp)
                for pattern in patterns
                for candidate in root.rglob(pattern)
                if candidate.is_file()
            ]

            if not candidates:
                raise RuntimeError(
                    "CMake completed successfully, but the Python extension "
                    f"was not found. Expected: {ext_fullpath}"
                )

            source = max(candidates, key=lambda path: path.stat().st_mtime)
            print(f"Copying Python extension from {source} to {ext_fullpath}")
            ext_fullpath.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, ext_fullpath)

        if not ext_fullpath.is_file():
            raise RuntimeError(
                f"Python extension was not generated at {ext_fullpath}"
            )

        print(f"Python extension ready: {ext_fullpath}")


setup(
    name="pdfxtmd",
    version="2.0.1",
    author="Ramin Kord Valeshabadi",
    author_email="raminkord92@gmail.com",
    description=(
        "PDFxTMD is a library for collinear, transverse-momentum-dependent, "
        "and double parton distributions."
    ),
    url="https://github.com/Raminkord92/PDFxTMD",
    long_description=Path("examples/python/readme-pyversion.md").read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    license="GPL-3.0",
    platforms=["any"],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.9",
    ext_modules=[CMakeExtension("pdfxtmd", sourcedir=".")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
)
