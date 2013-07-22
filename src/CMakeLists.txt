find_package(PkgConfig REQUIRED)
pkg_check_modules(V8 Qt5V8 REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)

set(V8_PRIVATE_INCLUDES
  "/usr/include/qt5/QtV8/${Qt5Qml_VERSION_STRING}/QtV8"
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  ${V8_PRIVATE_INCLUDES}
  ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
)

add_definitions(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Qml_DEFINITIONS}
  ${V8_DEFINITIONS}
)

add_definitions(-DQT_SHARED)

set(CMAKE_AUTOMOC TRUE)

add_subdirectory(lib)
add_subdirectory(cutes)
