cmake_minimum_required(VERSION 3.15)

PROJECT (simple-cli)
SET (CMAKE_VERBOSE_MAKEFILE OFF)

SET(CMAKE_CXX_FLAGS "-std=c++17 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")
#SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -std=c++17 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")
# SET(CMAKE_CXX_FLAGS "-fsanitize=memory -fsanitize-memory-track-origins -fPIE -fno-omit-frame-pointer -std=c++17 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections" CACHE INTERNAL "exe link flags")
ADD_DEFINITIONS ("-DUNIT_TEST")

SET (CATCH2_INCLUDE "deps/Catch2/single_include/catch2")
INCLUDE_DIRECTORIES("${CATCH2_INCLUDE}")
include_directories ("deps/GSL/include" "deps/etl/include") 

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("test")
    
ADD_EXECUTABLE (unittest
    "${CATCH2_INCLUDE}/catch.hpp"

    "test/00CatchInit.cc"
    "test/01InstantiationTest.cc"
    "test/02TokenizerTest.cc"
    "test/03OutputTest.cc"
)

ADD_EXECUTABLE (demo
    "test/demo.cc"
)

