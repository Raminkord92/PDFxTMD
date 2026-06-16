#include "PDFxTMDLib/Factory.h"
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/Interface/IQCDCoupling.h"
#include "PDFxTMDLib/Interface/SPDF/ICPDF.h"
#include "PDFxTMDLib/Interface/SPDF/ITMD.h"
#include "PDFxTMDLib/Interface/DPDF/ICDPDF.h"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace
{
    inline void validate_x(double x, const std::string &name)
    {
        if (x <= 0.0 || x >= 1.0)
        {
            throw std::invalid_argument(name + " must be in (0, 1)");
        }
    }

    inline void validate_mu2(double mu2, const std::string &name)
    {
        if (mu2 <= 0.0)
        {
            throw std::invalid_argument(name + " must be positive");
        }
    }

    inline void validate_kt2(double kt2)
    {
        if (kt2 < 0.0)
        {
            throw std::invalid_argument("kt2 must be non-negative");
        }
    }

    inline void validate_dpdf_kinematics(
        double x1,
        double mu1_2,
        double x2,
        double mu2_2)
    {
        validate_x(x1, "x1");
        validate_x(x2, "x2");
        validate_mu2(mu1_2, "mu1_2");
        validate_mu2(mu2_2, "mu2_2");
    }

    inline bool outside_dpdf_support(double x1, double x2)
    {
        return x1 + x2 >= 1.0;
    }

    inline void validate_same_size(
        const std::vector<double> &x1,
        const std::vector<double> &mu1_2,
        const std::vector<double> &x2,
        const std::vector<double> &mu2_2)
    {
        const std::size_t n = x1.size();

        if (mu1_2.size() != n || x2.size() != n || mu2_2.size() != n)
        {
            throw std::invalid_argument(
                "x1, mu1_2, x2, and mu2_2 must have the same length");
        }
    }


    using DoubleArray = py::array_t<double, py::array::c_style | py::array::forcecast>;

    inline void validate_same_shape_arrays(
        const py::buffer_info &a,
        const py::buffer_info &b,
        const std::string &a_name,
        const std::string &b_name)
    {
        if (a.ndim != b.ndim)
        {
            throw std::invalid_argument(
                a_name + " and " + b_name + " must have the same number of dimensions");
        }

        for (py::ssize_t i = 0; i < a.ndim; ++i)
        {
            if (a.shape[static_cast<std::size_t>(i)] != b.shape[static_cast<std::size_t>(i)])
            {
                throw std::invalid_argument(
                    a_name + " and " + b_name + " must have identical shapes");
            }
        }
    }

    inline void validate_same_shape_arrays(
        const py::buffer_info &x1,
        const py::buffer_info &mu1_2,
        const py::buffer_info &x2,
        const py::buffer_info &mu2_2)
    {
        validate_same_shape_arrays(x1, mu1_2, "x1", "mu1_2");
        validate_same_shape_arrays(x1, x2, "x1", "x2");
        validate_same_shape_arrays(x1, mu2_2, "x1", "mu2_2");
    }

    template <typename CPDF>
    py::array_t<double> pdf_batch_numpy_impl(
        CPDF &self,
        PDFxTMD::PartonFlavor flavor,
        DoubleArray x,
        DoubleArray mu2)
    {
        const py::buffer_info x_info = x.request();
        const py::buffer_info mu2_info = mu2.request();

        validate_same_shape_arrays(x_info, mu2_info, "x", "mu2");

        const auto n = x_info.size;
        const double *x_ptr = static_cast<const double *>(x_info.ptr);
        const double *mu2_ptr = static_cast<const double *>(mu2_info.ptr);

        for (py::ssize_t i = 0; i < n; ++i)
        {
            validate_x(x_ptr[i], "x");
            validate_mu2(mu2_ptr[i], "mu2");
        }

        py::array_t<double> values(x_info.shape);
        double *out_ptr = static_cast<double *>(values.request().ptr);

        {
            py::gil_scoped_release release;
            for (py::ssize_t i = 0; i < n; ++i)
            {
                out_ptr[i] = self.pdf(flavor, x_ptr[i], mu2_ptr[i]);
            }
        }

        return values;
    }

    template <typename DPDF>
    py::array_t<double> dpdf_batch_numpy_impl(
        DPDF &self,
        PDFxTMD::PartonFlavor flavor1,
        PDFxTMD::PartonFlavor flavor2,
        DoubleArray x1,
        DoubleArray mu1_2,
        DoubleArray x2,
        DoubleArray mu2_2,
        bool enforce_support)
    {
        const py::buffer_info x1_info = x1.request();
        const py::buffer_info mu1_info = mu1_2.request();
        const py::buffer_info x2_info = x2.request();
        const py::buffer_info mu2_info = mu2_2.request();

        validate_same_shape_arrays(x1_info, mu1_info, x2_info, mu2_info);

        const auto n = x1_info.size;
        const double *x1_ptr = static_cast<const double *>(x1_info.ptr);
        const double *mu1_ptr = static_cast<const double *>(mu1_info.ptr);
        const double *x2_ptr = static_cast<const double *>(x2_info.ptr);
        const double *mu2_ptr = static_cast<const double *>(mu2_info.ptr);

        for (py::ssize_t i = 0; i < n; ++i)
        {
            validate_dpdf_kinematics(x1_ptr[i], mu1_ptr[i], x2_ptr[i], mu2_ptr[i]);
        }

        py::array_t<double> values(x1_info.shape);
        double *out_ptr = static_cast<double *>(values.request().ptr);

        {
            py::gil_scoped_release release;
            for (py::ssize_t i = 0; i < n; ++i)
            {
                if (enforce_support && outside_dpdf_support(x1_ptr[i], x2_ptr[i]))
                {
                    out_ptr[i] = 0.0;
                }
                else
                {
                    out_ptr[i] = self.dpdf(
                        flavor1,
                        flavor2,
                        x1_ptr[i],
                        mu1_ptr[i],
                        x2_ptr[i],
                        mu2_ptr[i]);
                }
            }
        }

        return values;
    }
}

PYBIND11_MODULE(pdfxtmd, m)
{
    m.doc() =
        "Python bindings for PDFxTMD with CPDF, TMD, and DPDF support.";

    m.attr("__has_dpdf__") = true;

    py::enum_<PDFxTMD::PartonFlavor>(
        m,
        "PartonFlavor",
        "Enum representing parton flavors")
        .value("u", PDFxTMD::PartonFlavor::u)
        .value("d", PDFxTMD::PartonFlavor::d)
        .value("s", PDFxTMD::PartonFlavor::s)
        .value("c", PDFxTMD::PartonFlavor::c)
        .value("b", PDFxTMD::PartonFlavor::b)
        .value("t", PDFxTMD::PartonFlavor::t)
        .value("g", PDFxTMD::PartonFlavor::g)
        .value("ubar", PDFxTMD::PartonFlavor::ubar)
        .value("dbar", PDFxTMD::PartonFlavor::dbar)
        .value("sbar", PDFxTMD::PartonFlavor::sbar)
        .value("cbar", PDFxTMD::PartonFlavor::cbar)
        .value("bbar", PDFxTMD::PartonFlavor::bbar)
        .value("tbar", PDFxTMD::PartonFlavor::tbar)
        .export_values();

    py::class_<PDFxTMD::IQCDCoupling>(
        m,
        "IQCDCoupling",
        "Interface for QCD coupling calculations")
        .def(
            "AlphaQCDMu2",
            [](const PDFxTMD::IQCDCoupling &self, double mu2) {
                try
                {
                    validate_mu2(mu2, "mu2");
                    return self.AlphaQCDMu2(mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error calculating alpha_s at mu2=" +
                        std::to_string(mu2) + ": " + e.what());
                }
            },
            py::arg("mu2"));

    py::class_<PDFxTMD::CouplingFactory>(
        m,
        "CouplingFactory",
        "Factory for QCD coupling objects")
        .def(py::init<>())
        .def(
            "mkCoupling",
            [](PDFxTMD::CouplingFactory &self,
               const std::string &pdfSetName) {
                try
                {
                    if (pdfSetName.empty())
                    {
                        throw std::invalid_argument(
                            "PDF set name cannot be empty");
                    }

                    return self.mkCoupling(pdfSetName);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error creating QCD coupling for '" +
                        pdfSetName + "': " + e.what());
                }
            },
            py::arg("pdfSetName"),
            py::return_value_policy::take_ownership);

    py::class_<PDFxTMD::ICPDF>(
        m,
        "ICPDF",
        "Interface for collinear single PDFs")
        .def(
            "pdf",
            [](const PDFxTMD::ICPDF &self,
               PDFxTMD::PartonFlavor flavor,
               double x,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_mu2(mu2, "mu2");

                    return self.pdf(flavor, x, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CPDF at x=" +
                        std::to_string(x) +
                        ", mu2=" +
                        std::to_string(mu2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("flavor"),
            py::arg("x"),
            py::arg("mu2"),
            "Evaluate one collinear PDF flavor.")
        .def(
            "pdf_batch",
            [](const PDFxTMD::ICPDF &self,
               PDFxTMD::PartonFlavor flavor,
               DoubleArray x,
               DoubleArray mu2) {
                try
                {
                    return pdf_batch_numpy_impl(self, flavor, x, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CPDF batch: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor"),
            py::arg("x"),
            py::arg("mu2"),
            "Evaluate one collinear PDF flavor for a NumPy array of points.")
        .def(
            "pdf",
            [](const PDFxTMD::ICPDF &self,
               double x,
               double mu2,
               py::list &output) {
                try
                {
                    validate_x(x, "x");
                    validate_mu2(mu2, "mu2");

                    std::array<double, 13> temp;
                    self.pdf(x, mu2, temp);

                    for (double value : temp)
                    {
                        output.append(value);
                    }
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating all CPDF flavors at x=" +
                        std::to_string(x) +
                        ", mu2=" +
                        std::to_string(mu2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("x"),
            py::arg("mu2"),
            py::arg("output"),
            "Evaluate all collinear PDF flavors into output list.")
        .def(
            "pdf_all",
            [](const PDFxTMD::ICPDF &self,
               double x,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_mu2(mu2, "mu2");

                    std::array<double, 13> temp;
                    self.pdf(x, mu2, temp);

                    return std::vector<double>(temp.begin(), temp.end());
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating all CPDF flavors at x=" +
                        std::to_string(x) +
                        ", mu2=" +
                        std::to_string(mu2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("x"),
            py::arg("mu2"),
            "Evaluate all collinear PDF flavors and return a list.");

    py::class_<PDFxTMD::ITMD>(
        m,
        "ITMD",
        "Interface for transverse-momentum-dependent PDFs")
        .def(
            "tmd",
            [](const PDFxTMD::ITMD &self,
               PDFxTMD::PartonFlavor flavor,
               double x,
               double kt2,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_kt2(kt2);
                    validate_mu2(mu2, "mu2");

                    return self.tmd(flavor, x, kt2, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating TMD at x=" +
                        std::to_string(x) +
                        ", kt2=" +
                        std::to_string(kt2) +
                        ", mu2=" +
                        std::to_string(mu2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("flavor"),
            py::arg("x"),
            py::arg("kt2"),
            py::arg("mu2"),
            "Evaluate one TMD flavor.")
        .def(
            "tmd",
            [](const PDFxTMD::ITMD &self,
               double x,
               double kt2,
               double mu2,
               py::list &output) {
                try
                {
                    validate_x(x, "x");
                    validate_kt2(kt2);
                    validate_mu2(mu2, "mu2");

                    std::array<double, 13> temp;
                    self.tmd(x, kt2, mu2, temp);

                    for (double value : temp)
                    {
                        output.append(value);
                    }
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating all TMD flavors at x=" +
                        std::to_string(x) +
                        ", kt2=" +
                        std::to_string(kt2) +
                        ", mu2=" +
                        std::to_string(mu2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("x"),
            py::arg("kt2"),
            py::arg("mu2"),
            py::arg("output"),
            "Evaluate all TMD flavors into output list.")
        .def(
            "tmd_all",
            [](const PDFxTMD::ITMD &self,
               double x,
               double kt2,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_kt2(kt2);
                    validate_mu2(mu2, "mu2");

                    std::array<double, 13> temp;
                    self.tmd(x, kt2, mu2, temp);

                    return std::vector<double>(temp.begin(), temp.end());
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating all TMD flavors at x=" +
                        std::to_string(x) +
                        ", kt2=" +
                        std::to_string(kt2) +
                        ", mu2=" +
                        std::to_string(mu2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("x"),
            py::arg("kt2"),
            py::arg("mu2"),
            "Evaluate all TMD flavors and return a list.");

    py::class_<PDFxTMD::ICDPDF>(
        m,
        "ICDPDF",
        "Interface for collinear double PDFs")
        .def(
            "dpdf",
            [](const PDFxTMD::ICDPDF &self,
               PDFxTMD::PartonFlavor flavor1,
               PDFxTMD::PartonFlavor flavor2,
               double x1,
               double mu1_2,
               double x2,
               double mu2_2,
               bool enforce_support) {
                try
                {
                    validate_dpdf_kinematics(x1, mu1_2, x2, mu2_2);

                    if (enforce_support && outside_dpdf_support(x1, x2))
                    {
                        return 0.0;
                    }

                    return self.dpdf(
                        flavor1,
                        flavor2,
                        x1,
                        mu1_2,
                        x2,
                        mu2_2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating DPDF at x1=" +
                        std::to_string(x1) +
                        ", mu1_2=" +
                        std::to_string(mu1_2) +
                        ", x2=" +
                        std::to_string(x2) +
                        ", mu2_2=" +
                        std::to_string(mu2_2) +
                        ": " +
                        e.what());
                }
            },
            py::arg("flavor1"),
            py::arg("flavor2"),
            py::arg("x1"),
            py::arg("mu1_2"),
            py::arg("x2"),
            py::arg("mu2_2"),
            py::arg("enforce_support") = true,
            "Evaluate a collinear double PDF.")
        .def(
            "dpdf_batch",
            [](const PDFxTMD::ICDPDF &self,
               PDFxTMD::PartonFlavor flavor1,
               PDFxTMD::PartonFlavor flavor2,
               DoubleArray x1,
               DoubleArray mu1_2,
               DoubleArray x2,
               DoubleArray mu2_2,
               bool enforce_support) {
                try
                {
                    return dpdf_batch_numpy_impl(
                        self,
                        flavor1,
                        flavor2,
                        x1,
                        mu1_2,
                        x2,
                        mu2_2,
                        enforce_support);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating DPDF NumPy batch: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor1"),
            py::arg("flavor2"),
            py::arg("x1"),
            py::arg("mu1_2"),
            py::arg("x2"),
            py::arg("mu2_2"),
            py::arg("enforce_support") = true,
            "Evaluate many DPDF points for one flavor pair from NumPy arrays.")
        .def(
            "dpdf_batch",
            [](const PDFxTMD::ICDPDF &self,
               PDFxTMD::PartonFlavor flavor1,
               PDFxTMD::PartonFlavor flavor2,
               const std::vector<double> &x1,
               const std::vector<double> &mu1_2,
               const std::vector<double> &x2,
               const std::vector<double> &mu2_2,
               bool enforce_support) {
                try
                {
                    validate_same_size(x1, mu1_2, x2, mu2_2);

                    std::vector<double> values;
                    values.reserve(x1.size());

                    for (std::size_t i = 0; i < x1.size(); ++i)
                    {
                        validate_dpdf_kinematics(
                            x1[i],
                            mu1_2[i],
                            x2[i],
                            mu2_2[i]);

                        if (enforce_support && outside_dpdf_support(x1[i], x2[i]))
                        {
                            values.push_back(0.0);
                        }
                        else
                        {
                            values.push_back(
                                self.dpdf(
                                    flavor1,
                                    flavor2,
                                    x1[i],
                                    mu1_2[i],
                                    x2[i],
                                    mu2_2[i]));
                        }
                    }

                    return values;
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating DPDF batch: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor1"),
            py::arg("flavor2"),
            py::arg("x1"),
            py::arg("mu1_2"),
            py::arg("x2"),
            py::arg("mu2_2"),
            py::arg("enforce_support") = true,
            "Evaluate many DPDF points for one flavor pair.");

    py::class_<PDFxTMD::GenericCPDFFactory>(
        m,
        "GenericCPDFFactory",
        "Factory for creating CPDF objects")
        .def(py::init<>())
        .def(
            "mkCPDF",
            [](PDFxTMD::GenericCPDFFactory &self,
               const std::string &pdfSetName,
               int setMember) {
                try
                {
                    if (pdfSetName.empty())
                    {
                        throw std::invalid_argument(
                            "PDF set name cannot be empty");
                    }

                    if (setMember < 0)
                    {
                        throw std::invalid_argument(
                            "Set member index must be non-negative");
                    }

                    return self.mkCPDF(pdfSetName, setMember);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error creating CPDF for '" +
                        pdfSetName +
                        "': " +
                        e.what());
                }
            },
            py::arg("pdfSetName"),
            py::arg("setMember"),
            py::return_value_policy::take_ownership);

    py::class_<PDFxTMD::GenericTMDFactory>(
        m,
        "GenericTMDFactory",
        "Factory for creating TMD objects")
        .def(py::init<>())
        .def(
            "mkTMD",
            [](PDFxTMD::GenericTMDFactory &self,
               const std::string &pdfSetName,
               int setMember) {
                try
                {
                    if (pdfSetName.empty())
                    {
                        throw std::invalid_argument(
                            "PDF set name cannot be empty");
                    }

                    if (setMember < 0)
                    {
                        throw std::invalid_argument(
                            "Set member index must be non-negative");
                    }

                    return self.mkTMD(pdfSetName, setMember);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error creating TMD for '" +
                        pdfSetName +
                        "': " +
                        e.what());
                }
            },
            py::arg("pdfSetName"),
            py::arg("setMember"),
            py::return_value_policy::take_ownership);

    py::class_<PDFxTMD::GenericCDPDFFactory>(
        m,
        "GenericCDPDFFactory",
        "Factory for creating collinear double PDF objects")
        .def(py::init<>())
        .def(
            "mkCDPDF",
            [](PDFxTMD::GenericCDPDFFactory &self,
               const std::string &pdfSetName,
               int setMember) {
                try
                {
                    if (pdfSetName.empty())
                    {
                        throw std::invalid_argument(
                            "DPDF set name cannot be empty");
                    }

                    if (setMember < 0)
                    {
                        throw std::invalid_argument(
                            "Set member index must be non-negative");
                    }

                    return self.mkCDPDF(pdfSetName, setMember);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error creating CDPDF/DPDF for '" +
                        pdfSetName +
                        "': " +
                        e.what());
                }
            },
            py::arg("pdfSetName"),
            py::arg("setMember"),
            py::return_value_policy::take_ownership);

    py::class_<PDFxTMD::CollinearPDF>(
        m,
        "CollinearPDF",
        "Concrete collinear single PDF object")
        .def(
            py::init<const std::string &, int>(),
            py::arg("pdfSetName"),
            py::arg("setMember"))
        .def(
            "pdf",
            [](PDFxTMD::CollinearPDF &self,
               PDFxTMD::PartonFlavor flavor,
               double x,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_mu2(mu2, "mu2");

                    return self.pdf(flavor, x, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CollinearPDF: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor"),
            py::arg("x"),
            py::arg("mu2"))
        .def(
            "pdf_batch",
            [](PDFxTMD::CollinearPDF &self,
               PDFxTMD::PartonFlavor flavor,
               DoubleArray x,
               DoubleArray mu2) {
                try
                {
                    return pdf_batch_numpy_impl(self, flavor, x, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CollinearPDF NumPy batch: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor"),
            py::arg("x"),
            py::arg("mu2"),
            "Evaluate one collinear PDF flavor for a NumPy array of points.")
        .def(
            "pdf_all",
            [](PDFxTMD::CollinearPDF &self,
               double x,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_mu2(mu2, "mu2");

                    std::array<double, 13> temp;
                    self.pdf(x, mu2, temp);

                    return std::vector<double>(temp.begin(), temp.end());
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating all CollinearPDF flavors: " +
                        std::string(e.what()));
                }
            },
            py::arg("x"),
            py::arg("mu2"));

    py::class_<PDFxTMD::TMDPDF>(
        m,
        "TMDPDF",
        "Concrete TMD PDF object")
        .def(
            py::init<const std::string &, int>(),
            py::arg("pdfSetName"),
            py::arg("setMember"))
        .def(
            "tmd",
            [](PDFxTMD::TMDPDF &self,
               PDFxTMD::PartonFlavor flavor,
               double x,
               double kt2,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_kt2(kt2);
                    validate_mu2(mu2, "mu2");

                    return self.tmd(flavor, x, kt2, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating TMDPDF: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor"),
            py::arg("x"),
            py::arg("kt2"),
            py::arg("mu2"))
        .def(
            "tmd_all",
            [](PDFxTMD::TMDPDF &self,
               double x,
               double kt2,
               double mu2) {
                try
                {
                    validate_x(x, "x");
                    validate_kt2(kt2);
                    validate_mu2(mu2, "mu2");

                    std::array<double, 13> temp;
                    self.tmd(x, kt2, mu2, temp);

                    return std::vector<double>(temp.begin(), temp.end());
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating all TMDPDF flavors: " +
                        std::string(e.what()));
                }
            },
            py::arg("x"),
            py::arg("kt2"),
            py::arg("mu2"));

    py::class_<PDFxTMD::CollinearDPDF>(
        m,
        "CollinearDPDF",
        "Concrete collinear double PDF object")
        .def(
            py::init<const std::string &, int>(),
            py::arg("pdfSetName"),
            py::arg("setMember"))
        .def(
            "dpdf",
            [](PDFxTMD::CollinearDPDF &self,
               PDFxTMD::PartonFlavor flavor1,
               PDFxTMD::PartonFlavor flavor2,
               double x1,
               double mu1_2,
               double x2,
               double mu2_2,
               bool enforce_support) {
                try
                {
                    validate_dpdf_kinematics(x1, mu1_2, x2, mu2_2);

                    if (enforce_support && outside_dpdf_support(x1, x2))
                    {
                        return 0.0;
                    }

                    return self.dpdf(
                        flavor1,
                        flavor2,
                        x1,
                        mu1_2,
                        x2,
                        mu2_2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CollinearDPDF: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor1"),
            py::arg("flavor2"),
            py::arg("x1"),
            py::arg("mu1_2"),
            py::arg("x2"),
            py::arg("mu2_2"),
            py::arg("enforce_support") = true)
        .def(
            "dpdf_batch",
            [](PDFxTMD::CollinearDPDF &self,
               PDFxTMD::PartonFlavor flavor1,
               PDFxTMD::PartonFlavor flavor2,
               DoubleArray x1,
               DoubleArray mu1_2,
               DoubleArray x2,
               DoubleArray mu2_2,
               bool enforce_support) {
                try
                {
                    return dpdf_batch_numpy_impl(
                        self,
                        flavor1,
                        flavor2,
                        x1,
                        mu1_2,
                        x2,
                        mu2_2,
                        enforce_support);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CollinearDPDF NumPy batch: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor1"),
            py::arg("flavor2"),
            py::arg("x1"),
            py::arg("mu1_2"),
            py::arg("x2"),
            py::arg("mu2_2"),
            py::arg("enforce_support") = true,
            "Evaluate many DPDF points for one flavor pair from NumPy arrays.")
        .def(
            "dpdf_batch",
            [](PDFxTMD::CollinearDPDF &self,
               PDFxTMD::PartonFlavor flavor1,
               PDFxTMD::PartonFlavor flavor2,
               const std::vector<double> &x1,
               const std::vector<double> &mu1_2,
               const std::vector<double> &x2,
               const std::vector<double> &mu2_2,
               bool enforce_support) {
                try
                {
                    validate_same_size(x1, mu1_2, x2, mu2_2);

                    std::vector<double> values;
                    values.reserve(x1.size());

                    for (std::size_t i = 0; i < x1.size(); ++i)
                    {
                        validate_dpdf_kinematics(
                            x1[i],
                            mu1_2[i],
                            x2[i],
                            mu2_2[i]);

                        if (enforce_support && outside_dpdf_support(x1[i], x2[i]))
                        {
                            values.push_back(0.0);
                        }
                        else
                        {
                            values.push_back(
                                self.dpdf(
                                    flavor1,
                                    flavor2,
                                    x1[i],
                                    mu1_2[i],
                                    x2[i],
                                    mu2_2[i]));
                        }
                    }

                    return values;
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CollinearDPDF batch: " +
                        std::string(e.what()));
                }
            },
            py::arg("flavor1"),
            py::arg("flavor2"),
            py::arg("x1"),
            py::arg("mu1_2"),
            py::arg("x2"),
            py::arg("mu2_2"),
            py::arg("enforce_support") = true);
}