project(PiTres)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(PiTres_SRCS PiTres.cpp main.cpp)
qt4_automoc(${PiTres_SRCS})
add_executable(PiTres ${PiTres_SRCS})
target_link_libraries(PiTres ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
