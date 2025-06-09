project(Tracy)

add_library(Tracy STATIC)

target_compile_features(Tracy PRIVATE cxx_std_20)

target_sources(Tracy PRIVATE TracyClient.cpp)

target_include_directories(Tracy SYSTEM PUBLIC .)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(Tracy PRIVATE /w)
else ()
    target_compile_options(Tracy PRIVATE -w)
endif ()