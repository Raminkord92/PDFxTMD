#pragma once

#include "PDFxTMDLib/PDFSet.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Members belong to PDFSet's unique_ptr map. reference_internal keeps the
// owning Python set alive, including for CPDFSet(name)[0] temporaries. Do not
// expose member-replacement/reinitialization methods: they invalidate pointers.
template <typename Tag>
void bind_pdf_set(py::module_ &m, const char *name)
{
    using Set = PDFxTMD::PDFSet<Tag>;
    py::class_<Set> cls(m, name);
    cls.def(py::init<std::string, bool>(), py::arg("pdfSetName"),
            py::arg("alternativeReplicaUncertainty") = false)
        .def("__len__", &Set::size)
        .def("__getitem__", [](Set &self, py::ssize_t member) -> typename Set::PDF_t * {
            if (member < 0 || static_cast<std::size_t>(member) >= self.size())
                throw py::index_error("PDF member index out of range");
            return self[static_cast<int>(member)];
        }, py::arg("member"), py::return_value_policy::reference_internal,
        "Return a member owned by this set. Indices must be in [0, len(set)).")
        .def("alphasQ", &Set::alphasQ, py::arg("q"))
        .def("alphasQ2", &Set::alphasQ2, py::arg("q2"))
        .def("getStdPDFInfo", &Set::getStdPDFInfo)
        .def("getPDFErrorInfo", &Set::getPDFErrorInfo)
        .def("Uncertainty", [](Set &self, const std::vector<double> &values, double cl) {
            return self.Uncertainty(values, cl);
        }, py::arg("values"), py::arg("cl") = NO_REQUESTED_CONFIDENCE_LEVEL)
        .def("Correlation", [](const Set &self, const std::vector<double> &valuesA,
                               const std::vector<double> &valuesB) {
            return self.Correlation(valuesA, valuesB);
        }, py::arg("valuesA"), py::arg("valuesB"));

    if constexpr (std::is_same_v<Tag, PDFxTMD::CollinearPDFTag>)
    {
        cls.def("Uncertainty", [](Set &self, PDFxTMD::PartonFlavor flavor,
                                  double x, double mu2, double cl) {
            return self.Uncertainty(flavor, x, mu2, cl);
        }, py::arg("flavor"), py::arg("x"), py::arg("mu2"),
        py::arg("cl") = NO_REQUESTED_CONFIDENCE_LEVEL)
        .def("Correlation", [](Set &self, PDFxTMD::PartonFlavor flavorA,
                               double xA, double mu2A, PDFxTMD::PartonFlavor flavorB,
                               double xB, double mu2B) {
            return self.Correlation(flavorA, xA, mu2A, flavorB, xB, mu2B);
        }, py::arg("flavorA"), py::arg("xA"), py::arg("mu2A"),
        py::arg("flavorB"), py::arg("xB"), py::arg("mu2B"));
    }
    else
    {
        cls.def("Uncertainty", [](Set &self, PDFxTMD::PartonFlavor flavor,
                                  double x, double kt2, double mu2, double cl) {
            return self.Uncertainty(flavor, x, kt2, mu2, cl);
        }, py::arg("flavor"), py::arg("x"), py::arg("kt2"), py::arg("mu2"),
        py::arg("cl") = NO_REQUESTED_CONFIDENCE_LEVEL)
        .def("Correlation", [](Set &self, PDFxTMD::PartonFlavor flavorA,
                               double xA, double kt2A, double mu2A,
                               PDFxTMD::PartonFlavor flavorB,
                               double xB, double kt2B, double mu2B) {
            return self.Correlation(flavorA, xA, kt2A, mu2A, flavorB, xB, kt2B, mu2B);
        }, py::arg("flavorA"), py::arg("xA"), py::arg("kt2A"), py::arg("mu2A"),
        py::arg("flavorB"), py::arg("xB"), py::arg("kt2B"), py::arg("mu2B"));
    }
}

inline void bind_pdf_sets(py::module_ &m)
{
    using namespace PDFxTMD;
    py::class_<PDFUncertainty>(m, "PDFUncertainty")
        .def(py::init<>())
        .def_readwrite("central", &PDFUncertainty::central)
        .def_readwrite("errplus", &PDFUncertainty::errplus)
        .def_readwrite("errminus", &PDFUncertainty::errminus)
        .def_readwrite("errsymm", &PDFUncertainty::errsymm)
        .def_readwrite("scale", &PDFUncertainty::scale)
        .def_readwrite("errplus_pdf", &PDFUncertainty::errplus_pdf)
        .def_readwrite("errminus_pdf", &PDFUncertainty::errminus_pdf)
        .def_readwrite("errsymm_pdf", &PDFUncertainty::errsymm_pdf)
        .def_readwrite("errplus_par", &PDFUncertainty::errplus_par)
        .def_readwrite("errminus_par", &PDFUncertainty::errminus_par)
        .def_readwrite("errsymm_par", &PDFUncertainty::errsymm_par)
        .def_readwrite("errparts", &PDFUncertainty::errparts);

    py::enum_<OrderQCD>(m, "OrderQCD")
        .value("LO", OrderQCD::LO).value("NLO", OrderQCD::NLO)
        .value("N2LO", OrderQCD::N2LO).value("N3LO", OrderQCD::N3LO)
        .value("N4LO", OrderQCD::N4LO);
    py::class_<YamlStandardPDFInfo>(m, "YamlStandardPDFInfo")
        .def_readonly("orderQCD", &YamlStandardPDFInfo::orderQCD)
        .def_readonly("XMin", &YamlStandardPDFInfo::XMin)
        .def_readonly("XMax", &YamlStandardPDFInfo::XMax)
        .def_readonly("QMin", &YamlStandardPDFInfo::QMin)
        .def_readonly("QMax", &YamlStandardPDFInfo::QMax)
        .def_readonly("NumMembers", &YamlStandardPDFInfo::NumMembers)
        .def_readonly("Flavors", &YamlStandardPDFInfo::Flavors)
        .def_readonly("Format", &YamlStandardPDFInfo::Format)
        .def_readonly("SetDesc", &YamlStandardPDFInfo::SetDesc)
        .def_readonly("lhapdfID", &YamlStandardPDFInfo::lhapdfID);
    py::class_<YamlStandardTMDInfo, YamlStandardPDFInfo>(m, "YamlStandardTMDInfo")
        .def_readonly("KtMin", &YamlStandardTMDInfo::KtMin)
        .def_readonly("KtMax", &YamlStandardTMDInfo::KtMax)
        .def_readonly("TMDScheme", &YamlStandardTMDInfo::TMDScheme)
        .def_readonly("HybridCacheCapacity", &YamlStandardTMDInfo::HybridCacheCapacity)
        .def_readonly("HybridThreads", &YamlStandardTMDInfo::HybridThreads);
    py::class_<YamlErrorInfo>(m, "YamlErrorInfo")
        .def_readonly("ErrorConfLevel", &YamlErrorInfo::ErrorConfLevel)
        .def_readonly("ErrorType", &YamlErrorInfo::ErrorType)
        .def_readonly("errorSize", &YamlErrorInfo::errorSize)
        .def_readonly("size", &YamlErrorInfo::size);

    bind_pdf_set<CollinearPDFTag>(m, "CPDFSet");
    bind_pdf_set<TMDPDFTag>(m, "TMDSet");
}
