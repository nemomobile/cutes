set(TESTS_DIR /opt/tests/cutes)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
enable_testing()
add_test(test ./run_tests -l)
add_dependencies(check test)

FILE(GLOB JS_FILES *.js)
FILE(GLOB QML_FILES *.qml)

configure_file(tests.xml.in tests.xml @ONLY)
install(PROGRAMS run_tests DESTINATION ${TESTS_DIR})
install(FILES tests.xml ${JS_FILES} ${QML_FILES} DESTINATION ${TESTS_DIR})
