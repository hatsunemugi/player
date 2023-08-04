cmake_minimum_required(VERSION 3.16)

project(aplayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPENCV_PATH C:/Users/c/opencv/build)
set(FFMPEG_PATH C:/Users/c/lib/ffmpeg)

find_package(OpenCV REQUIRED PATHS ${OPENCV_PATH})

file(GLOB_RECURSE SOURCE_FILE "header/*.h" "header/*.hpp" "source/*.cpp")
file(GLOB_RECURSE FFMPEG_FILE ${FFMPEG_PATH}/include/*/*)
file(GLOB_RECURSE FFMPEG_LIBS #[[${FFMPEG_PATH}/msvc_2019/bin/*.lib]] ${FFMPEG_PATH}/lib/*.lib)
file(GLOB IMAGES "image/*/*.*")
file(GLOB VALUES "values/*.qml")
file(GLOB LAYOUTS "layout/*.qml")



find_package(Qt6 6.4 REQUIRED COMPONENTS Core Quick Concurrent Multimedia QuickControls2)

include_directories("header")
include_directories(${FFMPEG_PATH}/include)
include_directories(${OPENCV_PATH}/include)
include_directories(${OPENCV_PATH}/include/opencv2)


qt_standard_project_setup()

qt_add_executable(appaplayer
    ${SOURCE_FILE}
    ${FFMPEG_FILE}
)

foreach(filepath ${OpenCV_LIBS})
    message(${filepath})
endforeach(filepath)

foreach(filepath ${FFMPEG_LIBS})
    message(${filepath})
endforeach(filepath)

foreach(filepath ${LAYOUTS})
    message(${filepath} "\t" ${CMAKE_CURRENT_SOURCE_DIR})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath}) #.+/(.+)\\\\..*" "\\\\1
    list(APPEND QML_FILES ${filename})
endforeach(filepath)
list(REMOVE_DUPLICATES QML_FILES) #表示删除非第一个重名值

foreach(filepath ${VALUES})
    message(${filepath} "\t" ${CMAKE_CURRENT_SOURCE_DIR})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath}) #.+/(.+)\\\\..*" "\\\\1
    list(APPEND QML_FILES ${filename})
endforeach(filepath)
list(REMOVE_DUPLICATES QML_FILES) #表示删除非第一个重名值

foreach(filepath ${IMAGES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath}) #.+/(.+)\\\\..*" "\\\\1
    list(APPEND IMG_FILES ${filename})
endforeach(filepath)
list(REMOVE_DUPLICATES IMG_FILES)


list(APPEND QML_HEADERS
    "header/media.hpp"
    "header/backend.hpp"
    "header/painteditem.hpp"
)

list(APPEND QML_SOURCES

)

qt_add_qml_module(appaplayer
    URI aplayer
    VERSION 1.0
    QML_FILES
    ${QML_FILES}
    RESOURCES
    ${QML_HEADERS}
    ${QML_SOURCES}
    ${IMG_FILES}

)


set_target_properties(appaplayer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appaplayer
    PRIVATE Qt6::Core Qt6::Quick Qt6::Multimedia Qt6::Concurrent Qt6::QuickControls2
    ${OpenCV_LIBS} ${FFMPEG_LIBS}
)

install(TARGETS appaplayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
