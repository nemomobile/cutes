SET(PLUGIN_NAME cutesqml)
set(NAME cutes)
set(LIBNAME cutes-qt5)
set(QMLPKG Mer/Cutes)

set(CMAKE_AUTOMOC TRUE)

add_definitions(-DQT_SHARED)

qt5_wrap_cpp(UTIL_MOC_SRC ${CUTES_INCLUDES_DIR}/cutes/util.hpp)

set(SRC main.cpp QmlAdapter.cpp)
set(LIBSRC Env.cpp Actor.cpp ActorEvents.cpp  ${UTIL_MOC_SRC} js_util.cpp)

qt5_generate_moc(MOC_SRC ${HDRS} plugin-qt5.hpp)

add_library(${PLUGIN_NAME} MODULE ${MOC_SRC})
add_library(${LIBNAME} SHARED ${LIBSRC})
add_executable(${NAME} ${SRC})

set_target_properties(${LIBNAME} PROPERTIES
  SOVERSION 0
  VERSION ${CUTES_VERSION}
  )

target_link_libraries(${PLUGIN_NAME}
  ${LIBNAME}
)

qt5_use_modules(${LIBNAME} Core Qml Quick)
qt5_use_modules(${NAME} Core Gui Widgets)

target_link_libraries(${NAME}
  ${LIBNAME}
  )

install(TARGETS ${PLUGIN_NAME} DESTINATION lib${LIB_SUFFIX}/qt5/qml/${QMLPKG})
install(FILES qmldir DESTINATION lib${LIB_SUFFIX}/qt5/qml/${QMLPKG})
install(TARGETS ${LIBNAME} DESTINATION lib${LIB_SUFFIX})
install(TARGETS ${NAME} DESTINATION bin)
