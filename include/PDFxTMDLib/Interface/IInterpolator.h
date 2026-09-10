#pragma once
#include "PDFxTMDLib/Common/InterfaceUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{
template <typename Derived, typename Reader, typename... InterpolateArgs>
class IInterpolator : public CRTPBase<Derived>
{
  public:
    void initialize(const IReader<Reader> *reader)
    {
        this->derived().initialize(reader);
    }
    template <typename... Flavors>
    double interpolate(Flavors&&... flavors, InterpolateArgs&&... args) const
    {
        return this->derived().interpolate(std::forward<Flavors>(flavors)..., std::forward<Flavors>(args)...);
    }
    const IReader<Reader> *getReader() const
    {
        return this->derived().getReader();
    }
};

} // namespace PDFxTMD