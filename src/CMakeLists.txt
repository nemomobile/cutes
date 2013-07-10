SET(PLUGIN_NAME qtscript)
set(SRC main.cpp)
set(LIBSRC QmlAdapter.cpp QsActor.cpp QsEnv.cpp)
set(QMLPKG Mer/QtScript)

set(HDRS EngineAccess.hpp QsActor.hpp QsActorEvents.hpp QsEnv.hpp)

add_definitions(-DQT_SHARED)

set(PLUGIN_SRC )
set(NAME cutes)
set(LIBNAME cutescript-qt5)

qt5_wrap_cpp(MOC_SRC ${HDRS} plugin-qt5.hpp)

add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SRC} ${MOC_SRC})
add_library(${LIBNAME} SHARED ${LIBSRC} ${MOC_SRC})
add_executable(${NAME} ${SRC})

target_link_libraries(${PLUGIN_NAME}
  ${LIBNAME}
)

target_link_libraries(${NAME}
  ${LIBNAME}
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${Qt5Quick_LIBRARIES}
  )

install(TARGETS ${PLUGIN_NAME} DESTINATION lib${LIB_SUFFIX}/qt5/qml/${QMLPKG})
install(FILES qmldir DESTINATION lib${LIB_SUFFIX}/qt5/qml/${QMLPKG})

install(TARGETS ${NAME} DESTINATION bin)
install(TARGETS ${LIBNAME} DESTINATION lib${LIB_SUFFIX})
