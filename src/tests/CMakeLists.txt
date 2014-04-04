set(TESTS_DIR /opt/tests/cutes)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
enable_testing()
add_test(test ./run_tests -l)
add_dependencies(check test)

FILE(GLOB JS_FILES *.js)
FILE(GLOB QML_FILES *.qml)

install(
  FILES run_tests ${JS_FILES} ${QML_FILES}
  DESTINATION ${TESTS_DIR}
)

configure_file(tests.xml.in tests.xml @ONLY)
install(
  FILES tests.xml
  DESTINATION ${TESTS_DIR}
)
