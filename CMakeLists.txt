cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME service_monitor_heartbeat_lib)
project(${PROJECT_NAME})

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(legacy_compatible)
add_subdirectory(examples/legacy_compatible)
add_subdirectory(examples/Externals/CiUtils)
add_subdirectory(examples/legacy_compatible_with_ciutils)
add_subdirectory(examples/legacy_compatible_complex_with_ciutils)
