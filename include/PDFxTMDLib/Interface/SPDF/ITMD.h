#pragma once
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace PDFxTMD
{
/**
 * @brief Interface for Transverse Momentum Dependent (TMD) parton distribution functions.
 *
 * This class provides a type-erased interface for accessing TMD PDFs.
 *
 * The class handles:
 * - Single flavor TMD evaluation with tmd(flavor, x, kt2, mu2)
 * - All flavors TMD evaluation with tmd(x, kt2, mu2, tmds)
 */
class ITMD
{
  public:
    template <typename TMDApproachT>
    explicit ITMD(TMDApproachT tmdApproach)
        : pimpl_(new OwningModel<TMDApproachT>(std::move(tmdApproach)),
                 [](void *tmdfApproachBytes) {
                     using Model = OwningModel<TMDApproachT>;
                     auto *const model = static_cast<Model *>(tmdfApproachBytes);
                     delete model;
                 }),
          tmdOperation_([](void *tmdfApproachBytes, PartonFlavor flavor, double x, double kt2,
                           double mu2) -> double {
              using Model = OwningModel<TMDApproachT>;
              auto *const model = static_cast<Model *>(tmdfApproachBytes);
              return model->tmd(flavor, x, kt2, mu2);
          }),
          tmdOperation1_([](void *tmdfApproachBytes, double x, double kt2, double mu2,
                            std::array<double, DEFAULT_TOTAL_PDFS> &output) -> void {
              using Model = OwningModel<TMDApproachT>;
              auto *const model = static_cast<Model *>(tmdfApproachBytes);
              model->tmd(x, kt2, mu2, output);
          }),
          clone_([](void *tmdfApproachBytes) -> void * {
              using Model = OwningModel<TMDApproachT>;
              auto *const model = static_cast<Model *>(tmdfApproachBytes);
              return new Model(*model);
          })
    {
    }

    /**
     * @brief Evaluate the TMD PDF for a specific flavor.
     *
     * This function evaluates the TMD PDF for a given flavor, x, kt2, and mu2.
     *
     * @param flavor The parton flavor to evaluate the TMD PDF for.
     * @param x The momentum fraction of the parton.
     * @param kt2 The transverse momentum squared of the parton.
     * @param mu2 The factorization scale squared.
     * @return The value of the TMD PDF for the given flavor, x, kt2, and mu2.
     */
    double tmd(PartonFlavor flavor, double x, double kt2, double mu2) const
    {
        return tmdOperation_(pimpl_.get(), flavor, x, kt2, mu2);
    }

    /**
     * @brief Evaluate the TMD PDF for all flavors.
     *
     * This function evaluates the TMD PDF for all flavors at once, given x, kt2, and mu2.
     *
     * @param x The momentum fraction of the parton.
     * @param kt2 The transverse momentum squared of the parton.
     * @param mu2 The factorization scale squared.
     * @return A vector containing the TMD PDF values for all flavors {tbar [0], bbar [1], cbar [2],
     * sbar [3], ubar [4], dbar[5], g [6], d [7], u [8], s [9], c [10], b [11], t [12]}.
     */
    void tmd(double x, double kt2, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        return tmdOperation1_(pimpl_.get(), x, kt2, mu2, output);
    }

    /**
     * @brief Copy constructor for ITMD objects.
     *
     * This constructor creates a new ITMD object as a copy of another ITMD object.
     *
     * @param other The ITMD object to copy.
     */
    ITMD(const ITMD &other)
        : pimpl_(other.clone_(other.pimpl_.get()), other.pimpl_.get_deleter()),
          clone_(other.clone_), tmdOperation_(other.tmdOperation_),
          tmdOperation1_(other.tmdOperation1_)
    {
    }

    /**
     * @brief Assignment operator for ITMD objects.
     *
     * This operator assigns the value of another ITMD object to the current ITMD object.
     *
     * @param other The ITMD object to assign from.
     * @return A reference to the current ITMD object.
     */
    ITMD &operator=(ITMD const &other)
    {
        using std::swap;
        ITMD copy(other);
        swap(pimpl_, copy.pimpl_);
        swap(clone_, copy.clone_);
        swap(tmdOperation_, copy.tmdOperation_);
        swap(tmdOperation1_, copy.tmdOperation1_);
        return *this;
    }

    ITMD(ITMD &&other) noexcept = default;
    ~ITMD() = default;
    ITMD &operator=(ITMD &&other) = default;

  private:
    template <typename TMDApproachT> struct OwningModel
    {
        OwningModel(TMDApproachT tmdApproach) : m_tmdApproach(std::move(tmdApproach))
        {
        }
        /**
         * @brief Evaluate the TMD PDF for a specific flavor.
         *
         * This function evaluates the TMD PDF for a given flavor, x, kt2, and mu2.
         *
         * @param flavor The parton flavor to evaluate the TMD PDF for.
         * @param x The momentum fraction of the parton.
         * @param kt2 The transverse momentum squared of the parton.
         * @param mu2 The factorization scale squared.
         * @return The value of the TMD PDF for the given flavor, x, kt2, and mu2.
         */

        double tmd(PartonFlavor flavor, double x, double kt2, double mu2)
        {
            return m_tmdApproach.tmd(flavor, x, kt2, mu2);
        }
        /**
         * @brief Evaluate the array of TMD PDF values for {tbar, bbar, cbar, sbar, ubar, dbar,
         * g, d, u, s, c, b, t}
         *
         * This function evaluates the TMD PDF for all flavors at once, given x, kt2, and mu2.
         *
         * @param x The momentum fraction of the parton.
         * @param kt2 The transverse momentum squared of the parton.
         * @param mu2 The factorization scale squared.
         * @param output The array to store the TMD PDF values for all flavors.
         */
        void tmd(double x, double kt2, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output)
        {
            return m_tmdApproach.tmd(x, kt2, mu2, output);
        }
        TMDApproachT m_tmdApproach;
    };

    using DestroyOperation = void(void *);
    using CloneOperation = void *(void *);
    using TMDOperation = double(void *, PartonFlavor, double, double, double);
    using TMDOperation1 = void(void *, double, double, double,
                               std::array<double, DEFAULT_TOTAL_PDFS> &output);

    std::unique_ptr<void, DestroyOperation *> pimpl_;
    CloneOperation *clone_{nullptr};
    TMDOperation *tmdOperation_{nullptr};
    TMDOperation1 *tmdOperation1_{nullptr};
};
} // namespace PDFxTMD
