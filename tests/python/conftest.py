"""Small, deterministic on-disk grids read by the real native readers."""
import pytest


@pytest.fixture
def grid_sets(tmp_path, monkeypatch):
    # The library searches the current directory as well as configured paths.
    monkeypatch.chdir(tmp_path)
    for kind, fmt in [("CPDF", "lhagrid1"), ("TMD", "lhagrid_tmd1")]:
        name = "BindingTest" + kind
        folder = tmp_path / name
        folder.mkdir()
        (folder / (name + ".info")).write_text(f"""SetDesc: Python binding test grid
Format: {fmt}
OrderQCD: 1
NumMembers: 3
Flavors: [1, 2, 21]
XMin: 0.001
XMax: 0.5
QMin: 2
QMax: 50
KtMin: 1
KtMax: 10
ErrorType: symmhessian
ErrorConfLevel: 68.26894921370858
AlphaS_Type: ipol
AlphaS_Qs: [2, 5, 10, 20, 50]
AlphaS_Vals: [0.3, 0.25, 0.2, 0.17, 0.13]
""")
        for member, offset in enumerate([0, 1, -2]):
            header = "PdfType: central\nFormat: " + fmt + "\n---\n"
            axes = "0.001 0.01 0.05 0.1 0.5\n"
            if kind == "TMD":
                axes += "1 2 3 5 10\n"
            axes += "2 5 10 20 50\n1 2 21\n"
            # Constant over the grid, with known member variation. TMD's
            # existing convention divides these grid values by kt2.
            row = f"{4 + offset} {6 + offset} {10 + offset}\n"
            count = 125 if kind == "TMD" else 25
            tail = "" if kind == "TMD" else "---\n"
            (folder / f"{name}_{member:04d}.dat").write_text(
                header + axes + row * count + tail
            )
    return "BindingTestCPDF", "BindingTestTMD"
