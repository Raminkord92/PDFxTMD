"""
PDFxTMDLib Python Tutorial (Complete)

This tutorial demonstrates how to use all exposed methods of the PDFxTMD Python bindings.
It covers `PDFSet` interface for uncertainty and correlation analysis,
and the low-level factory interface for direct access to individual PDF members.

Installation:
    pip install pdfxtmd
"""
import pdfxtmd

def main():
    # --- High-Level Interface: PDFSet for Uncertainty Analysis (Recommended) ---

    print("--- Using CPDFSet for Collinear PDF Uncertainty ---")
    # 1. Create a CPDFSet object for a given PDF set name.
    #    This object manages all members of the set.
    cpdfSetName = "CT18NLO"
    cpdf_set = pdfxtmd.CPDFSet(cpdfSetName)
    print(f"Loaded CPDF Set: CT18NLO with {len(cpdf_set)} members.")

    # 2. Accessing Metadata
    print("\n--- Accessing Metadata from CPDFSet ---")
    std_info = cpdf_set.getStdPDFInfo()
    err_info = cpdf_set.getPDFErrorInfo()
    print(f"Description: {std_info.SetDesc}")
    print(f"NumMembers: {std_info.NumMembers}")
    print(f"Error Type: {err_info.ErrorType}")
    print(f"Error Confidence Level: {err_info.ErrorConfLevel}%")
    print(f"Q range: [{std_info.QMin}, {std_info.QMax}]")

    # 2. Define kinematics
    x = 0.01
    mu2 = 100

    # 3. Get the central value PDF (member 0) using the index operator
    central_cpdf = cpdf_set[0]
    up_pdf_central = central_cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2)
    print(f"Central Up Quark PDF at x={x}, mu2={mu2}: {up_pdf_central:.6f}")

    # 4. Calculate PDF uncertainty (e.g., at 68% CL)
    #    The result is a PDFUncertainty object.
    uncertainty_cpdf = cpdf_set.Uncertainty(pdfxtmd.PartonFlavor.u, x, mu2, cl=68.0)
    print(f"Uncertainty (Up Quark):")
    print(f"  Central Value: {uncertainty_cpdf.central:.6f}")
    print(f"  errplus: +{uncertainty_cpdf.errplus:.6f}")
    print(f"  errminus: -{uncertainty_cpdf.errminus:.6f}")
    print(f"  errsymm: +/-{uncertainty_cpdf.errsymm:.6f}")

    # 5. Calculate correlation between two different PDFs
    corr = cpdf_set.Correlation(
        pdfxtmd.PartonFlavor.u, x, mu2,  # PDF A
        pdfxtmd.PartonFlavor.d, x, mu2   # PDF B
    )
    print(f"Correlation between u and d quarks at same kinematics: {corr:.4f}\n")


    print("--- Using TMDSet for TMD PDF Uncertainty ---")
    # 6. Create a TMDSet object.
    tmd_set_name = "PB-NLO-HERAI+II-2018-set2"
    tmd_set = pdfxtmd.TMDSet(tmd_set_name)
    print(f"Loaded TMD Set: {tmd_set_name} with {len(tmd_set)} members.")

    # 7. Define TMD kinematics
    kt2 = 10

    # 8. Get the central value TMD
    central_tmd = tmd_set[0]
    gluon_tmd_central = central_tmd.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2)
    print(f"Central Gluon TMD at x={x}, kt2={kt2}, mu2={mu2}: {gluon_tmd_central:.6f}")

    # 9. Calculate TMD uncertainty
    uncertainty_tmd = tmd_set.Uncertainty(pdfxtmd.PartonFlavor.g, x, kt2, mu2, cl=68.0)
    print(f"Uncertainty (Gluon TMD):")
    print(f"  Central Value: {uncertainty_tmd.central:.6f}")
    print(f"  errplus: +{uncertainty_tmd.errplus:.6f}")
    print(f"  errminus: -{uncertainty_tmd.errminus:.6f}\n")


    # --- Low-Level Interface: Factories for Single PDF Members ---
    print("\n--- Low-Level Interface: Factories for Single PDF Members ---")
    
    print("\n--- Collinear PDFs (CPDF) using Factory ---")
    # 1. Create a CPDF factory and a single CPDF object
    cpdf_factory = pdfxtmd.GenericCPDFFactory()
    cpdf_member = cpdf_factory.mkCPDF("CT18NLO", 0)

    # 2. Evaluate a single flavor
    print("Up quark PDF:", cpdf_member.pdf(pdfxtmd.PartonFlavor.u, x, mu2))
    print("Gluon PDF:", cpdf_member.pdf(pdfxtmd.PartonFlavor.g, x, mu2))

    # 3. Evaluate all flavors at once (returns a list)
    all_flavors_cpdf = cpdf_member.pdf_all(x, mu2)
    print("All flavors (CPDF):", all_flavors_cpdf)


    print("\n--- TMD PDFs using Factory ---")
    # 4. Create a TMD factory and a single TMD object
    tmd_factory = pdfxtmd.GenericTMDFactory()
    tmd_member = tmd_factory.mkTMD(tmd_set_name, 0)

    # 5. Evaluate a single flavor TMD
    print("Gluon TMD:", tmd_member.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2))
    print("Up quark TMD:", tmd_member.tmd(pdfxtmd.PartonFlavor.u, x, kt2, mu2))

    # 6. Evaluate all TMD flavors at once
    all_flavors_tmd = tmd_member.tmd_all(x, kt2, mu2)
    print("All flavors (TMD):", all_flavors_tmd)


    # --- QCD Coupling (Two Ways) ---
    print("\n--- QCD Coupling (alpha_s) ---")
    # Method A: Directly from the PDFSet object (recommended)
    print("Method A: From CPDFSet")
    for scale in [10, 100, 1000, 10000]:
        alpha_s_from_set = cpdf_set.alphasQ2(scale)
        print(f"Alpha_s at mu2={scale}: {alpha_s_from_set:.5f}")

    # Method B: Using the low-level factory
    print("\nMethod B: From CouplingFactory")
    coupling_factory = pdfxtmd.CouplingFactory()
    coupling = coupling_factory.mkCoupling(cpdfSetName)
    for scale in [10, 100, 1000, 10000]:
        alpha_s_from_factory = coupling.AlphaQCDMu2(scale)
        print(f"Alpha_s at mu2={scale}: {alpha_s_from_factory:.5f}")


    # --- Error Handling Examples ---
    print("\n--- Error Handling Examples ---")
    try:
        # Invalid x (should raise an error)
        cpdf_member.pdf(pdfxtmd.PartonFlavor.u, -0.1, mu2)
    except Exception as e:
        print(f"Caught expected error for invalid x in CPDF: {e}")

    try:
        # Invalid kt2 (should raise an error)
        tmd_member.tmd(pdfxtmd.PartonFlavor.g, x, -5, mu2)
    except Exception as e:
        print(f"Caught expected error for invalid kt2 in TMD: {e}")

    try:
        # Invalid mu2 (should raise an error)
        coupling.AlphaQCDMu2(-1)
    except Exception as e:
        print(f"Caught expected error for invalid mu2 in Coupling: {e}")


    # --- Enumerate all PartonFlavor values ---
    print("\n--- All PartonFlavor enum values ---")
    for name, flavor in pdfxtmd.PartonFlavor.__members__.items():
        print(f"  {name}: {flavor.value}")

if __name__ == "__main__":
    main()
