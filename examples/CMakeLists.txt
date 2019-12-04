add_executable( example01 example01.cpp )
target_link_libraries(example01 PRIVATE wflow )

add_executable( example02 example02.cpp )
target_link_libraries(example02 PRIVATE wflow )

add_executable( example03 example03.cpp )
target_link_libraries(example03 PRIVATE wflow )

add_executable( example04 example04.cpp )
target_link_libraries(example04 PRIVATE wflow )

add_executable( example05 example05.cpp )
target_link_libraries(example05 PRIVATE wflow )

add_executable( example06 example06.cpp )
target_link_libraries(example06 PRIVATE wflow )

add_executable( example07 example07.cpp )
target_link_libraries(example07 PRIVATE wflow )

add_executable( example08 example08.cpp )
target_link_libraries(example08 PRIVATE wflow )

add_executable( example09 example09.cpp )
target_link_libraries(example09 PRIVATE wflow )

add_executable( example10 example10.cpp )
target_link_libraries(example10 PRIVATE wflow )

add_executable( example11 example11.cpp )
target_link_libraries(example11 PRIVATE wflow )

add_executable( example12 example12.cpp )
target_link_libraries(example12 PRIVATE wflow )

add_executable( example13 example13.cpp )
target_link_libraries(example13 PRIVATE wflow )

add_executable( example14 example14.cpp )
target_link_libraries(example14 PRIVATE wflow )

wci_utils(
  example01 example02 example03 example04 example05 
  example06 example07 example08 example09 example10 
  example11 example12 example13 example14)

if (NOT WFLOW_DISABLE_JSON)
  add_executable( example15 example15.cpp )
  target_link_libraries(example15 PRIVATE wflow )
  wci_utils(example15)
endif()
