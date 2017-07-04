cmake_minimum_required(VERSION 2.6)

project(iow)

if ( "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  INCLUDE(FindThreads)
  SET(CMAKE_CXX_FLAGS       "${CXX_FLAGS} -std=c++11 -ftemplate-backtrace-limit=0")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ggdb -DNDEBUG -march=core2 ") 
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb ")

  set(FASLIB_DIR "${CMAKE_SOURCE_DIR}/../faslib")
  set(FAS_TESTING_CPP "${FASLIB_DIR}/fas/testing/testing.cpp")
  find_package(Boost COMPONENTS system program_options filesystem date_time REQUIRED)
  include_directories(../faslib)
  include_directories(../wjson)
endif()

include_directories(.)

add_library(
  iow
  iow/io/aux/read_buffer.cpp
  iow/io/aux/write_buffer.cpp
  iow/logger/global_log.cpp
  iow/workflow/workflow.cpp
  iow/workflow/delayed_queue.cpp
  iow/workflow/asio_queue.cpp
  iow/workflow/bique.cpp
  iow/workflow/timer.cpp
  iow/workflow/task_manager.cpp
  iow/workflow/timer_manager_base.cpp
  iow/workflow/timer_manager.cpp
  iow/workflow/thread_pool_base.cpp
)

target_link_libraries(iow ${Boost_LIBRARIES} )

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

