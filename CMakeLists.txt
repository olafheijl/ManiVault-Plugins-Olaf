cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED MV_EXAMPLES_USE_VCPKG)
    set(MV_EXAMPLES_USE_VCPKG OFF)
elseif(MV_EXAMPLES_USE_VCPKG)
    message(STATUS "DVRPlugins: Using vcpkg to install dependencies")
endif()

# -----------------------------------------------------------------------------
# Plugins
# -----------------------------------------------------------------------------
set(PROJECT "DVRPlugins")
PROJECT(${PROJECT})

add_subdirectory(VolumeDataPlugin)
add_subdirectory(DVRVolumeLoaderPlugin)
add_subdirectory(DVRViewPlugin)
add_subdirectory(DVRTransferFunctionPlugin)

set_target_properties(VolumeDataPlugin DVRVolumeLoaderPlugin DVRViewPlugin DVRTransferFunctionPlugin
    PROPERTIES
    FOLDER DVRPlugins
)