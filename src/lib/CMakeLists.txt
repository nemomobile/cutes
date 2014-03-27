set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
add_library(cutes-core SHARED core.cpp core.moc)
qt5_use_modules(cutes-core Core Qml)

target_link_libraries(cutes-core
  cutes-qt5
  -ldl
)

add_executable(test_core_linking test_linking.cpp)
target_link_libraries(test_core_linking cutes-core cutes-qt5)

install(TARGETS cutes-core DESTINATION ${DST_LIB}/qt5/cutes/qt)
install(FILES qtcore.js DESTINATION share/cutes)
install(FILES wrap-qt.js DESTINATION share/cutes)
