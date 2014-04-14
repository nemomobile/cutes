find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
pkg_check_modules(COR cor REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${COR_INCLUDE_DIRS}
)

link_directories(
  ${COR_LIBRARY_DIRS}
)

add_subdirectory(cutes)
add_subdirectory(lib)
add_subdirectory(tests)

# TODO tests?
