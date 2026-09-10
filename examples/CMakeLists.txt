cmake_minimum_required(VERSION 3.10)
project(Examples LANGUAGES CXX)

add_executable(tutorial tutorial.cpp)
target_link_libraries(tutorial PRIVATE PDFxTMDLib)
target_include_directories(tutorial PRIVATE "../include")

add_executable(CPDFSet_tutorial CPDFSet_tutorial.cpp)
target_link_libraries(CPDFSet_tutorial PRIVATE PDFxTMDLib)
target_include_directories(CPDFSet_tutorial PRIVATE "../include")


add_executable(TMDSet_tutorial TMDSet_tutorial.cpp)
target_link_libraries(TMDSet_tutorial PRIVATE PDFxTMDLib)
target_include_directories(TMDSet_tutorial PRIVATE "../include")

add_executable(Uncertainty_tutorial Uncertainty_tutorial.cpp)
target_link_libraries(Uncertainty_tutorial PRIVATE PDFxTMDLib)
target_include_directories(Uncertainty_tutorial PRIVATE "../include")

add_executable(cPDF_TMD_Factory_tutorial cPDF_TMD_Factory_tutorial.cpp)
target_link_libraries(cPDF_TMD_Factory_tutorial PRIVATE PDFxTMDLib)
target_include_directories(cPDF_TMD_Factory_tutorial PRIVATE "../include")

add_executable(QCDCoupling_tutorial QCDCoupling_tutorial.cpp)
target_link_libraries(QCDCoupling_tutorial PRIVATE PDFxTMDLib)
target_include_directories(QCDCoupling_tutorial PRIVATE "../include")

add_executable(AdvancedUsage_tutorial AdvancedUsage_tutorial.cpp)
target_link_libraries(AdvancedUsage_tutorial PRIVATE PDFxTMDLib)
target_include_directories(AdvancedUsage_tutorial PRIVATE "../include")

add_executable(DPDFTutorial DPDFTutorial.cpp)
target_link_libraries(DPDFTutorial PRIVATE PDFxTMDLib)
target_include_directories(DPDFTutorial PRIVATE "../include")

if (NOT WIN32)
    if (CMAKE_Fortran_COMPILER)
        add_subdirectory(Fortran)
    endif()
else()
    message(STATUS "Fortran not found. Fortran example is not going to be compiled")
endif()
