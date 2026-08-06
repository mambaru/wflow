cmake_minimum_required(VERSION 3.10)
project(wflow VERSION "0.9.8")

include(cmake/ci.cmake)

include_directories(.)
set(
  source_list
  wflow/workflow.cpp
  wflow/queue/delayed_queue.cpp
  wflow/queue/asio_queue.cpp
  wflow/queue/bique.cpp
  wflow/timer/private/timer_handler.cpp
  wflow/timer/timer_manager_base.cpp
  wflow/timer/timer_manager.cpp
  wflow/timer/private/time_parser.cpp
  wflow/task/task_manager.cpp
  wflow/task/thread_pool_base.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  set_source_files_properties(wflow/queue/asio_queue.cpp PROPERTIES COMPILE_FLAGS "-Wno-zero-as-null-pointer-constant")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.1)
    set_source_files_properties(wflow/queue/asio_queue.cpp PROPERTIES COMPILE_FLAGS "-Wno-null-dereference")
    set_source_files_properties(wflow/queue/bique.cpp PROPERTIES COMPILE_FLAGS "-Wno-null-dereference")
  endif()
endif()

if (NOT WFLOW_DISABLE_JSON)
  wci_submodule(NAME faslib)
  wci_submodule(NAME wjson)
  list(APPEND source_list wflow/json/load.cpp )
else()
  add_definitions(-DWFLOW_DISABLE_JSON)
  set(WLOG_DISABLE_JSON ON)
endif()

if (NOT WFLOW_DISABLE_LOG)
  wci_submodule(NAME wlog)
else()
  add_definitions(-DWFLOW_DISABLE_LOG)
endif()

get_boost(system asio)

add_library(wflow ${source_list})
wci_targets(wflow)
target_include_directories(wflow PUBLIC $<BUILD_INTERFACE:${wflow_SOURCE_DIR}> )
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
include(FindThreads REQUIRED)

target_compile_definitions(wflow PRIVATE -DBOOST_ASIO_NO_DEPRECATED)

if (NOT WFLOW_DISABLE_LOG)
  target_link_libraries(wflow PRIVATE Threads::Threads PUBLIC wlog Boost::system Boost::asio )
elseif (NOT WFLOW_DISABLE_JSON)
  target_link_libraries(wflow PRIVATE Threads::Threads PUBLIC wjson Boost::system Boost::asio )
else()
  target_link_libraries(wflow PRIVATE Threads::Threads PUBLIC faslib Boost::system Boost::asio )
endif()

if ( BUILD_TESTING )
  if ( WFLOW_DISABLE_JSON )
    wci_submodule(faslib master)
  endif()

  enable_testing()
  add_subdirectory(tests)
  if ( WITH_SAMPLES )
    add_subdirectory(examples)
  endif()
endif()

include(cmake/install.cmake)
