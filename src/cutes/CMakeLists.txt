SET(PLUGIN_NAME cutes)
set(NAME cutes)
set(LIBNAME cutes-qt5)
set(QMLPKG Mer/Cutes)

set(CMAKE_AUTOMOC TRUE)

add_definitions(-DQT_SHARED)

set(SRC main.cpp)
set(LIBSRC Env.cpp Actor.cpp ActorEvents.cpp)

#add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SRC} ${MOC_SRC})
add_library(${LIBNAME} SHARED ${LIBSRC})
#add_executable(${NAME} ${SRC})

# target_link_libraries(${PLUGIN_NAME}
#   ${LIBNAME}
# )

target_link_libraries(${LIBNAME}
  cutes-util
  ${Qt5Core_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${Qt5Quick_LIBRARIES}
  )

# target_link_libraries(${NAME}
  # ${Qt5Gui_LIBRARIES}
  # ${Qt5Widgets_LIBRARIES}
#   ${LIBNAME}
#   )

# install(TARGETS ${PLUGIN_NAME} DESTINATION lib${LIB_SUFFIX}/qt5/qml/${QMLPKG})
# install(FILES qmldir DESTINATION lib${LIB_SUFFIX}/qt5/qml/${QMLPKG})

# install(TARGETS ${NAME} DESTINATION bin)
# install(TARGETS ${LIBNAME} DESTINATION lib${LIB_SUFFIX})
