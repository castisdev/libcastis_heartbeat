cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME hb_asio_base)
project(${PROJECT_NAME})
find_package(Boost)
# set(Boost_USE_MULTITHREADED TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
  ${Boost_INCLUDE_DIRS}
)
if(WIN32)
add_definitions(-D_AFXDLL -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else(WIN32)
add_definitions(-Wall -g -Os -fPIC -fno-strict-aliasing -D_REENTRANT -D_GNU_SOURCE)
endif(WIN32)

file(GLOB CIHB_SOURCES "*.h" "*.cpp")
#message("sources: " ${CIHB_SOURCES})
	
message(STATUS "CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}")
message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")
message(STATUS "Boost_LIBRARY_DIRS=${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
message(STATUS "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")

add_library(${PROJECT_NAME} ${CIHB_SOURCES})
#set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME})
