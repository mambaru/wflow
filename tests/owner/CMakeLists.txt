add_executable( owner_test owner_test.cpp  )
target_link_libraries(owner_test PRIVATE wflow fas::testing)
wci_test( TARGET owner_test COMMAND owner_test )
