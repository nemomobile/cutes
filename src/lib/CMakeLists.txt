set(CMAKE_AUTOMOC TRUE)

MACRO(GENERATE_BRIDGE _name)
  set(_tgt ${_name}.cpp)
  set(_js qt${_name}.js)
  set(_dst ${CMAKE_CURRENT_SOURCE_DIR}/${_tgt})
  set(_js_file ${CMAKE_CURRENT_SOURCE_DIR}/${_js})
  set(_src ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.cpp.in)
  set(_gen_script ${CMAKE_CURRENT_SOURCE_DIR}/preprocess_bridge.py)

  add_custom_command(OUTPUT ${_tgt} ${_js}
    COMMAND python ${_gen_script} ${_src} ${_dst} ${_js_file}
    DEPENDS ${_src} ${_gen_script}
    )
  qt5_generate_moc(${_tgt} ${_name}.moc)
ENDMACRO(GENERATE_BRIDGE)

generate_bridge(core)
add_library(cutes-core SHARED core.cpp util.hpp util.cpp core.moc)

target_link_libraries(cutes-core
  ${Qt5Core_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  -ldl
)

add_executable(test_linking test_linking.cpp)
target_link_libraries(test_linking cutes-core)

install(TARGETS cutes-core DESTINATION ${DST_LIB}/qt5/cutes/qt)
install(FILES qtcore.js DESTINATION share/cutes)
install(FILES wrap-qt.js DESTINATION share/cutes)
