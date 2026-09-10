#pragma once

#include "PDFxTMDLib/Common/PartonUtils.h"

#include <memory>
#include <type_traits>
#include <utility>

namespace PDFxTMD
{

/**
 * @brief Type-erased interface for collinear double parton distribution functions.
 *
 * This class wraps any DPDF implementation that provides:
 *
 *   double dpdf(PartonFlavor flavor1,
 *               PartonFlavor flavor2,
 *               double x1,
 *               double mu1_2,
 *               double x2,
 *               double mu2_2);
 *
 * For example, it can wrap:
 *
 *   PDFxTMD::CollinearDPDF
 */
class ICDPDF
{
  public:
    template <
        typename DPDFApproachT,
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<DPDFApproachT>, ICDPDF>>>
    explicit ICDPDF(DPDFApproachT &&dpdfApproach)
        : pimpl_(
              new OwningModel<std::decay_t<DPDFApproachT>>(
                  std::forward<DPDFApproachT>(dpdfApproach)),
              [](void *dpdfApproachBytes) {
                  using Model = OwningModel<std::decay_t<DPDFApproachT>>;
                  auto *const model = static_cast<Model *>(dpdfApproachBytes);
                  delete model;
              }),
          dpdfOperation_(
              [](void *dpdfApproachBytes,
                 PartonFlavor flavor1,
                 PartonFlavor flavor2,
                 double x1,
                 double mu1_2,
                 double x2,
                 double mu2_2) -> double {
                  using Model = OwningModel<std::decay_t<DPDFApproachT>>;
                  auto *const model = static_cast<Model *>(dpdfApproachBytes);

                  return model->dpdf(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
              }),
          clone_([](void *dpdfApproachBytes) -> void * {
              using Model = OwningModel<std::decay_t<DPDFApproachT>>;
              auto *const model = static_cast<Model *>(dpdfApproachBytes);
              return new Model(*model);
          })
    {
    }

    /**
     * @brief Evaluate the DPDF for two flavors, two x values, and two scales.
     *
     * @param flavor1 First parton flavor.
     * @param flavor2 Second parton flavor.
     * @param x1 First momentum fraction.
     * @param mu1_2 First factorization scale squared.
     * @param x2 Second momentum fraction.
     * @param mu2_2 Second factorization scale squared.
     *
     * @return DPDF value.
     */
    double dpdf(PartonFlavor flavor1,
                PartonFlavor flavor2,
                double x1,
                double mu1_2,
                double x2,
                double mu2_2) const
    {
        return dpdfOperation_(pimpl_.get(), flavor1, flavor2, x1, mu1_2, x2, mu2_2);
    }

    ICDPDF(const ICDPDF &other)
        : pimpl_(other.clone_(other.pimpl_.get()), other.pimpl_.get_deleter()),
          clone_(other.clone_),
          dpdfOperation_(other.dpdfOperation_)
    {
    }

    ICDPDF &operator=(const ICDPDF &other)
    {
        if (this == &other)
            return *this;

        ICDPDF copy(other);

        using std::swap;
        swap(pimpl_, copy.pimpl_);
        swap(clone_, copy.clone_);
        swap(dpdfOperation_, copy.dpdfOperation_);

        return *this;
    }

    ICDPDF(ICDPDF &&other) noexcept = default;
    ICDPDF &operator=(ICDPDF &&other) noexcept = default;

    ~ICDPDF() = default;

  private:
    template <typename DPDFApproachT>
    struct OwningModel
    {
        explicit OwningModel(DPDFApproachT dpdfApproach)
            : dpdfApproach_(std::move(dpdfApproach))
        {
        }

        double dpdf(PartonFlavor flavor1,
                    PartonFlavor flavor2,
                    double x1,
                    double mu1_2,
                    double x2,
                    double mu2_2)
        {
            return dpdfApproach_.dpdf(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
        }

        DPDFApproachT dpdfApproach_;
    };

    using DestroyOperation = void(void *);
    using CloneOperation = void *(void *);
    using DPDFOperation =
        double(void *, PartonFlavor, PartonFlavor, double, double, double, double);

    std::unique_ptr<void, DestroyOperation *> pimpl_;
    CloneOperation *clone_{nullptr};
    DPDFOperation *dpdfOperation_{nullptr};
};

} // namespace PDFxTMD