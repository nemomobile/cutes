add_library(cutes-qt5v8 SHARED util.cpp)
add_library(cutes-core SHARED core.cpp)

target_link_libraries(cutes-core
  cutes-qt5v8
  ${Qt5Core_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${V8_LIBRARIES}
  -ldl
)

if(NORMAL_V8_COMPILATION)
  target_link_libraries(cutes-core
    ${Qt5V8_LIBRARIES}
    )
else(NORMAL_V8_COMPILATION)
  pkg_check_modules(V8 Qt5V8 REQUIRED)
  target_link_libraries(cutes-core
    ${V8_LIBRARIES}
    )
endif(NORMAL_V8_COMPILATION)

install(TARGETS cutes-qt5v8 DESTINATION ${DST_LIB})
install(TARGETS cutes-core DESTINATION ${DST_LIB}/qt5/cutes/qt)
