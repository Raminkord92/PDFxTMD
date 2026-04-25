cmake_minimum_required(VERSION 3.14)
include(cmake/version.cmake)
include(CheckLanguage)
project(PDFxTMDLib VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} LANGUAGES CXX)

option(ENABLE_FORMATTING "Enable code formatting" OFF)
option(ENABLE_DOCUMENTATION "Enable documentation generation" OFF)
option(ENABLE_BUILDING_WRAPPERS "Enable building wrappers" OFF)
option(ENABLE_BUILDING_EXAMPLES "Enable building examples" ON)

find_library(ZSTD_LIBRARY NAMES zstd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Check if the target is x86
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i[3-6]86")
    set(IS_X86 TRUE)
else()
    set(IS_X86 FALSE)
endif()

if (NOT WIN32)
    check_language(Fortran)
    if (CMAKE_Fortran_COMPILER)
        enable_language(Fortran OPTIONAL)
    endif()
endif()

include(GNUInstallDirs)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(IS_X86)
        add_compile_options(-mavx2 -mfma)
    endif()
    #    add_compile_options("$<$<CONFIG:Release>:-O3>")
    add_compile_options("$<$<CONFIG:Debug>:-O0;-g>")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        add_compile_options("$<$<CONFIG:Release>:-flto=auto>")
    else()
        add_compile_options("$<$<CONFIG:Release>:-flto>")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(IS_X86)
        add_compile_options(/arch:AVX2)
    endif()
else()
    message(WARNING "Compiler ${CMAKE_CXX_COMPILER_ID} not fully optimized. Add SIMD flags if needed.")
endif()

set(ALL_SOURCES
    src/Common/MathUtils.cpp
    src/Common/PartonUtils.cpp 
    src/Common/YamlMetaInfo/YamlCouplingInfo.cpp
    src/Common/YamlMetaInfo/YamlImpelemntationInfo.cpp
    src/Common/YamlMetaInfo/YamlStandardPDFInfo.cpp
    src/Common/YamlMetaInfo/YamlErrorInfo.cpp
    src/Common/PDFErrInfo.cpp
    src/Common/DPDF/UnequalScaleGrid.cpp
    src/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.cpp
    src/Implementation/Reader/TMD/SPDF/TDefaultLHAPDF_TMDReader.cpp
    src/Implementation/Reader/TMD/SPDF/TDefaultAllFlavorReader.cpp
    src/Implementation/Reader/Collinear/DPDF/CDefaultDPDFReader.cpp
    src/Factory.cpp
    src/FortranFactoryWrapper.cpp
    src/Common/FileUtils.cpp
    src/Common/AllFlavorsShape.cpp
    src/Uncertainty/HessianStrategy.cpp
    src/Uncertainty/ReplicasPercentileStrategy.cpp
    src/Uncertainty/ReplicasStdDevStrategy.cpp
    src/Uncertainty/SymmHessianStrategy.cpp
    src/Implementation/Coupling/ODE/ODEQCDCoupling.cpp
    src/Implementation/Coupling/Analytic/AnalyticQCDCoupling.cpp
    src/Implementation/Coupling/Interpolation/InterpolateQCDCoupling.cpp
    src/Common/ConfigWrapper.cpp
)
#for M_PI
add_definitions(-D_USE_MATH_DEFINES)

if(WIN32)
    add_library(${PROJECT_NAME} STATIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
    target_sources(${PROJECT_NAME} PRIVATE ${ALL_SOURCES})
    target_link_libraries(${PROJECT_NAME})
    target_include_directories(${PROJECT_NAME} 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
else()
    add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ZSTD_LIBRARY})

    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
    target_include_directories(${PROJECT_NAME} 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    add_library(${PROJECT_NAME}_static STATIC ${ALL_SOURCES})
    set_target_properties(${PROJECT_NAME}_static PROPERTIES 
        OUTPUT_NAME ${PROJECT_NAME}
        DEBUG_POSTFIX "d"
    )
    target_link_libraries(${PROJECT_NAME}_static  PRIVATE PRIVATE ${ZSTD_LIBRARY})
    target_include_directories(${PROJECT_NAME}_static 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif()

if(WIN32)
    set(DEFAULT_ENV_PATH "C:/ProgramData/PDFxTMDLib")
else()
    set(DEFAULT_ENV_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/PDFxTMDLib")
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
        DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}"
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
        DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}"
    )
    target_compile_definitions(${PROJECT_NAME}_static PRIVATE 
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
        DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}"
    )
endif()

if(ENABLE_BUILDING_WRAPPERS)
    add_subdirectory(wrappers)
endif()
include(cmake/Install.cmake)

if(ENABLE_FORMATTING)
    include(cmake/Format.cmake)
    Format(${PROJECT_NAME} include/)
    Format(${PROJECT_NAME} src/)
endif()

if(ENABLE_DOCUMENTATION)
    include(cmake/Doxygen.cmake)
    Doxygen(${PROJECT_NAME} include/)
endif()

if(ENABLE_BUILDING_EXAMPLES)
    add_subdirectory(examples)
endif()
