cmake_minimum_required(VERSION 3.14)

project(server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPENCV_PATH C:/Users/c/opencv/build)
set(FFMPEG_PATH C:/Users/c/lib/ffmpeg)
set(ASIO_PATH C:/Users/c/lib/asio)

file(GLOB_RECURSE FILES "source/*.cpp" "header/*.h" "header/*.hpp")
file(GLOB_RECURSE ASIO ${ASIO_PATH}/include/asio/*.* ${ASIO_PATH}/include/asio/*/*.* ${ASIO_PATH}/include/asio.hpp)
file(GLOB_RECURSE FFMPEG_FILE ${FFMPEG_PATH}/include/*/*)
file(GLOB_RECURSE FFMPEG_LIBS #[[${FFMPEG_PATH}/msvc_2019/bin/*.lib]] ${FFMPEG_PATH}/lib/*.lib)

find_package(OpenCV REQUIRED PATHS ${OPENCV_PATH})
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Sql Core Network Multimedia Core5Compat HttpServer)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql Core Network Multimedia Core5Compat HttpServer)

add_executable(server
  ${FILES}
  ${FFMPEG_FILE}
  ${ASIO}
)
target_include_directories(server
    PUBLIC
    header
    ${ASIO_PATH}/include/asio
    ${ASIO_PATH}/include
    ${OPENCV_PATH}/include
    ${OPENCV_PATH}/include/opencv2
    ${FFMPEG_PATH}/include
)
target_link_libraries(server
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Core5Compat
    Qt${QT_VERSION_MAJOR}::HttpServer
)
target_link_libraries(server ${OpenCV_LIBS} ${FFMPEG_LIBS})
include(GNUInstallDirs)
install(TARGETS server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
