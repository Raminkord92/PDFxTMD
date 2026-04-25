#pragma once
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

namespace PDFxTMD
{
/**
 * @brief Interface for Collinear Parton Distribution Functions (CPDFs).
 *
 * This class provides a type-erased interface for accessing CPDFs.
 * The class handles:
 * - Single flavor CPDF evaluation with pdf(flavor, x, mu2)
 * - All flavors TMD evaluation with tmd(x, kt2, mu2, cPDFs)
 */
class ICPDF
{
  public:
    template <typename CPDFApproachT>
    explicit ICPDF(CPDFApproachT pdfApproach)
        : pimpl_(new OwningModel<CPDFApproachT>(std::move(pdfApproach)),
                 [](void *pdfApproachBytes) {
                     using Model = OwningModel<CPDFApproachT>;
                     auto *const model = static_cast<Model *>(pdfApproachBytes);
                     delete model;
                 }),
          pdfOperation_(
              [](void *pdfApproachBytes, PartonFlavor flavor, double x, double mu2) -> double {
                  using Model = OwningModel<CPDFApproachT>;
                  auto *const model = static_cast<Model *>(pdfApproachBytes);
                  return model->pdf(flavor, x, mu2); // fixed the pdf method call
              }),
          pdfOperation1_([](void *pdfApproachBytes, double x, double mu2,
                            std::array<double, 13> &output) -> void {
              using Model = OwningModel<CPDFApproachT>;
              auto *const model = static_cast<Model *>(pdfApproachBytes);
              return model->pdf(x, mu2, output); // fixed the pdf method call
          }),
          clone_([](void *pdfApproachBytes) -> void * {
              using Model = OwningModel<CPDFApproachT>;
              auto *const model = static_cast<Model *>(pdfApproachBytes);
              return new Model(*model);
          })
    {
    }

    /**
     * @brief Evaluate the CPDF for a specific flavor.
     *
     * This function evaluates the CPDF for a given flavor, x, and mu2.
     *
     * @param flavor The parton flavor to evaluate the CPDF for.
     * @param x The momentum fraction of the parton.
     * @param mu2 The factorization scale squared.
     * @return The value of the CPDF for the given flavor, x, and mu2.
     */
    double pdf(PartonFlavor parton, double x, double mu2) const
    {
        return pdfOperation_(pimpl_.get(), parton, x, mu2);
    }
    /**
     * @brief Evaluate the array of Collinear PDF values for {tbar, bbar, cbar, sbar, ubar, dbar,
     * g, d, u, s, c, b, t}
     *
     * This function evaluates the CPDF for all flavors given x and mu2, and stores the results in
     * the output array.
     *
     * @param x The momentum fraction of the parton.
     * @param mu2 The factorization scale squared.
     * @param output The array to store the CPDF values for all flavors.
     */

    void pdf(double x, double mu2, std::array<double, 13> &output) const
    {
        pdfOperation1_(pimpl_.get(), x, mu2, output);
    }
    /**
     * @brief Copy constructor for ICPDF objects.
     *
     * This constructor creates a new ICPDF object as a copy of another ICPDF object.
     *
     * @param other The ICPDF object to copy.
     */
    ICPDF(const ICPDF &other)
        : pimpl_(other.clone_(other.pimpl_.get()), other.pimpl_.get_deleter()),
          clone_(other.clone_), pdfOperation_(other.pdfOperation_),
          pdfOperation1_(other.pdfOperation1_)

    {
    }

    /**
     * @brief Assignment operator for ICPDF objects.
     *
     * This operator assigns the value of another ICPDF object to the current ICPDF object.
     *
     * @param other The ICPDF object to assign from.
     * @return A reference to the current ICPDF object.
     */
    ICPDF &operator=(ICPDF const &other)
    {
        using std::swap;
        ICPDF copy(other);
        swap(pimpl_, copy.pimpl_);
        swap(clone_, copy.clone_);
        swap(pdfOperation_, copy.pdfOperation_);
        swap(pdfOperation1_, copy.pdfOperation1_);
        return *this;
    }

    ICPDF(ICPDF &&other) noexcept = default;
    ~ICPDF() = default;
    ICPDF &operator=(ICPDF &&other) = default;

  private:
    template <typename CPDFApproachT> struct OwningModel
    {
        OwningModel(CPDFApproachT pdfApproach) : pdfApproach_(std::move(pdfApproach))
        {
        }

        double pdf(PartonFlavor flavor, double x, double mu2)
        {
            return pdfApproach_.pdf(flavor, x, mu2);
        }
        void pdf(double x, double mu2, std::array<double, 13> &output)
        {
            return pdfApproach_.pdf(x, mu2, output);
        }
        CPDFApproachT pdfApproach_;
    };

    using DestroyOperation = void(void *);
    using CloneOperation = void *(void *);
    using CPDFOperation = double(void *, PartonFlavor, double, double);
    using CPDFOperation1 = void(void *, double, double, std::array<double, 13> &);

    std::unique_ptr<void, DestroyOperation *> pimpl_;
    CloneOperation *clone_{nullptr};
    CPDFOperation *pdfOperation_{nullptr};
    CPDFOperation1 *pdfOperation1_{nullptr};
};
} // namespace PDFxTMD
