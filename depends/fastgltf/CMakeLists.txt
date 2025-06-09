project(fastgltf)

add_library(fastgltf OBJECT)

target_compile_features(fastgltf PRIVATE cxx_std_20)

file(
    GLOB
    FASTGLTF_FILES

    src/*.cpp
    "include/fastgltf/*.hpp"
)

target_sources(fastgltf PRIVATE ${FASTGLTF_FILES})

target_include_directories(fastgltf SYSTEM PUBLIC include)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(fastgltf PRIVATE /w)
else ()
    target_compile_options(fastgltf PRIVATE -w)
endif ()

# Using simdjson
target_link_libraries(fastgltf PRIVATE simdjson)
