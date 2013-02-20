set(NAME cutes)
set(LIBNAME cutescript)
SET(PLUGIN_NAME qtscript)
set(SRC main.cpp)
set(LIBSRC QmlAdapter.cpp QsActor.cpp QsEnv.cpp)
set(PLUGIN_SRC QmlPlugin.cpp)
set(QMLPKG Mer/QtScript)

set(HDRS EngineAccess.hpp QmlAdapter.hpp QsActor.hpp QsEnv.hpp)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)

add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SRC} ${MOC_SRC})
add_library(${LIBNAME} SHARED ${LIBSRC} ${MOC_SRC})
add_executable(${NAME} ${SRC})

target_link_libraries(${NAME}
  ${LIBNAME}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDECLARATIVE_LIBRARY}
  ${QT_QTSCRIPT_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  )

target_link_libraries(${PLUGIN_NAME}
  ${LIBNAME}
)

install(TARGETS ${NAME} DESTINATION bin)
install(TARGETS ${LIBNAME} DESTINATION lib${LIB_SUFFIX})
install(TARGETS ${PLUGIN_NAME} DESTINATION lib${LIB_SUFFIX}/qt4/imports/${QMLPKG})
install(FILES qmldir DESTINATION lib${LIB_SUFFIX}/qt4/imports/${QMLPKG})
