set(NAME cutes)
set(SRC main.cpp QmlAdapter.cpp QsActor.cpp QsEnv.cpp)

set(HDRS EngineAccess.hpp QmlAdapter.hpp QsActor.hpp QsEnv.hpp)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)

add_executable(${NAME} ${SRC} ${MOC_SRC})
target_link_libraries(${NAME}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDECLARATIVE_LIBRARY}
  ${QT_QTSCRIPT_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  )

install(TARGETS ${NAME} DESTINATION bin)
