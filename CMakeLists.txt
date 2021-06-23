cmake_minimum_required(VERSION 3.1)
project(wflow)

include(cmake/ci.cmake)

include_directories(.)
set(
  source_list
  wflow/workflow.cpp
  wflow/queue/delayed_queue.cpp
  wflow/queue/asio_queue.cpp
  wflow/queue/bique.cpp
  wflow/timer/timer.cpp
  wflow/timer/timer_manager_base.cpp
  wflow/timer/timer_manager.cpp
  wflow/task/task_manager.cpp
  wflow/task/thread_pool_base.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  set_source_files_properties(wflow/queue/asio_queue.cpp PROPERTIES COMPILE_FLAGS -Wno-zero-as-null-pointer-constant)
endif()

if (NOT WFLOW_DISABLE_JSON)
  wci_getlib(NAME faslib)
  wci_getlib(NAME wjson)
  list(APPEND source_list wflow/json/load.cpp )
else()
  add_definitions(-DWFLOW_DISABLE_JSON)
  set(WLOG_DISABLE_JSON ON)
endif()

if (NOT WFLOW_DISABLE_LOG)
  wci_getlib(NAME wlog)
else()
  add_definitions(-DWFLOW_DISABLE_LOG)
endif()

add_library(wflow ${source_list})
wci_targets(wflow)
target_include_directories(wflow PUBLIC ${wflow_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
include(FindThreads REQUIRED)

get_boost(system)
target_compile_definitions(wflow PRIVATE -DBOOST_ASIO_NO_DEPRECATED)

if (NOT WFLOW_DISABLE_LOG)
  target_link_libraries(wflow PUBLIC wlog Threads::Threads Boost::system )
elseif (NOT WFLOW_DISABLE_JSON)
  target_link_libraries(wflow PUBLIC wjson Threads::Threads Boost::system )
else()
  target_link_libraries(wflow PUBLIC faslib Threads::Threads Boost::system )
endif()

if ( BUILD_TESTING )
  if ( WFLOW_DISABLE_JSON )
    wci_getlib(faslib master)
  endif()

  enable_testing()
  add_subdirectory(tests)
  if ( WITH_SAMPLES )
    add_subdirectory(examples)
  endif()
endif()

set(CPACK_PACKAGE_NAME "wflow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "3")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY wflow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS wflow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
