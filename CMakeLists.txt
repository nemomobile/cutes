PROJECT(CUTES)
cmake_minimum_required(VERSION 2.8)

set(NAME cutes)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(LIB_SUFFIX "")
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(LIB_SUFFIX 64)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

if(${USEQT} STREQUAL "4")

find_package(Qt4 4.7.0 REQUIRED)

include(${QT_USE_FILE})

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTDECLARATIVE_INCLUDE_DIR}
  ${QT_QTSCRIPT_INCLUDE_DIR}
)

link_directories(
  ${QT_LIBRARY_DIR}
)

elseif(${USEQT} STREQUAL "5")

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5Script REQUIRED)

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Script_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  ${Qt5Quick_INCLUDE_DIRS}
)

add_definitions(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Gui_DEFINITIONS}
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Script_DEFINITIONS}
  ${Qt5Qml_DEFINITIONS}
  ${Qt5Quick_DEFINITIONS}
)

endif(${USEQT} STREQUAL "4")

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wextra -Werror -Wno-psabi -g -O2 -std=c++0x")

add_subdirectory("src")
add_subdirectory("doc")
