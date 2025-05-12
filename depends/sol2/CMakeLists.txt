project(Sol2)

add_library(Sol2 INTERFACE)

file(GLOB SOL2_FILES sol/*.hpp)

target_sources(Sol2 INTERFACE ${SOL2_FILES})

target_include_directories(Sol2 SYSTEM INTERFACE .)

# Using Lua
target_link_libraries(Sol2 INTERFACE Lua)
