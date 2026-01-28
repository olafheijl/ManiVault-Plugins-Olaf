 cmake_minimum_required(VERSION 3.22)
 
option(MV_UNITY_BUILD "Combine target source files into batches for faster compilation" OFF)

set(VOLUMEDATA "VolumeDataPlugin")
PROJECT(${VOLUMEDATA} LANGUAGES CXX)

# -----------------------------------------------------------------------------
# CMake Options and Setup
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

# Include cmake tools
include(GenerateExportHeader)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

find_package(ManiVault COMPONENTS Core PointData CONFIG QUIET)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
# Define the plugin sources
set(VOLUMEDATA_SOURCES
    src/Size3D.cpp
    src/Volume.cpp
    src/VolumeData.cpp
    src/Volumes.cpp
    src/InfoAction.cpp
    src/Volume.h
    src/VolumeData.h
    src/Volumes.h
    src/InfoAction.h
    src/Size3D.h
    PluginInfo.json
)

set(VOLUMEDATA_HEADERS
    src/Volume.h
    src/VolumeData.h
    src/Volumes.h
    src/InfoAction.h
    src/Size3D.h
)

source_group(Plugin FILES ${VOLUMEDATA_SOURCES})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
# Create dynamic library for the plugin
add_library(${VOLUMEDATA} SHARED ${VOLUMEDATA_SOURCES})

target_sources(${VOLUMEDATA} PRIVATE ${VOLUMEDATA_SOURCES})

# Generate a header file that contains the EXPORT macro for this library.
generate_export_header(${VOLUMEDATA} EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${VOLUMEDATA}_export.h)

# Add the generated header to the plugin's header files
list(APPEND VOLUMEDATA_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${VOLUMEDATA}_export.h)

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include ManiVault headers, including system data plugins
target_include_directories(${VOLUMEDATA} PRIVATE "${ManiVault_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++20
target_compile_features(${VOLUMEDATA} PRIVATE cxx_std_20)

set_target_properties(${VOLUMEDATA} PROPERTIES PUBLIC_HEADER "${VOLUMEDATA_HEADERS}")

if(MV_UNITY_BUILD)
    set_target_properties(${VOLUMEDATA} PROPERTIES UNITY_BUILD ON)
endif()

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
# Link to Qt libraries
target_link_libraries(${VOLUMEDATA} PRIVATE Qt6::Widgets)
target_link_libraries(${VOLUMEDATA} PRIVATE Qt6::WebEngineWidgets)

# Link to ManiVault and data plugins
target_link_libraries(${VOLUMEDATA} PRIVATE ManiVault::Core)
target_link_libraries(${VOLUMEDATA} PRIVATE ManiVault::PointData)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
    ARCHIVE DESTINATION lib     COMPONENT LINKLIB # Windows .lib
    PUBLIC_HEADER DESTINATION include/${VOLUMEDATA} COMPONENT HEADERS
)

add_custom_command(TARGET ${VOLUMEDATA} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)


# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${VOLUMEDATA} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/Debug,$<IF:$<CONFIG:RELWITHDEBINFO>,${ManiVault_INSTALL_DIR}/RelWithDebInfo,${ManiVault_INSTALL_DIR}/Release>>)
    set_property(TARGET ${VOLUMEDATA} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/Debug/ManiVault Studio.exe",$<IF:$<CONFIG:RELWITHDEBINFO>,"${ManiVault_INSTALL_DIR}/RelWithDebInfo/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/Release/ManiVault Studio.exe">>)
endif()