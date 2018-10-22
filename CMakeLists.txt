cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME service_monitor_heartbeat_lib)
project(${PROJECT_NAME})
FIND_PACKAGE(Boost 1.33.1)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
else(WIN32)
add_definitions(-Wno-unused-variable)
endif(WIN32)

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(asio_base)
add_subdirectory(legacy_compatible)
add_subdirectory(examples/asio_base EXCLUDE_FROM_ALL)
add_subdirectory(examples/legacy_compatible EXCLUDE_FROM_ALL)
add_subdirectory(examples/Externals/CiUtils EXCLUDE_FROM_ALL)
add_subdirectory(examples/legacy_compatible_with_ciutils EXCLUDE_FROM_ALL)
add_subdirectory(examples/legacy_compatible_complex_with_ciutils EXCLUDE_FROM_ALL)
