project(xen_doc)

if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not found, needed to generate documentation.")
endif ()

add_custom_target(
    xen_doc

    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generate documentation for xen"
    VERBATIM
)
