PROJECT(OpenCV_Test)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c17")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

IF (BUILD_MODE STREQUAL "debug")
	MESSAGE(STATUS "****** BUILD MODE: DEBUG ******")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -O2")
ELSE()
	MESSAGE(STATUS "====== BUILD MODE: REALSE ======")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")
ENDIF()

MESSAGE(STATUS "project source dir: ${PROJECT_SOURCE_DIR}")
SET(PATH_TEST_FILES ${PROJECT_SOURCE_DIR}/./../../demo/OpenCV_Test)
SET(PATH_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/fbc_cv)
SET(PATH_LIBEXIF_SRC_FILES ${PROJECT_SOURCE_DIR}/../../src/libexif)
MESSAGE(STATUS "path src files: ${PATH_TEST_FILES}")

FIND_PACKAGE(OpenCV)

# head file search path
INCLUDE_DIRECTORIES(
	${PATH_TEST_FILES}
	${PATH_SRC_FILES}/include
	${OpenCV_INCLUDE_DIRS}
	${PATH_LIBEXIF_SRC_FILES}
)

LINK_DIRECTORIES(
	${OpenCV_LIB_DIR}
	${PROJECT_SOURCE_DIR}/build
)

# recursive query match files :*.cpp, *c
FILE(GLOB_RECURSE TEST_CPP_LIST ${PATH_TEST_FILES}/*.cpp)
FILE(GLOB_RECURSE TEST_C_LIST ${PATH_TEST_FILES}/*.c)
#MESSAGE(STATUS "cpp list: ${TEST_CPP_LIST} ${TEST_C_LIST}")

FILE(GLOB_RECURSE SRC_CPP_LIST ${PATH_SRC_FILES}/*.cpp)
#MESSAGE(STATUS "src cpp list: ${SRC_CPP_LIST}")

# generate dynamic library for fbc_cv
ADD_LIBRARY(fbc_cv SHARED ${SRC_CPP_LIST})

# build executable program
ADD_EXECUTABLE(OpenCV_Test ${TEST_CPP_LIST} ${TEST_C_LIST})
# add dependent library: static and dynamic
TARGET_LINK_LIBRARIES(OpenCV_Test fbc_cv ${OpenCV_LIBS} exif pthread)
