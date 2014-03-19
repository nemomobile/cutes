find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
)

if (${QT5JS} MATCHES "QT5V8")
  if(NORMAL_V8_COMPILATION)
    find_package(Qt5V8 REQUIRED)
    MESSAGE(STATUS ${Qt5V8_PRIVATE_INCLUDE_DIRS})
    include_directories(
      ${Qt5V8_PRIVATE_INCLUDE_DIRS}
      )
  else(NORMAL_V8_COMPILATION)
    # hack/workaround - V8 private headers was disabled in upstream
    # commit 1cc6b7a92b326842d2dcd57b55303077d1f3efc3 :/
    message(WARNING "Using hard-coded V8 private headers path"
      "   because V8 private headers was disabled in upstream"
      "   by commit 1cc6b7a92b326842d2dcd57b55303077d1f3efc3 :/")
    set(V8_PRIVATE_INCLUDES
      "/usr/include/qt5/QtV8/${Qt5Qml_VERSION_STRING}/QtV8"
      )
    include_directories(
      ${V8_PRIVATE_INCLUDES}
      )
  endif(NORMAL_V8_COMPILATION)
endif (${QT5JS} MATCHES "QT5V8")


add_definitions(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Qml_DEFINITIONS}
)

add_definitions(-DQT_SHARED)

set(CMAKE_AUTOMOC TRUE)

if (${QT5JS} MATCHES "QT5V8")
add_subdirectory(lib)
else (${QT5JS} MATCHES "QT5V8")
add_subdirectory(lib-v4)
endif (${QT5JS} MATCHES "QT5V8")

add_subdirectory(cutes)

if (${QT5JS} MATCHES "QT5V8")
add_subdirectory(tests/v8)
else (${QT5JS} MATCHES "QT5V8")
# TODO tests?
endif (${QT5JS} MATCHES "QT5V8")
