cmake_minimum_required(VERSION 3.15)
project(TempMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/TempMonitor.cpp
    src/FloatingWindow.cpp
    src/TrayIcon.cpp
    src/SettingsDialog.cpp
)

set(HEADERS
    src/Config.h
    src/TempMonitor.h
    src/FloatingWindow.h
    src/TrayIcon.h
    src/SettingsDialog.h
    src/resource.h
)

# Resource file
set(RESOURCES
    resources/app.rc
)

# Add executable
add_executable(TempMonitor WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Include directories
target_include_directories(TempMonitor PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link libraries
target_link_libraries(TempMonitor
    gdi32
    gdiplus
    shell32
    ole32
    oleaut32
    wbemuuid
    comctl32
)

# Set subsystem to Windows (no console)
if(MSVC)
    set_target_properties(TempMonitor PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup"
    )
endif()

# Copy nvml.dll to output directory if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/lib/nvml.dll")
    add_custom_command(TARGET TempMonitor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/nvml.dll"
        $<TARGET_FILE_DIR:TempMonitor>
    )
endif()
