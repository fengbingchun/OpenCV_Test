PROJECT(FFmpeg_Test)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c17")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

MESSAGE(STATUS "project source dir: ${PROJECT_SOURCE_DIR}")
SET(PATH_TEST_FILES ${PROJECT_SOURCE_DIR}/./../../demo/FFmpeg_Test)
SET(PATH_FFMPEG_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/ffmpeg)
SET(PATH_LIVE555_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/live555)
SET(PATH_LIBUSB_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/libusb)
SET(PATH_LIBUVC_SRC_FILES ${PROJECT_SOURCE_DIR}/./../../src/libuvc)
MESSAGE(STATUS "path src files: ${PATH_TEST_FILES}")

FIND_PACKAGE(OpenCV)

# head file search path
INCLUDE_DIRECTORIES(
	${PATH_TEST_FILES}
	${PATH_FFMPEG_SRC_FILES}/build/install/include
	${PATH_LIVE555_SRC_FILES}/liveMedia/include
	${PATH_LIVE555_SRC_FILES}/groupsock/include
	${PATH_LIVE555_SRC_FILES}/UsageEnvironment/include
	${PATH_LIVE555_SRC_FILES}/BasicUsageEnvironment/include
	${PATH_LIBUSB_SRC_FILES}/libusb
	${PATH_LIBUVC_SRC_FILES}/include
	${PATH_LIBUVC_SRC_FILES}/build/include
	${OpenCV_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${OpenCV_LIB_DIR}
	${PROJECT_SOURCE_DIR}/build
)

# recursive query match files :*.cpp
FILE(GLOB_RECURSE TEST_CPP_LIST ${PATH_TEST_FILES}/*.cpp)
#MESSAGE(STATUS "cpp list: ${TEST_CPP_LIST})

# build executable program
ADD_EXECUTABLE(FFmpeg_Test ${TEST_CPP_LIST})
# add dependent library: static and dynamic
TARGET_LINK_LIBRARIES(FFmpeg_Test avformat avcodec avdevice avfilter avformat swresample swscale avutil liveMedia groupsock BasicUsageEnvironment UsageEnvironment uvc usb-1.0 ${OpenCV_LIBS} lzma X11 dl Xext z SDL2 sndio asound xcb xcb-shm xcb-shape xcb-xfixes Xv pthread udev)
