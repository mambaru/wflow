set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR})

set( SUITE_LIST
  delayed_queue_suite.cpp
  asio_queue_suite.cpp
  thread_pool_suite.cpp
  timer_manager_suite.cpp
  workflow_suite.cpp
)

add_executable( workflow_test ${FAS_TESTING_CPP} workflow_test.cpp ${SUITE_LIST} )

target_link_libraries(workflow_test wflow )
add_test(NAME workflow_test COMMAND workflow_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
add_test_memcheck(workflow_test 100) # 100 is program arg for workflow_test - precision in milliseconds to run under valgrind
