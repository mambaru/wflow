add_executable( example01 example01.cpp )
target_link_libraries(example01 wflow )

add_executable( example02 example02.cpp )
target_link_libraries(example02 wflow )

add_executable( example03 example03.cpp )
target_link_libraries(example03 wflow )

add_executable( example04 example04.cpp )
target_link_libraries(example04 wflow )

add_executable( example05 example05.cpp )
target_link_libraries(example05 wflow )

add_executable( example06 example06.cpp )
target_link_libraries(example06 wflow )

add_executable( example07 example07.cpp )
target_link_libraries(example07 wflow )

add_executable( example08 example08.cpp )
target_link_libraries(example08 wflow )

add_executable( example09 example09.cpp )
target_link_libraries(example09 wflow )

add_executable( example10 example10.cpp )
target_link_libraries(example10 wflow )

add_executable( example11 example11.cpp )
target_link_libraries(example11 wflow )

add_executable( example12 example12.cpp )
target_link_libraries(example12 wflow )

add_executable( example13 example13.cpp )
target_link_libraries(example13 wflow )

add_executable( example14 example14.cpp )
target_link_libraries(example14 wflow )

if (NOT WFLOW_DISABLE_JSON)
  add_executable( example15 example15.cpp )
  target_link_libraries(example15 wflow )
endif()
