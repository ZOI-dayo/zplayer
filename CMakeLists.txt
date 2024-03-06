cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
project(zplayer)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INC_DIR ${PROJECT_SOURCE_DIR}/include)
add_definitions( -std=c++17 -Wall )

find_package(Threads REQUIRED)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)

add_executable(
  zplayer
  ${SRC_DIR}/videodecoder.cpp
  ${SRC_DIR}/main.cpp
)
target_include_directories(
  zplayer
  PRIVATE
  ${INC_DIR}
  ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR}
${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR}
)
target_link_libraries(
  zplayer
  PRIVATE
  ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY}
  ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} Threads::Threads
)
