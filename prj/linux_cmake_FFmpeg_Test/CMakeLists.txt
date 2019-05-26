PROJECT(FFmpeg_Test)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# support C++11
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# support C++14, when gcc version > 5.1, use -std=c++14 instead of c++1y
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -O2")

MESSAGE(STATUS "project source dir: ${PROJECT_SOURCE_DIR}")
SET(PATH_TEST_FILES ${PROJECT_SOURCE_DIR}/./../../demo/FFmpeg_Test)
SET(PATH_FFMPEG_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/ffmpeg)
SET(PATH_LIVE555_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/live555)
MESSAGE(STATUS "path src files: ${PATH_TEST_FILES}")

# head file search path
INCLUDE_DIRECTORIES(
	${PATH_TEST_FILES}
	${PATH_FFMPEG_SRC_FILES}/build/install/include
	${PATH_LIVE555_SRC_FILES}/liveMedia/include
	${PATH_LIVE555_SRC_FILES}/groupsock/include
	${PATH_LIVE555_SRC_FILES}/UsageEnvironment/include
	${PATH_LIVE555_SRC_FILES}/BasicUsageEnvironment/include
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

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/build)

# recursive query match files :*.cpp
FILE(GLOB_RECURSE TEST_CPP_LIST ${PATH_TEST_FILES}/*.cpp)
#MESSAGE(STATUS "cpp list: ${TEST_CPP_LIST})

# build executable program
ADD_EXECUTABLE(FFmpeg_Test ${TEST_CPP_LIST})
# add dependent library: static and dynamic
TARGET_LINK_LIBRARIES(FFmpeg_Test avcodec avdevice avfilter avformat swresample swscale avutil liveMedia groupsock BasicUsageEnvironment UsageEnvironment ${opencv_core} ${opencv_imgproc} ${opencv_highgui} ${opencv_imgcodecs} ${opencv_video} ${opencv_videoio} pthread)

