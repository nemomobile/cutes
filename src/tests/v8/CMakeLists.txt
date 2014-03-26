pkg_check_modules(TUT tut REQUIRED)
pkg_check_modules(COR cor REQUIRED)

include_directories(${TUT_INCLUDES} ${COR_INCLUDES})

add_executable(test-cutes-lib main.cpp cutes-lib.cpp)
target_link_libraries(test-cutes-lib
  cutes-core
  ${CMAKE_THREAD_LIBS_INIT}
  ${COR_LIBRARIES}
  )
