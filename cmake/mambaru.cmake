include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if ( NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  message(STATUS "${PROJECT_NAME} is not top level project")
  return()
endif()

include(mambaopt)
include(mambalibs)

get_faslib()

if (NOT WFLOW_DISABLE_JSON)
  get_mambaru(wjson WJSON_DIR "" "")
  add_definitions(-DWFLOW_DISABLE_JSON)
  add_definitions(-DWLOG_DISABLE_JSON)
  set(WLOG_DISABLE_JSON ON)
endif()

if (NOT WFLOW_DISABLE_LOG)
  get_mambaru(wlog WLOG_DIR WLOG_LIB "")
else()
  add_definitions(-DWFLOW_DISABLE_LOG)
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})


