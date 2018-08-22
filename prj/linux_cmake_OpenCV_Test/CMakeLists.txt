PROJECT(OpenCV_Test)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# support C++11
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# support C++14, when gcc version > 5.1, use -std=c++14 instead of c++1y
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

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
MESSAGE(STATUS "path src files: ${PATH_TEST_FILES}")

# head file search path
INCLUDE_DIRECTORIES(
	${PATH_TEST_FILES}
	${PATH_SRC_FILES}/include
	/opt/opencv3.1/include
)

# find library
FIND_LIBRARY(opencv_core NAMES opencv_core PATHS /opt/opencv3.1/lib NO_DEFAULT_PATH)
FIND_LIBRARY(opencv_imgproc NAMES opencv_imgproc PATHS /opt/opencv3.1/lib NO_DEFAULT_PATH)
FIND_LIBRARY(opencv_highgui NAMES opencv_highgui PATHS /opt/opencv3.1/lib NO_DEFAULT_PATH)
FIND_LIBRARY(opencv_imgcodecs NAMES opencv_imgcodecs PATHS /opt/opencv3.1/lib NO_DEFAULT_PATH)
FIND_LIBRARY(opencv_video NAMES opencv_video PATHS /opt/opencv3.1/lib NO_DEFAULT_PATH)
FIND_LIBRARY(opencv_videoio NAMES opencv_videoio PATHS /opt/opencv3.1/lib NO_DEFAULT_PATH)
MESSAGE(STATUS "opencv libraries: ${opencv_core} ${opencv_imgproc} ${opencv_highgui} ${opencv_imgcodecs} ${opencv_video}" ${opencv_videoio})

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
TARGET_LINK_LIBRARIES(OpenCV_Test fbc_cv ${opencv_core} ${opencv_imgproc} ${opencv_highgui} ${opencv_imgcodecs} ${opencv_video} ${opencv_videoio} pthread)

