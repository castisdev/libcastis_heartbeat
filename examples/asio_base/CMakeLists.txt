cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME example_asio_base)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.40.0 COMPONENTS system date_time regex REQUIRED)
project(${PROJECT_NAME})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
  ${Boost_INCLUDE_DIRS}
  ../../asio_base
)
if(WIN32)
add_definitions(-D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else(WIN32)
add_definitions(-Wall -g -Os -fPIC -fno-strict-aliasing -D_REENTRANT -D_GNU_SOURCE)
endif(WIN32)

file(GLOB SRCS "*.h" "*.cpp")
	
add_executable(${PROJECT_NAME} ${SRCS})
if(WIN32)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} hb_asio_base)
else(WIN32)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} hb_asio_base pthread)
endif(WIN32)
