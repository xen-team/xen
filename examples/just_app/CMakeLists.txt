cmake_minimum_required (VERSION 3.16)

project (xen-just-app-example)

add_library(xen_examples INTERFACE)
target_compile_features(xen_examples INTERFACE cxx_std_17)
target_link_libraries(xen_examples INTERFACE xen)
target_sources(xen_examples INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/demo_utils.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/demo_utils.hpp")
target_compile_options(xen_examples INTERFACE ${RAZ_EXAMPLES_DISABLED_WARNINGS})

include(CompilerFlags)
add_compiler_flags(TARGET xen_examples SCOPE INTERFACE ${SANITIZERS_OPTION})

add_executable(xen_full_example main.cpp)
target_link_libraries(xen_full_example xen_examples)
list(APPEND EXAMPLE_TARGETS xen_full_example)

# target_link_libraries(${PROJECT_NAME} xen)