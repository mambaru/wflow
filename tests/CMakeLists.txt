
set( SUITE_LIST
  delayed_queue_suite.cpp
  asio_queue_suite.cpp
  thread_pool_suite.cpp
  timer_manager_suite.cpp
  workflow_suite.cpp
)

add_executable( workflow_test workflow_test.cpp ${SUITE_LIST} )

target_link_libraries(workflow_test PRIVATE wflow fas::testing)
wci_test(TARGET workflow_test COMMAND workflow_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

