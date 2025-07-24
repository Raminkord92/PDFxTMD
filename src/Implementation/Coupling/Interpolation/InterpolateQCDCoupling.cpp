#include "PDFxTMDLib/Implementation/Coupling/Interpolation/InterpolateQCDCoupling.h"
#include <PDFxTMDLib/external/mlinterp/mlinterp.hpp>
#include <algorithm>
#include <cmath>
#include <exception>

namespace PDFxTMD
{
void InterpolateQCDCoupling::initialize(const YamlCouplingInfo &couplingInfo)
{
    m_couplingInfo = couplingInfo;
    m_alsphasVec_vec = couplingInfo.alphas_vec;
    for (double mu_ : m_couplingInfo.mu_vec)
    {
        m_mu2_vec.push_back(SQR(mu_));
        m_logMu2_vec.push_back(std::log(SQR(mu_)));
    }
    setup_grids();
}

/// @note This is const so it can be called silently from a const method
void InterpolateQCDCoupling::setup_grids()
{
    if (!_knotarrays.empty())
        throw AlphaQCDError("AlphaS interpolation subgrids being initialized a second time!");

    if (m_mu2_vec.size() != m_alsphasVec_vec.size())
        throw MetadataError("AlphaS value and Q interpolation arrays are differently sized");

    // Walk along the Q2 vector, making subgrids at each boundary
    double prevQ2 = m_mu2_vec.front();
    std::vector<double> q2s, as;
    size_t combined_lenq2s = 0; //< For consistency checking
    for (size_t i = 0; i <= m_mu2_vec.size(); ++i)
    { //< The iteration to len+1 is intentional
        // Get current Q2 and alpha_s points, faked if i > vector to force syncing
        const double currQ2 = (i != m_mu2_vec.size()) ? m_mu2_vec[i] : m_mu2_vec.back();
        const double currAS = (i != m_mu2_vec.size()) ? m_alsphasVec_vec[i] : -1;
        // If the Q2 value is repeated, sync the current subgrid and start a new one.
        // Note special treatment for the first and last points in q2s.
        if (fabs(currQ2 - prevQ2) < std::numeric_limits<double>::epsilon())
        {
            // Sync current subgrid as as AlphaSArray
            if (i != 0)
            {
                _knotarrays[q2s.front()] = AlphaSArray(q2s, as);
                combined_lenq2s += q2s.size();
            }
            // Reset temporary vectors
            q2s.clear();
            q2s.reserve(m_mu2_vec.size() - i);
            as.clear();
            as.reserve(m_mu2_vec.size() - i);
        }
        // Append current value to temporary vectors
        q2s.push_back(currQ2);
        as.push_back(currAS);
        prevQ2 = currQ2;
    }
    if (combined_lenq2s != m_mu2_vec.size())
        throw AlphaQCDError("Sum of alpha_s subgrid sizes does not match input knot array (" +
                            std::to_string(combined_lenq2s) + " vs. " +
                            std::to_string(m_mu2_vec.size()) + ")");
}

double InterpolateQCDCoupling::interpolateCubic(double T, double VL, double VDL, double VH,
                                                double VDH) const
{
    // Pre-calculate powers of T
    const double t2 = T * T;
    const double t3 = t2 * T;

    // Calculate left point
    const double p0 = (2 * t3 - 3 * t2 + 1) * VL;
    const double m0 = (t3 - 2 * t2 + T) * VDL;

    // Calculate right point
    const double p1 = (-2 * t3 + 3 * t2) * VH;
    const double m1 = (t3 - t2) * VDH;

    return abs(p0 + m0 + p1 + m1) < 2. ? p0 + m0 + p1 + m1 : std::numeric_limits<double>::max();
}

// Interpolate alpha_s from tabulated points in Q2 via metadata
double InterpolateQCDCoupling::AlphaQCDMu2(double q2)
{
    assert(q2 >= 0);

    // Using base 10 for logs to get constant gradient extrapolation in
    // a log 10 - log 10 plot
    if (q2 < m_mu2_vec.front())
    {
        // Remember to take situations where the first knot also is a
        // flavor threshold into account
        double dlogq2, dlogas;
        unsigned int next_point = 1;
        while (m_mu2_vec[0] == m_mu2_vec[next_point])
            next_point++;
        dlogq2 = log10(m_mu2_vec[next_point] / m_mu2_vec[0]);
        dlogas = log10(m_alsphasVec_vec[next_point] / m_alsphasVec_vec[0]);
        const double loggrad = dlogas / dlogq2;
        return m_alsphasVec_vec[0] * pow(q2 / m_alsphasVec_vec[0], loggrad);
    }

    if (q2 > m_mu2_vec.back())
        return m_alsphasVec_vec.back();

    // Retrieve the appropriate subgrid
    auto it = --(_knotarrays.upper_bound(q2));
    const AlphaSArray &arr = it->second;

    // Get the Q/alpha_s index on this array which is *below* this Q point
    const size_t i = arr.iq2below(q2);

    // Calculate derivatives
    double didlogq2, di1dlogq2;
    if (i == 0)
    {
        didlogq2 = arr.ddlogq_forward(i);
        di1dlogq2 = arr.ddlogq_central(i + 1);
    }
    else if (i == arr.logq2s().size() - 2)
    {
        didlogq2 = arr.ddlogq_central(i);
        di1dlogq2 = arr.ddlogq_backward(i + 1);
    }
    else
    {
        didlogq2 = arr.ddlogq_central(i);
        di1dlogq2 = arr.ddlogq_central(i + 1);
    }

    // Calculate alpha_s
    const double dlogq2 = arr.logq2s()[i + 1] - arr.logq2s()[i];
    const double tlogq2 = (log(q2) - arr.logq2s()[i]) / dlogq2;
    return interpolateCubic(tlogq2, arr.alphas()[i], didlogq2 * dlogq2, arr.alphas()[i + 1],
                            di1dlogq2 * dlogq2);
}
} // namespace PDFxTMD
