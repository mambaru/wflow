if ( NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}" )
  message(STATUS "${PROJECT_NAME} is not top level project")
  return()
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

if ( ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") 
      OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") )
      
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Werror -pedantic -ftemplate-backtrace-limit=0")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
  
  if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STRICT_ANSI__ ${CLANG_LIBPP} ")  # -stdlib=libc++
  endif()

  if ( PARANOID_WARNING )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -ftemplate-depth=1400 -Wcast-align -Winvalid-pch -pedantic-errors  -Wformat-nonliteral")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcomment -Wconversion -Wformat-security -Wimport  -Wchar-subscripts")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat-y2k")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlong-long -Wmissing-braces -Wmissing-field-initializers")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-format-attribute -Wmissing-noreturn -Wpacked -Wparentheses -Wpointer-arith")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wreturn-type -Wsequence-point -Wsign-compare -Wstack-protector -Wuninitialized")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch-enum -Wtrigraphs -Wunknown-pragmas")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal  -Wformat -Wformat=2 -Winit-self  -Wmissing-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunreachable-code -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-variable  -Wvariadic-macros -Wvolatile-register-var  -Wwrite-strings")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-overflow=2 -Wswitch -Wswitch-default -Wundef -Werror")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      # -Wunsafe-loop-optimizations
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op  -Wnoexcept -Wstrict-null-sentinel -Wno-pragma-once-outside-header")
    endif()
  endif(PARANOID_WARNING)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /W3 /wd4503 /wd4307 /EHsc /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG  "/Od /D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "/Yd /O2 /DNDEBUG")
endif()


include(ConfigureLibrary)
set(store_BUILD_TESTING ${BUILD_TESTING})
unset(BUILD_TESTING)

#if ( BUILD_TESTING OR NOT WFLOW_DISABLE_JSON OR NOT WFLOW_DISABLE_LOG) 
#  set(get_FASLIB ON)
#endif()

 set(get_FASLIB BUILD_TESTING OR NOT WFLOW_DISABLE_JSON OR NOT WFLOW_DISABLE_LOG)

### faslib

if (get_FASLIB)
  unset(FASLIB_DIR CACHE)
  find_path( 
    FASLIB_DIR NAMES "fas/aop.hpp"
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" 
    PATH_SUFFIXES "build/faslib" "../build/faslib" "faslib" "../faslib"
  )
  if ( "${FASLIB_DIR}" STREQUAL "FASLIB_DIR-NOTFOUND") 
    unset(FASLIB_DIR CACHE)
    clone_library(faslib "FASLIB_DIR" "https://github.com/migashko/faslib.git")
  endif()
  include_directories("${FASLIB_DIR}")
  set(FAS_TESTING_CPP "${FASLIB_DIR}/fas/testing/testing.cpp")
endif (get_FASLIB)

### wjson

if (NOT WFLOW_DISABLE_JSON)
  unset(WJSON_DIR CACHE)
  find_path( 
    WJSON_DIR NAMES "wjson/json.hpp"
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" 
    PATH_SUFFIXES "build/wjson" "../build/wjson" "wjson" "../wjson"
  )
  if ( "${WJSON_DIR}" STREQUAL "WJSON_DIR-NOTFOUND") 
    unset(WJSON_DIR CACHE)
    clone_library(wjson "WJSON_DIR" "https://github.com/mambaru/wjson.git")
  endif()
  include_directories("${WJSON_DIR}")
else()
  add_definitions(-DWFLOW_DISABLE_JSON)
endif(NOT WFLOW_DISABLE_JSON)

### wlog

if (NOT WFLOW_DISABLE_LOG)

  unset(WLOG_DIR CACHE)
  find_path( 
    WLOG_DIR NAMES "wlog/wlog.hpp"
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" 
    PATH_SUFFIXES "build/wlog" "../build/wlog" "wlog" "../wlog"
  )
  if ( "${WLOG_DIR}" STREQUAL "WLOG_DIR-NOTFOUND") 
    unset(WLOG_DIR CACHE)
    clone_library(wlog "WLOG_DIR" "https://github.com/mambaru/wlog.git")
  endif()
  include_directories("${WLOG_DIR}")
else()
  add_definitions(-DWFLOW_DISABLE_LOG)
endif(NOT WFLOW_DISABLE_LOG)


set(BUILD_TESTING ${store_BUILD_TESTING})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


