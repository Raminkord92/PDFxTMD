# Installation rules for PDFxTMDLib.
#
# This file contains only CMake installation and package-export logic.

include(CMakePackageConfigHelpers)

# -----------------------------------------------------------------------------
# Install library targets
# -----------------------------------------------------------------------------

if(WIN32)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT pdfxtmdlib-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    install(
        TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static
        EXPORT pdfxtmdlib-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# -----------------------------------------------------------------------------
# Export CMake targets
# -----------------------------------------------------------------------------

install(
    EXPORT pdfxtmdlib-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NAMESPACE PDFxTMD::
    FILE pdfxtmdlib-targets.cmake
)

# -----------------------------------------------------------------------------
# Generate the CMake package configuration
# -----------------------------------------------------------------------------

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/pdfxtmdlib-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pdfxtmdlib-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pdfxtmdlib-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pdfxtmdlib-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/pdfxtmdlib-config-version.cmake
        ${PROJECT_SOURCE_DIR}/cmake/FindPDFxTMDZstd.cmake
        ${PROJECT_SOURCE_DIR}/cmake/FindPDFxTMDDNNL.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# -----------------------------------------------------------------------------
# Install public headers
# -----------------------------------------------------------------------------

message(
    STATUS
    "PDFxTMD headers: ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} "
    "-> ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# -----------------------------------------------------------------------------
# Create the shared PDFxTMD data directory
# -----------------------------------------------------------------------------

install(
    DIRECTORY
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}
)
