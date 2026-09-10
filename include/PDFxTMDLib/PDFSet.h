/// @file PDFSet.h
/// @brief This file contains the declaration of the PDFSet class.
/// This class and all uncertainties related features are based on the LHAPDF library, but is
/// heavily refactored to improve its readibility and even its performance!
#pragma once
#include "PDFxTMDLib/Common/Uncertainty.h"
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/Interface/SPDF/ICPDF.h"
#include "PDFxTMDLib/Interface/SPDF/ITMD.h"
#include <PDFxTMDLib/Common/Exception.h>
#include <PDFxTMDLib/Common/MathUtils.h>
#include <PDFxTMDLib/Common/PDFErrInfo.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h>
#include <PDFxTMDLib/Factory.h>
#include <PDFxTMDLib/Interface/IQCDCoupling.h>
#include <PDFxTMDLib/Implementation/Coupling/Null/NullQCDCoupling.h>
#include <PDFxTMDLib/Interface/IUncertainty.h>
#include <PDFxTMDLib/Uncertainty/HessianStrategy.h>
#include <PDFxTMDLib/Uncertainty/NullUncertaintyStrategy.h>
#include <PDFxTMDLib/Uncertainty/ReplicasPercentileStrategy.h>
#include <PDFxTMDLib/Uncertainty/ReplicasStdDevStrategy.h>
#include <PDFxTMDLib/Uncertainty/SymmHessianStrategy.h>
#include <PDFxTMDLib/Common/Logger.h>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

namespace PDFxTMD
{
/// @brief A type trait to map a tag to a specific PDF interface type.
/// @tparam T The tag (e.g., TMDPDFTag, CollinearPDFTag).
template <typename T> struct PDFType;

/// @brief Specialization of PDFType for Transverse Momentum Dependent (TMD) PDFs.
template <> struct PDFType<TMDPDFTag>
{
    using type = ITMD;
};

/// @brief Specialization of PDFType for Collinear PDFs.
template <> struct PDFType<CollinearPDFTag>
{
    using type = ICPDF;
};

/**
 * @class PDFSet
 * @brief Manages a set of Parton Distribution Functions (PDFs), providing tools for uncertainty and correlation analysis.
 *
 * This class acts as a container for all members of a specific PDF set (e.g., CT18, PB-NLO). It handles the
 * loading of PDF data, calculation of uncertainties using various methods (replicas, Hessian), and evaluation of
 * correlations between PDF values. It is a templated class that can be specialized for either TMD or Collinear PDFs.
 *
 * @tparam Tag A tag to specify the PDF type (TMDPDFTag or CollinearPDFTag).
 */
template <typename Tag> class PDFSet
{
  public:
    /// @brief The specific PDF interface type (ITMD or ICPDF) determined by the Tag.
    using PDF_t = typename PDFType<Tag>::type;

    /**
     * @brief Constructs a PDFSet for a given PDF name.
     * @param pdfSetName The name of the PDF set to load.
     * @param alternativeReplicaUncertainty If true, use percentile strategy for replica uncertainties; otherwise, use standard deviation.
     */
    explicit PDFSet(std::string pdfSetName, bool alternativeReplicaUncertainty = false)
        : m_pdfSetName(std::move(pdfSetName)),
          m_alternativeReplicaUncertainty(alternativeReplicaUncertainty),
          m_uncertaintyStrategy_(NullUncertaintyStrategy()),
          m_qcdCoupling(CouplingFactory().mkCoupling(m_pdfSetName))
    {
        Initialize();
        CreateAllPDFSets();
    }

    /// @brief Default constructor.
    PDFSet():m_uncertaintyStrategy_(NullUncertaintyStrategy()), m_qcdCoupling(NullQCDCoupling()){};

    /**
     * @brief Get the strong coupling constant alpha_s at a given scale Q.
     * @param q The momentum transfer scale Q in GeV.
     * @return The value of alpha_s(Q).
     */
    double alphasQ(double q) const
    {
        return alphasQ2(q * q);
    }

    /**
     * @brief Get the strong coupling constant alpha_s at a given squared scale Q^2.
     * @param q2 The squared momentum transfer scale Q^2 in GeV^2.
     * @return The value of alpha_s(Q^2).
     */
    double alphasQ2(double q2) const
    {
        return m_qcdCoupling.AlphaQCDMu2(q2);
    }

    /**
     * @brief Access a specific PDF member from the set.
     * @param member The member index (0 is the central value).
     * @return A pointer to the PDF object. Creates the object if it doesn't exist.
     */
    PDF_t *operator[](int member)
    {
        std::lock_guard<std::mutex> lock(m_pdfSetMtx);
        if (m_PDFSet_.find(member) == m_PDFSet_.end())
        {
            CreatePDFSet(member);
        }
        return m_PDFSet_[member].get();
    }

    /**
     * @brief Access a specific PDF member from the set (const version).
     * @param member The member index.
     * @return A const pointer to the PDF object, or nullptr if it has not been created.
     */
    PDF_t *operator[](int member) const
    {
        if (m_PDFSet_.find(member) == m_PDFSet_.end())
            return nullptr;
        return m_PDFSet_.at(member).get();
    }

    /**
     * @brief Calculate the TMD uncertainty. (Enabled only for TMDPDFTag)
     * @param flavor The parton flavor.
     * @param x The momentum fraction.
     * @param kt2 The squared transverse momentum.
     * @param mu2 The squared factorization scale.
     * @param cl The desired confidence level in percent.
     * @param resUncertainty The output PDFUncertainty object.
     */
    template <typename T = Tag, typename = std::enable_if_t<std::is_same_v<T, TMDPDFTag>>>
    void Uncertainty(PartonFlavor flavor, double x, double kt2, double mu2, double cl,
                     PDFUncertainty &resUncertainty)
    {
        const auto pdfs = CalculatePDFValues(flavor, x, kt2, mu2);
        PDFUncertaintyInternalEvaluation(pdfs, cl, resUncertainty);
    }

    /**
     * @brief Calculate the collinear PDF uncertainty. (Enabled only for CollinearPDFTag)
     * @param flavor The parton flavor.
     * @param x The momentum fraction.
     * @param mu2 The squared factorization scale.
     * @param cl The desired confidence level in percent.
     * @param resUncertainty The output PDFUncertainty object.
     */
    template <typename T = Tag, typename = std::enable_if_t<std::is_same_v<T, CollinearPDFTag>>>
    void Uncertainty(PartonFlavor flavor, double x, double mu2, double cl,
                     PDFUncertainty &resUncertainty)
    {
        const auto pdfs = CalculatePDFValues(flavor, x, mu2);
        PDFUncertaintyInternalEvaluation(pdfs, cl, resUncertainty);
    }
    
    /**
     * @brief Calculate the TMD uncertainty and return the result. (Enabled only for TMDPDFTag)
     * @param flavor The parton flavor.
     * @param x The momentum fraction.
     * @param kt2 The squared transverse momentum.
     * @param mu2 The squared factorization scale.
     * @param cl The desired confidence level in percent (default is the set's native CL).
     * @return A PDFUncertainty object with the results.
     */
    template <typename T = Tag, typename = std::enable_if_t<std::is_same_v<T, TMDPDFTag>>>
    PDFUncertainty Uncertainty(PartonFlavor flavor, double x, double kt2, double mu2,
                               double cl = NO_REQUESTED_CONFIDENCE_LEVEL)
    {
        PDFUncertainty resUncertainty;
        const auto pdfs = CalculatePDFValues(flavor, x, kt2, mu2);
        PDFUncertaintyInternalEvaluation(pdfs, cl, resUncertainty);
        return resUncertainty;
    }
    
    /**
     * @brief Calculate the collinear PDF uncertainty and return the result. (Enabled only for CollinearPDFTag)
     * @param flavor The parton flavor.
     * @param x The momentum fraction.
     * @param mu2 The squared factorization scale.
     * @param cl The desired confidence level in percent (default is the set's native CL).
     * @return A PDFUncertainty object with the results.
     */
    template <typename T = Tag, typename = std::enable_if_t<std::is_same_v<T, CollinearPDFTag>>>
    PDFUncertainty Uncertainty(PartonFlavor flavor, double x, double mu2,
                               double cl = NO_REQUESTED_CONFIDENCE_LEVEL)
    {
        PDFUncertainty resUncertainty;
        const auto pdfs = CalculatePDFValues(flavor, x, mu2);
        PDFUncertaintyInternalEvaluation(pdfs, cl, resUncertainty);
        return resUncertainty;
    }
    
    /**
     * @brief Calculate uncertainty from a pre-computed vector of PDF values.
     * @param values A vector of PDF values from all members of the set.
     * @param cl The desired confidence level in percent.
     * @param resUncertainty The output PDFUncertainty object.
     */
    void Uncertainty(const std::vector<double> &values, double cl, PDFUncertainty &resUncertainty)
    {
        if (values.size() != m_pdfSetErrorInfo.size)
            throw InvalidInputError("Error in PDFxTMD::PDFSet::Uncertainty. Input vector must "
                                    "contain values for all PDF members.");
        PDFUncertaintyInternalEvaluation(values, cl, resUncertainty);
    }
    
    /**
     * @brief Calculate uncertainty from a pre-computed vector of PDF values and return the result.
     * @param values A vector of PDF values from all members of the set.
     * @param cl The desired confidence level in percent (default is the set's native CL).
     * @return A PDFUncertainty object.
     */
    PDFUncertainty Uncertainty(const std::vector<double> &values,
                               double cl = NO_REQUESTED_CONFIDENCE_LEVEL)
    {
        if (values.size() != m_pdfSetErrorInfo.size)
            throw InvalidInputError("Error in PDFxTMD::PDFSet::Uncertainty. Input vector must "
                                    "contain values for all PDF members.");
        PDFUncertainty resUncertainty;
        PDFUncertaintyInternalEvaluation(values, cl, resUncertainty);
        return resUncertainty;
    }
    
    /**
     * @brief Calculate the correlation for collinear PDFs. (Enabled only for CollinearPDFTag)
     * @param flavorA The parton flavor for the first observable.
     * @param xA The momentum fraction 'x' for the first observable.
     * @param mu2A The scale 'mu2' for the first observable.
     * @param flavorB The parton flavor for the second observable.
     * @param xB The momentum fraction 'x' for the second observable.
     * @param mu2B The scale 'mu2' for the second observable.
     * @return The correlation coefficient.
     */
    template <typename T = Tag, typename = std::enable_if_t<std::is_same_v<T, CollinearPDFTag>>>
    double Correlation(PartonFlavor flavorA, double xA, double mu2A, PartonFlavor flavorB,
                       double xB, double mu2B)
    {
        const auto pdfsA = CalculatePDFValues(flavorA, xA, mu2A);
        const auto pdfsB = CalculatePDFValues(flavorB, xB, mu2B);
        return m_uncertaintyStrategy_.Correlation(pdfsA, pdfsB, m_pdfErrInfo.nmemCore());
    }

    /**
     * @brief Calculate the correlation for TMDs. (Enabled only for TMDPDFTag)
     * @param flavorA The parton flavor for the first TMD.
     * @param xA The momentum fraction 'x' for the first TMD.
     * @param kt2A The transverse momentum 'kt2' for the first TMD.
     * @param mu2A The scale 'mu2' for the first TMD.
     * @param flavorB The parton flavor for the second TMD.
     * @param xB The momentum fraction 'x' for the second TMD.
     * @param kt2B The transverse momentum 'kt2' for the second TMD.
     * @param mu2B The scale 'mu2' for the second TMD.
     * @return The correlation coefficient.
     */
    template <typename T = Tag, typename = std::enable_if_t<std::is_same_v<T, TMDPDFTag>>>
    double Correlation(PartonFlavor flavorA, double xA, double kt2A, double mu2A,
                       PartonFlavor flavorB, double xB, double kt2B, double mu2B)
    {
        const auto pdfsA = CalculatePDFValues(flavorA, xA, kt2A, mu2A);
        const auto pdfsB = CalculatePDFValues(flavorB, xB, kt2B, mu2B);
        return m_uncertaintyStrategy_.Correlation(pdfsA, pdfsB, m_pdfErrInfo.nmemCore());
    }

    /**
     * @brief Calculate correlation from two pre-computed vectors of PDF values.
     * @param valuesA A vector of PDF values for the first observable.
     * @param valuesB A vector of PDF values for the second observable.
     * @return The correlation coefficient.
     */
    double Correlation(const std::vector<double> &valuesA, const std::vector<double> &valuesB) const
    {
        if (valuesA.size() != m_pdfSetErrorInfo.size || valuesB.size() != m_pdfSetErrorInfo.size)
            throw InvalidInputError("Error in PDFxTMD::PDFSet::Correlation. Input vectors must "
                                    "contain values for all PDF members.");
        return m_uncertaintyStrategy_.Correlation(valuesA, valuesB, m_pdfErrInfo.nmemCore());
    }

    /**
     * @brief Explicitly creates a single PDF member.
     * @param setMember The index of the PDF member to create.
     */
    void CreatePDFSet(unsigned int setMember)
    {
        {
            if constexpr (std::is_same_v<Tag, TMDPDFTag>)
            {
                m_PDFSet_.insert_or_assign(
                    setMember, std::make_unique<PDF_t>(
                                   PDFxTMD::GenericTMDFactory().mkTMD(m_pdfSetName, setMember)));
            }
            else if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
            {
                m_PDFSet_.insert_or_assign(
                    setMember, std::make_unique<PDF_t>(
                                   PDFxTMD::GenericCPDFFactory().mkCPDF(m_pdfSetName, setMember)));
            }
            else
            {
                static_assert(!std::is_same_v<Tag, Tag>, "Unsupported Tag");
            }
        }
    }
    
    /**
     * @brief Pre-loads all PDF members in the set.
     */
    void CreateAllPDFSets()
    {
        for (int i = 0; i < m_pdfSetStdInfo.NumMembers; ++i)
        {
            if (m_PDFSet_.find(i) == m_PDFSet_.end()) {
                CreatePDFSet(i);
            }
        }
    }
    
    /**
     * @brief Re-initializes the PDFSet with a new PDF set name.
     * @param pdfSetName The name of the new PDF set.
     */
    void InitailizePDFSetName(std::string pdfSetName)
    {
        m_pdfSetName = std::move(pdfSetName);
        Initialize();
    }
    
    /**
     * @brief Get the total number of members in this PDF set.
     * @return The number of members.
     */
    size_t size() const
    {
        return m_pdfSetStdInfo.NumMembers;
    }

    /**
     * @brief Get the standard metadata for the PDF set.
     * @return A YamlStandardPDFInfo object.
     */
    YamlStandardTMDInfo getStdPDFInfo() const
    {
        return m_pdfSetStdInfo;
    }

    /**
     * @brief Get the error metadata for the PDF set.
     * @return A YamlErrorInfo object.
     */
    YamlErrorInfo getPDFErrorInfo() const
    {
        return m_pdfSetErrorInfo;
    }

    /**
     * @brief Get the full configuration metadata object for the set.
     * @return A ConfigWrapper object.
     */
    ConfigWrapper info()
    {
        return m_pdfSetInfo;
    }
    
    // Rule of Five: disable copying, allow moving.
    PDFSet(const PDFSet &) = delete;
    PDFSet &operator=(const PDFSet &) = delete;
    PDFSet(PDFSet &&) = default;
    PDFSet &operator=(PDFSet &&) = default;

  private:
    /**
     * @brief Internal routine for uncertainty calculation.
     * @param pdfs A vector of PDF values from all set members.
     * @param cl The desired confidence level.
     * @param resUncertainty The output PDFUncertainty object.
     */
    inline void PDFUncertaintyInternalEvaluation(const std::vector<double> &pdfs, double cl,
                                                 PDFUncertainty &resUncertainty)
    {
        const double reqCL = ValidateAndGetCL(cl);

        m_uncertaintyStrategy_.Uncertainty(pdfs, m_pdfErrInfo.nmemCore(), reqCL, resUncertainty);
        resUncertainty.central = pdfs[0];

        ApplyConfidenceLevelScaling(resUncertainty, reqCL);
        StoreCoreVariationErros(resUncertainty);
        CalculateParameterVariationErrors(resUncertainty, pdfs);
    }
    
    /// @brief Initializes the PDF set by loading metadata and preparing strategies.
    void Initialize()
    {
        ValidatePDFSetName();
        LoadYamlInfo();
        InitializeUncertaintyStrategy();
    }

    /// @brief Initializes the QCD coupling object.
    void InitializeQCDCoupling()
    {
        m_qcdCoupling = CouplingFactory().mkCoupling(m_pdfSetName);
    }

    /// @brief Validates that the PDF set name is not empty.
    void ValidatePDFSetName()
    {
        if (m_pdfSetName.empty())
        {
            throw InvalidInputError(
                "PDFxTMD::PDFSet::ValidatePDFSetName: PDF set name cannot be empty.");
        }
    }

    /// @brief Loads all metadata from the .info YAML file.
    void LoadYamlInfo()
    {
        auto infoPathPair = StandardInfoFilePath(m_pdfSetName);
        if (infoPathPair.second != ErrorType::None)
            throw FileLoadException(
                "PDFxTMD::PDFSet::LoadYamlInfo: Unable to find info file of PDF set " +
                m_pdfSetName);

        m_isValid = true;
        auto yamlInfoErrPair = YamlErrorInfoReader(*infoPathPair.first);
        if (yamlInfoErrPair.second != ErrorType::None || !yamlInfoErrPair.first.has_value())
        {
            m_isValid = false;
        }
        else
        {
            m_pdfSetErrorInfo = *yamlInfoErrPair.first;
        }

        auto yamlStdInfoErrPair = YamlStandardPDFInfoReader(*infoPathPair.first);
        if (yamlStdInfoErrPair.second != ErrorType::None || !yamlStdInfoErrPair.first.has_value())
        {
            m_isValid = false;
        }
        else
        {
            m_pdfSetStdInfo = *yamlStdInfoErrPair.first;
        }

        if (!m_isValid)
        {
            throw InvalidInputError("PDFSet: Invalid PDF set '" + m_pdfSetName +
                                    "'. Please check the YAML file.");
        }
        if (!m_pdfSetInfo.loadFromFile(*infoPathPair.first, ConfigWrapper::Format::YAML))
        {
            throw FileLoadException("path: " + *infoPathPair.first + "not found.");
        }
    }

    /// @brief Selects and initializes the correct uncertainty calculation strategy.
    void InitializeUncertaintyStrategy()
    {
        m_pdfErrInfo = PDFErrInfo::CalculateErrorInfo(m_pdfSetErrorInfo);
        if (m_pdfErrInfo.nmemCore() <= 0)
        {
            m_uncertaintyStrategy_ = IUncertainty(NullUncertaintyStrategy());
            PDFxTMDLOG <<  "Error in PDFxTMD::PDFSet::InitializeUncertaintyStrategy. PDF "
                                    "set must contain more than just the central value.";
            return;
        }

        const auto &coreType = m_pdfErrInfo.coreType();
        if (coreType == "replicas")
        {
            if (m_alternativeReplicaUncertainty)
            {
                m_uncertaintyStrategy_ = IUncertainty(ReplicasPercentileStrategy());
            }
            else
            {
                m_uncertaintyStrategy_ = IUncertainty(ReplicasStdDevStrategy());
            }
        }
        else if (coreType == "hessian")
        {
            m_uncertaintyStrategy_ = IUncertainty(HessianStrategy());
        }
        else if (coreType == "symmhessian" || coreType == "symm-hessian")
        {
            m_uncertaintyStrategy_ = IUncertainty(SymmHessianStrategy());
        }
        else if (coreType == "unknown")
        {
            m_uncertaintyStrategy_ = IUncertainty(NullUncertaintyStrategy());
        }
        else
        {
            throw NotSupportError(
                "PDFxTMD::PDFSet::InitializeUncertaintyStrategy: Unsupported error type '" +
                coreType + "' for PDF set " + m_pdfSetName + ".");
        }

        m_setCL =
            (coreType != "replicas") ? m_pdfSetErrorInfo.ErrorConfLevel / 100.0 : CL1SIGMA / 100.0;
    }

    /// @brief Calculates PDF values for all members of the set for a given kinematic point.
    template <typename... Args>
    std::vector<double> CalculatePDFValues(PartonFlavor flavor, Args... args) const
    {
        std::vector<double> pdfs;
        pdfs.reserve(m_pdfSetStdInfo.NumMembers);
        for (size_t i = 0; i < size(); i++)
        {
            if constexpr (sizeof...(args) == 3)
            { // TMD case
                pdfs.emplace_back(operator[](i)->tmd(flavor, args...));
            }
            else
            { // Collinear case
                pdfs.emplace_back(operator[](i)->pdf(flavor, args...));
            }
        }
        return pdfs;
    }
    
    /// @brief Validates and returns the confidence level for calculations.
    double ValidateAndGetCL(double cl) const
    {
        const double reqCL = (cl >= 0) ? cl / 100.0 : m_setCL;
        if (!in_range(reqCL, 0, 1) || !in_range(m_setCL, 0, 1))
        {
            throw InvalidInputError(
                "Error in PDFxTMD::PDFSet::ValidateAndGetCL. Requested or PDF set "
                "confidence level outside [0,1] range.");
        }
        return reqCL;
    }

    /// @brief Scales the uncertainty to the requested confidence level.
    void ApplyConfidenceLevelScaling(PDFUncertainty &rtn, double reqCL) const
    {
        if (m_setCL != reqCL && !m_alternativeReplicaUncertainty)
        {
            double qsetCL = chisquared_quantile(m_setCL, 1);
            double qreqCL = chisquared_quantile(reqCL, 1);
            const double scale = std::sqrt(qreqCL / qsetCL);

            rtn.scale = scale;
            rtn.errplus *= scale;
            rtn.errminus *= scale;
            rtn.errsymm *= scale;
        }
    }

    /// @brief Stores the core PDF variation errors into the result struct.
    void StoreCoreVariationErros(PDFUncertainty &resUncertainty)
    {
        // Store core variation uncertainties
        resUncertainty.errplus_pdf = resUncertainty.errplus;
        resUncertainty.errminus_pdf = resUncertainty.errminus;
        resUncertainty.errsymm_pdf = resUncertainty.errsymm;
        resUncertainty.errparts.push_back(
            {resUncertainty.errplus_pdf,
             resUncertainty.errminus_pdf}); ///< @note (+,-) pair-ordering
    }

    /// @brief Calculates errors from parameter variations (e.g., alpha_s, quark masses).
    void CalculateParameterVariationErrors(PDFUncertainty &rtn,
                                           const std::vector<double> &values) const
    {
        double errsq_par_plus = 0.0;
        double errsq_par_minus = 0.0;
        size_t index = m_pdfErrInfo.nmemCore();

        for (size_t iq = 1; iq < m_pdfErrInfo.qparts.size(); ++iq)
        {
            const auto &eparts = m_pdfErrInfo.qparts[iq];
            double vmin = rtn.central;
            double vmax = rtn.central;

            for (const auto &epart : eparts)
            {
                const bool symm = StartsWith(epart.first, "$");
                for (size_t ie = 0; ie < epart.second; ++ie)
                {
                    index++;
                    if (!symm)
                    {
                        vmin = std::min(values[index], vmin);
                        vmax = std::max(values[index], vmax);
                    }
                    else
                    {
                        const double delta = values[index] - rtn.central;
                        vmin = std::min({values[index], rtn.central - delta, vmin});
                        vmax = std::max({values[index], rtn.central - delta, vmax});
                    }
                }
            }

            const double eplus = vmax - rtn.central;
            const double eminus = rtn.central - vmin;
            rtn.errparts.push_back({eplus, eminus});
            errsq_par_plus += SQR(eplus);
            errsq_par_minus += SQR(eminus);
        }
    }
    std::string m_pdfSetName;                   ///< The name of the PDF set.
    ConfigWrapper m_pdfSetInfo;                 ///< General metadata object for the set.
    YamlErrorInfo m_pdfSetErrorInfo;           ///< Specific error metadata.
    YamlStandardTMDInfo m_pdfSetStdInfo;       ///< Specific standard PDF metadata.
    PDFErrInfo m_pdfErrInfo;                     ///< Processed error structure information.
    std::map<unsigned, std::unique_ptr<PDF_t>> m_PDFSet_; ///< Map of created PDF member objects.
    std::vector<unsigned int> m_createdPDFSetsMember; ///< (Currently unused) Tracks created members.
    IUncertainty m_uncertaintyStrategy_;         ///< The strategy object for uncertainty calculations.
    IQCDCoupling m_qcdCoupling; ///< The QCD coupling calculation object.
    double m_setCL;                              ///< The native confidence level of the set.
    bool m_alternativeReplicaUncertainty;      ///< Flag for replica uncertainty method.
    bool m_isValid = false;                      ///< Flag indicating if the set loaded correctly.
    std::mutex m_pdfSetMtx;                      ///< Mutex for thread-safe creation of PDF members.
};

} // namespace PDFxTMD