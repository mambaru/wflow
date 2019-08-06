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

if (NOT WFLOW_DISABLE_JSON)
  rootlibs(faslib wjson)
  list(APPEND source_list wflow/json/load.cpp )
else()
  add_definitions(-DWFLOW_DISABLE_JSON)
  set(WLOG_DISABLE_JSON ON)
endif()

if (NOT WFLOW_DISABLE_LOG)
  rootlibs(wlog)
else()
  add_definitions(-DWFLOW_DISABLE_LOG)
endif()

add_library(wflow ${source_list})

include(FindThreads)
find_package(Boost REQUIRED system )
if (NOT WFLOW_DISABLE_LOG)
  target_link_libraries(wflow wlog ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )
else()
  target_link_libraries(wflow ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )
endif()

if ( BUILD_TESTING )
  if ( WFLOW_DISABLE_JSON )
    rootlibs(faslib)
  endif()
 
  enable_testing()
  add_subdirectory(tests)
  if ( WITH_SAMPLES )
    add_subdirectory(examples)
  endif()
endif()

set(CPACK_PACKAGE_NAME "wflow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "8")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY wflow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS wflow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
