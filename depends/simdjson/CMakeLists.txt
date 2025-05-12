project(simdjson)

add_library(simdjson OBJECT)

target_compile_features(simdjson PRIVATE cxx_std_20)

set(
    SIMDJSON_FILES

    simdjson.cpp
    simdjson.h
)

target_sources(simdjson PRIVATE ${SIMDJSON_FILES})

target_include_directories(simdjson SYSTEM PUBLIC .)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(simdjson PRIVATE /w)
else ()
    target_compile_options(simdjson PRIVATE -w)
endif ()
