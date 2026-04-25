#pragma once
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include <array>

namespace PDFxTMD
{
template <typename Derived, typename... ExtrapolateArgs>
class IExtrapolator : public CRTPBase<Derived>
{
  public:
   template<typename... Flavors>
    double extrapolate(Flavors... partons, ExtrapolateArgs... args) const
    {
        return this->derived().extrapolate(partons..., args...);
    }
};

template <typename Derived, typename... ExtrapolateArgs>
class IAdvancedExtrapolator : public IExtrapolator<Derived, ExtrapolateArgs...>
{
  protected:
    template <typename InterpolatorType> void setInterpolator(const InterpolatorType *interpolator)
    {
        this->derived().setInterpolator(interpolator);
    }
};
} // namespace PDFxTMD