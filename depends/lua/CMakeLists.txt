project(Lua)

add_library(Lua OBJECT)

target_compile_features(Lua PRIVATE c_std_11)

file(
    GLOB
    LUA_FILES

    *.c
    *.h
)

target_sources(Lua PRIVATE ${LUA_FILES})

target_include_directories(Lua SYSTEM PUBLIC .)

# Disabling all compilers warnings
if (XEN_IS_COMPILER_MSVC)
    target_compile_options(Lua PRIVATE /w)
else ()
    target_compile_options(Lua PRIVATE -w)
endif ()
