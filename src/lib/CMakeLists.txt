add_library(cutes-util SHARED util.cpp)

add_library(cutes-core SHARED core.cpp)

target_link_libraries(cutes-core
  cutes-util
  ${Qt5Core_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${V8_LIBRARIES}
)

install(TARGETS cutes-util cutes-core DESTINATION ${DST_LIB})
