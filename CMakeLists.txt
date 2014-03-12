PROJECT(CUTES)
cmake_minimum_required(VERSION 2.8)

set(NAME cutes)

set(prefix ${CMAKE_INSTALL_PREFIX})

option(ENABLE_MULTIARCH "enable support for multi-arch distros (lib64)" OFF)
IF(ENABLE_MULTIARCH)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    SET(LIB_SUFFIX "")
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    SET(LIB_SUFFIX 64)
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
ELSE()
  SET(LIB_SUFFIX "")
ENDIF()
SET(DST_LIB lib${LIB_SUFFIX})

set(CUTES_LIB_PATH "${prefix}/share/cutes")

if(${prefix} STREQUAL "/usr")
  set(CUTES_LIB_PATH "${CUTES_LIB_PATH}:/usr/${DST_LIB}/qt5/cutes")
else(${prefix} STREQUAL "/usr")
  set(CUTES_LIB_PATH "${CUTES_LIB_PATH}:${prefix}/${DST_LIB}/qt5/cutes:/usr/${DST_LIB}/qt5/cutes")
endif(${prefix} STREQUAL "/usr")

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

# Different JS engines - set QT5JS
if(Qt5Qml_VERSION_MINOR GREATER 1)
  set(QT5JS QT5V4)
else(Qt5Qml_VERSION_MINOR GREATER 1)
  set(QT5JS QT5V8)
endif(Qt5Qml_VERSION_MINOR GREATER 1)
message(STATUS "Qt JS engine is ${QT5JS}")

if (${QT5JS} MATCHES "QT5V8")
  find_package(Qt5V8 QUIET)
  if (Qt5V8_PRIVATE_INCLUDE_DIRS)
    set(NORMAL_V8_COMPILATION ON)
    message(STATUS "Normal Qt5V8 compilation")
    message(STATUS "Private dirs are ${Qt5V8_PRIVATE_INCLUDE_DIRS}")
  endif(Qt5V8_PRIVATE_INCLUDE_DIRS)
endif (${QT5JS} MATCHES "QT5V8")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  ${Qt5Quick_INCLUDE_DIRS}
)

add_definitions(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Gui_DEFINITIONS}
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Qml_DEFINITIONS}
  ${Qt5Quick_DEFINITIONS}
)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -DCUTES_LIB_PATH=\"${CUTES_LIB_PATH}\" -fPIC -W -Wall -Wextra -Werror -g -O2 -std=c++0x")

if (${QT5JS} MATCHES "QT5V8")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQJS_ENGINE=8")
else (${QT5JS} MATCHES "QT5V8")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQJS_ENGINE=4")
endif (${QT5JS} MATCHES "QT5V8")

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wno-psabi")

add_subdirectory("src")
add_subdirectory("doc")
