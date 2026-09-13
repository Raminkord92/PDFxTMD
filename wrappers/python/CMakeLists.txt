message(STATUS "Detecting pybind11...")
find_package(pybind11 QUIET)
if(NOT pybind11_FOUND)
  include(FetchContent)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  message(STATUS "pybind11 not detected, fetching pybind11...")
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG v3.0.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)
  FetchContent_MakeAvailable(pybind11)
endif()

pybind11_add_module(pdfxtmd pyBindings.cpp)

# setup.py passes the exact directory where setuptools expects the extension.
# Explicit per-configuration properties are needed for multi-config generators
# such as Visual Studio, which would otherwise add a Release/Debug subfolder.
if(DEFINED PDFXTMD_PYTHON_OUTPUT_DIR)
  set_target_properties(pdfxtmd PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PDFXTMD_PYTHON_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PDFXTMD_PYTHON_OUTPUT_DIR}"
  )
endif()

target_include_directories(pdfxtmd PUBLIC ${PROJECT_SOURCE_DIR}/include)

# The Python extension must use CMake's native target graph rather than linking
# an installed archive manually. On Unix-like systems use the static target so
# the wheel does not depend on a separately installed libPDFxTMDLib.so. CMake
# then propagates the native DPD link requirements (Zstandard, oneDNN, OpenMP)
# to the extension's final link step.
if(WIN32)
  set(PDFXTMD_PYTHON_NATIVE_TARGET ${PROJECT_NAME})
else()
  set(PDFXTMD_PYTHON_NATIVE_TARGET ${PROJECT_NAME}_static)
endif()

target_link_libraries(pdfxtmd PRIVATE ${PDFXTMD_PYTHON_NATIVE_TARGET})

if(NOT ENABLE_DPD)
  target_compile_definitions(pdfxtmd PRIVATE PDFXTMD_DISABLE_DPD=1)
endif()
