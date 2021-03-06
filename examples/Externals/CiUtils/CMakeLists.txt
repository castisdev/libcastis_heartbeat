cmake_minimum_required(VERSION 3.16.0)
SET(PROJECT_NAME CiUtils)
PROJECT(${PROJECT_NAME})
set(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost 1.33.1 COMPONENTS filesystem REQUIRED)
# SET(Boost_USE_MULTITHREADED TRUE)
# ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
# FIND_PACKAGE(Boost 1.41 COMPONENTS filesystem REQUIRED)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(
	${Boost_INCLUDE_DIRS}
	Externals/Include
	Include
	Src
)
IF(WIN32)
ADD_DEFINITIONS(-D_AFXDLL -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
ELSE(WIN32)
ADD_DEFINITIONS(-Wall -g -Os -fPIC -fno-strict-aliasing -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
ENDIF(WIN32)
SET(SRCS
	Src/CiSafeString.cpp
	Src/CiMutex.cpp
	Src/CiSemaphore.cpp
	Src/CiSocket.cpp
	Src/CiThread2.cpp
	Src/CiThreadComplex.cpp
	Src/CiThreadRealTimeComplex.cpp
	Src/CiUtils.cpp
	Src/MTime2.cpp
	Src/MTimeSpan2.cpp
	Src/NetUtil.cpp
	Src/NetworkThread.cpp
	Src/RealTimeFlavor.cpp
)

ADD_LIBRARY(${PROJECT_NAME} ${SRCS})
