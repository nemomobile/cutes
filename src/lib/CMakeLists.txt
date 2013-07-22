add_library(cutes-util SHARED util.cpp)

add_library(cutes-core SHARED core.cpp)

target_link_libraries(cutes-core
  cutes-util
  ${Qt5Core_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${V8_LIBRARIES}
)

SET(SRC
  main.cpp
)
ADD_EXECUTABLE(qttest ${SRC})

target_link_libraries(qttest
  cutes-core
  ${CMAKE_THREAD_LIBS_INIT}
)

