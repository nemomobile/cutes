PROJECT(CUTES)
cmake_minimum_required(VERSION 2.8)

set(NAME cutes)

IF(NOT DEFINED CUTES_VERSION)
MESSAGE(FATAL_ERROR "Define CUTES_VERSION")
ENDIF(NOT DEFINED CUTES_VERSION)

set(CUTES_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
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

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wno-psabi")

add_subdirectory("src")
add_subdirectory("doc")
