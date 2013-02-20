PROJECT(CUTES)
cmake_minimum_required(VERSION 2.8)

set(NAME cutes)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(LIB_SUFFIX "")
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(LIB_SUFFIX 64)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

find_package(Qt4 4.7.0 REQUIRED)

include(${QT_USE_FILE})

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wextra -Werror -Wno-psabi -g -O2 -std=c++0x")

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTDECLARATIVE_INCLUDE_DIR}
  ${QT_QTSCRIPT_INCLUDE_DIR}
)

link_directories(
  ${QT_LIBRARY_DIR}
)

add_subdirectory("src")
add_subdirectory("doc")
