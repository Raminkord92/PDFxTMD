#pragma once
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/Interface/SPDF/ICPDF.h"
#include "PDFxTMDLib/Interface/SPDF/ITMD.h"
#include "PDFxTMDLib/Interface/DPDF/ICDPDF.h"
#include "PDFxTMDLib/Interface/IQCDCoupling.h"

namespace PDFxTMD
{
/**
 * @brief Factory class for creating QCD coupling objects
 *
 * This class provides a factory method to create IQCDCoupling objects from a specified
 * PDF set name. It handles the instantiation of the appropriate QCD coupling implementation
 * based on the provided parameters.
 */
class CouplingFactory
{
  public:
    CouplingFactory() = default;
    /**
     * @brief Creates a new IQCDCoupling object
     *
     * @param pdfSetName The name of the PDF set
     * @return IQCDCoupling The newly created IQCDCoupling object
     */
    IQCDCoupling mkCoupling(const std::string &pdfSetName);

  private:
    std::string m_pdfSetName;
};
/**
 * @brief Factory class for creating TMD (Transverse Momentum Dependent) PDF objects
 *
 * This class provides a factory method to create ITMD objects from a specified
 * PDF set name and set member. It handles the instantiation of the appropriate
 * TMD implementation based on the provided parameters. This class is on top of the GenericPDF
 * class. Note: Users should be aware that the list of different TMD or PDF set is read from the
 * yaml file located in C:\ProgramData\PDFxTMDLib\config.yaml (on windows) or
 * ~/.PDFxTMDLib/config.yaml (on linux). here is an example of the config.yaml file: paths:
 * "C:\\Users\\Ramin\\Documents\\code|C:/Users/Ramin/Pictures" This means that the user can have
 * multiple PDF sets in different directories. The paths are separated by |. Also by default the
 * current directory and also C:\ProgramData\PDFxTMDLib\ (on windows) or
 * ~/usr/local/share/PDFxTMDLib/ (on linux) are also included. Note: This class by default uses
 * CDefaultLHAPDFFileReader to read the PDF set. If you want to use a different reader, you modify
 * the yaml info file of the PDF set and adding or modify "Reader" field. For interpolation, you can
 * modify the "Interpolator" field. The default interpolator is TTrilinearInterpolator. For
 * extrapolation, you can modify the "Extrapolator" field. The default extrapolator is
 * TZeroExtrapolator
 */
class GenericTMDFactory
{
  public:
    GenericTMDFactory() = default;
    /**
     * @brief Creates a new ITMD object
     *
     * @param pdfSetName The name of the PDF set
     * @param setMember The member number of the PDF set
     * @return ITMD The newly created ITMD object
     */
    ITMD mkTMD(const std::string &pdfSetName, int setMember);
};
/**
 * @brief Factory class for creating collinear PDF objects
 *
 * This class provides a factory method to create ICPDF objects from a specified
 * PDF set name and set member. It handles the instantiation of the appropriate
 * collinear PDF implementation based on the provided parameters.
 * Note: Users should be aware that the list of different TMD or PDF set is read from the yaml file
 * located in C:\ProgramData\PDFxTMDLib\config.yaml (on windows) or ~/.PDFxTMDLib/config.yaml (on
 * linux). here is an example of the config.yaml file: paths:
 * "C:\\Users\\Ramin\\Documents\\code|C:/Users/Ramin/Pictures" This means that the user can have
 * multiple PDF sets in different directories. The paths are separated by |. Also by default the
 * current directory and also C:\ProgramData\PDFxTMDLib\ (on windows) or
 * ~/usr/local/share/PDFxTMDLib/ (on linux) are also included. Note: This class by default uses
 * CDefaultLHAPDFFileReader to read the PDF set. If you want to use a different reader, you modify
 * the yaml info file of the PDF set and adding or modify "Reader" field. For interpolation, you can
 * modify the "Interpolator" field. The default interpolator is CLHAPDFBilinearInterpolator. For
 * extrapolation, you can modify the "Extrapolator" field. The default extrapolator is
 * CContinuationExtrapolator
 */

class GenericCPDFFactory
{
  public:
    GenericCPDFFactory() = default;
    /**
     * @brief Creates a new ICPDF object
     *
     * @param pdfSetName The name of the PDF set
     * @param setMember The member number of the PDF set
     * @return ICPDF The newly created ICPDF object
     */
    ICPDF mkCPDF(const std::string &pdfSetName, int setMember);
};


class GenericCDPDFFactory
{
  public:
    GenericCDPDFFactory() = default;
    /**
     * @brief Creates a new ICPDF object
     *
     * @param pdfSetName The name of the PDF set
     * @param setMember The member number of the PDF set
     * @return ICPDF The newly created ICPDF object
     */
    ICDPDF mkCDPDF(const std::string &pdfSetName, int setMember);
};
} // namespace PDFxTMD
