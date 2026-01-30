cmake_minimum_required(VERSION 3.10)
project(TempMonitor)

set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Link libraries
# WbemUuid.lib is needed for WMI
# Comctl32.lib is needed for common controls
add_executable(TempMonitor WIN32 ${SOURCES})

if(MSVC)
    target_link_libraries(TempMonitor PRIVATE WbemUuid.lib Comctl32.lib)
    target_compile_options(TempMonitor PRIVATE /W4 /EHsc)
endif()
