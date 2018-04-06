cmake_minimum_required(VERSION 3.1)
project(wflow)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AUX)
include(FindThreads)
find_package(Boost REQUIRED system )
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
  list(APPEND source_list wflow/json/load.cpp )
endif()

add_library(wflow ${source_list})

if (NOT WFLOW_DISABLE_LOG)
  target_link_libraries(wflow wlog ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )
elseif() 
  target_link_libraries(wflow ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} )
endif()

include_directories(.)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(examples)
endif()

set(CPACK_PACKAGE_NAME "wflow")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "7")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
set(CPACK_PACKAGE_CONTACT "migashko@gmail.com")

install(DIRECTORY wflow DESTINATION include FILES_MATCHING PATTERN "*.hpp" )
install(TARGETS wflow LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include(CPack)
