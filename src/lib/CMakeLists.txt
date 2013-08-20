add_library(cutes-qt5v8 SHARED util.cpp)

add_library(cutes-core SHARED core.cpp)

target_link_libraries(cutes-core
  cutes-qt5v8
  ${Qt5Core_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${V8_LIBRARIES}
)

install(TARGETS cutes-qt5v8 DESTINATION ${DST_LIB})
install(TARGETS cutes-core DESTINATION ${DST_LIB}/qt5/cutes/qt)
