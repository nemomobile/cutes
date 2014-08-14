set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

MACRO(GENERATE_BRIDGE _name _qt_lib)
  set(_tgt ${_name}.cpp)
  set(_js qt-${_name}.js)
  set(_dst ${CMAKE_CURRENT_SOURCE_DIR}/${_tgt})
  set(_js_file ${CMAKE_CURRENT_SOURCE_DIR}/${_js})
  set(_src ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.cpp.in)
  set(_gen_script ${CMAKE_CURRENT_SOURCE_DIR}/preprocess_bridge.py)

  add_custom_command(OUTPUT ${_tgt} ${_js}
    COMMAND python ${_gen_script} ${_src} ${_dst} ${_js_file}
    DEPENDS ${_src} ${_gen_script}
    )
  qt5_generate_moc(${_tgt} ${_name}.moc)

  add_library(cutes-${_name} SHARED ${_name}.cpp ${_name}.moc)
  qt5_use_modules(cutes-${_name} ${_qt_lib} Qml)

  target_link_libraries(cutes-${_name}
    cutes-qt5
    -ldl
    )

  install(TARGETS cutes-${_name} DESTINATION ${DST_LIB}/qt5/cutes/qt)
  install(FILES qt-${_name}.js DESTINATION share/cutes)

ENDMACRO(GENERATE_BRIDGE)

generate_bridge(core Core)

add_executable(test_core_linking test_linking.cpp)
target_link_libraries(test_core_linking cutes-core cutes-qt5)

install(FILES wrap-qt.js DESTINATION share/cutes)
install(PROGRAMS preprocess_bridge.py DESTINATION ${DST_LIB}/cutes/bin)
