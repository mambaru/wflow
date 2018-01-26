cmake_minimum_required(VERSION 2.6)

project(wflow)

if ( "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  INCLUDE(FindThreads)
  SET(CMAKE_CXX_FLAGS       "${CXX_FLAGS} -std=c++11 -ftemplate-backtrace-limit=0")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ggdb -DNDEBUG -march=core2 ") 
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb ")
  if (PARANOID_WARNING)
    if(UNIX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align \
                         -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization\
                         -Wformat=2 -Winit-self -Wmissing-declarations \
                         -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
                         -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo \
                         -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror")
    endif()
  endif ()
  add_definitions(-DWFLOW_DISABLE_LOG=ON)
  find_package(Boost COMPONENTS system date_time REQUIRED)
endif()

include_directories(.)
include_directories(../wlog)

add_library(
  wflow
  wflow/workflow.cpp
  wflow/delayed_queue.cpp
  wflow/asio_queue.cpp
  wflow/bique.cpp
  wflow/timer.cpp
  wflow/task_manager.cpp
  wflow/timer_manager_base.cpp
  wflow/timer_manager.cpp
  wflow/thread_pool_base.cpp
)

target_link_libraries(wflow ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES})

if ( BUILD_TESTING )
  enable_testing()
  set(FASLIB_DIR "${CMAKE_SOURCE_DIR}/../faslib")
  set(FAS_TESTING_CPP "${FASLIB_DIR}/fas/testing/testing.cpp")
  include_directories(../faslib)
  add_definitions(-DIOW_DISABLE_LOG)
  add_subdirectory(tests)
endif()

