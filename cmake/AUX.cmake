if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_DEFAULT 11)
set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std=c++17")
set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "-std=c++14")




if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()


get_property(cur_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

if (NOT WFLOW_CLONE_DEPENDENCIES)
include(ConfigureLibrary)
CONFIGURE_LIBRARY( fas/aop.hpp "${cur_dirs} \
                                ${CMAKE_CURRENT_SOURCE_DIR}/../faslib \
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../../faslib \
                                ${CMAKE_SOURCE_DIR}/../faslib \
                                ${PROJECT_BINARY_DIR}/faslib \
                                /usr/include/faslib \
                                /usr/local/include/faslib" 
                  faslib "" )
CONFIGURE_LIBRARY( wjson/json.hpp "${cur_dirs} \
                                ${CMAKE_CURRENT_SOURCE_DIR}/../wjson \
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../../wjson \
                                ${CMAKE_SOURCE_DIR}/../wjson \
                                ${PROJECT_BINARY_DIR}/wjson \
                                /usr/include/wjson \
                                /usr/local/include/wjson" 
                  wjson "" )

CONFIGURE_LIBRARY( wlog/wlog.hpp "${cur_dirs} \
                                ${CMAKE_CURRENT_SOURCE_DIR}/../wlog \
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../../wlog \
                                ${CMAKE_SOURCE_DIR}/../wlog \
                                ${PROJECT_BINARY_DIR}/wlog \
                                /usr/include/wlog \
                                /usr/local/include/wlog" 
                  wlog 
                                "/usr/lib /usr/local/lib /usr/lib64 \
                                 ${PROJECT_BINARY_DIR}/wlog/build \
                                 ${CMAKE_SOURCE_DIR}/../wlog/build \
                                 ${CMAKE_SOURCE_DIR}/wlog/build \
                                 ${CMAKE_CURRENT_SOURCE_DIR}/../wlog/build \
                                 ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/wlog \
                                 ${CMAKE_CURRENT_SOURCE_DIR}/wlog/build" 
                  )
endif(NOT WFLOW_CLONE_DEPENDENCIES)

#if ( NOT HAVE_INCLUDE_faslib )
#  message(WARNING "faslib not found! Use 'git clone https://github.com/migashko/faslib.git' \
#                   in parent directory or install to system and set path in you project")
#endif()

#if ( NOT HAVE_INCLUDE_wjson )
#  message(WARNING "wjson not found! Use 'git clone https://github.com/mambaru/wjson.git' \
#                   in parent directory or install to system and set path in you project")
#endif()


if ( ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") 
      OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") )
      
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Werror -pedantic -ftemplate-backtrace-limit=0")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
  
  if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STRICT_ANSI__ -stdlib=libc++")
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wno-pragma-once-outside-header")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-overflow=2 -Wswitch -Wswitch-default -Wundef -Werror")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      # -Wunsafe-loop-optimizations
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op  -Wnoexcept -Wstrict-null-sentinel")
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

set(store_BUILD_TESTING ${BUILD_TESTING})

if ( NOT FASLIB_DIR )
  if ( HAVE_INCLUDE_faslib )
    set(FASLIB_DIR "${HAVE_INCLUDE_faslib}")
    if (FASLIB_DIR STREQUAL "${PROJECT_SOURCE_DIR}/faslib" )
      add_subdirectory(faslib)
    endif()

  else()
    #execute_process(COMMAND git clone https://github.com/migashko/faslib.git WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
    #execute_process(COMMAND mkdir -p build WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/faslib")
    #execute_process(COMMAND cmake .. WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/faslib/build")
    #set(FASLIB_DIR "${PROJECT_BINARY_DIR}/faslib")
    if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/faslib")
      execute_process(COMMAND git clone https://github.com/migashko/faslib.git WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    endif()
    
    set(BUILD_TESTING OFF)
    add_subdirectory(faslib)
    set(FASLIB_DIR "${PROJECT_SOURCE_DIR}/faslib")
    
  endif()
endif( NOT FASLIB_DIR )

if ( NOT WJSON_DIR )
  if ( HAVE_INCLUDE_wjson )
    set(WJSON_DIR "${HAVE_INCLUDE_wjson}")
    if (WJSON_DIR STREQUAL "${PROJECT_SOURCE_DIR}/wjson" )
      add_subdirectory(wjson)
    endif()
  else()
    #execute_process(COMMAND git clone https://github.com/mambaru/wjson.git WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
    #execute_process(COMMAND mkdir -p build WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/wjson")
    #execute_process(COMMAND cmake .. WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/wjson/build")
    #set(WJSON_DIR "${PROJECT_BINARY_DIR}/wjson")
    
    if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/wjson")
      execute_process(COMMAND git clone https://github.com/mambaru/wjson.git WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    endif()
    set(BUILD_TESTING OFF)
    add_subdirectory(wjson)
    set(WJSON_DIR "${PROJECT_SOURCE_DIR}/wjson")

  endif()
endif( NOT WJSON_DIR )

if ( NOT WLOG_DIR )
  if ( HAVE_INCLUDE_wlog )
    set(WLOG_DIR "${HAVE_INCLUDE_wlog}")
    set(WLOG_LIBDIR "${HAVE_BINARY_wlog}")
    
    if ( WLOG_DIR STREQUAL "${PROJECT_SOURCE_DIR}/wlog" )
      add_subdirectory(wlog)
    endif()
  else()
#    execute_process(COMMAND git clone https://github.com/mambaru/wlog.git WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
#    execute_process(COMMAND make static WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/wlog")
#    execute_process(COMMAND make shared WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/wlog")
#    set(WLOG_DIR "${PROJECT_BINARY_DIR}/wlog")
#    set(WLOG_LIBDIR "${PROJECT_BINARY_DIR}/wlog/build")
    if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/wlog")
      execute_process(COMMAND git clone https://github.com/mambaru/wlog.git WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    endif()
    set(BUILD_TESTING OFF)
    add_subdirectory(wlog)
    set(WLOG_DIR "${PROJECT_SOURCE_DIR}/wlog")
    set(WLOG_LIBDIR "${PROJECT_BINARY_DIR}")
  endif()
endif( NOT WLOG_DIR )

set(BUILD_TESTING ${store_BUILD_TESTING})

set(FAS_TESTING_CPP "${FASLIB_DIR}/fas/testing/testing.cpp")

include_directories(${FASLIB_DIR})
include_directories(${WJSON_DIR})
include_directories(${WLOG_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${WLOG_LIBDIR})
