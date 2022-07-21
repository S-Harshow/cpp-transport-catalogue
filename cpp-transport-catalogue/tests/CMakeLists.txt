#cmake_minimum_required(VERSION 3.5)

project(test_name LANGUAGES CXX)


add_definitions(-DGTEST_LANGUAGE_CXX11)

find_package(Threads REQUIRED)

if ($ENV{GOOGLETEST_DIR})
    set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})
else ()
    if (NOT "" STREQUAL "")
        message(WARNING "Using googletest src dir specified at Qt Creator wizard")
    endif ()
    set(GOOGLETEST_DIR "")
endif ()
if (EXISTS ${GOOGLETEST_DIR})
    set(GTestSrc ${GOOGLETEST_DIR}/googletest)
    set(GMockSrc ${GOOGLETEST_DIR}/googlemock)
elseif (UNIX AND EXISTS /usr/include/gtest)
    set(GTestSrc /usr/include/gtest)
    message(WARNING "Using gtest from system")
    if (EXISTS /usr/include/gmock)
        set(GMockSrc /usr/include/gmock)
    endif ()
else ()
    message( FATAL_ERROR "No googletest src dir found - set GOOGLETEST_DIR to enable!")
endif ()

set(GTestFiles ${GTestSrc}/src/gtest-all.cc)
set(GTestIncludes ${GTestSrc} ${GTestSrc}/include)
if (NOT ${GMockSrc} STREQUAL "")
    list(APPEND GTestFiles ${GMockSrc}/src/gmock-all.cc)
    list(APPEND GTestIncludes ${GMockSrc} ${GMockSrc}/include)
endif ()

include_directories(${GTestIncludes})

# configure the path.h.in file.
# This file can only use the @VARIABLE@ syntax in the file
configure_file(path.h.in ${CMAKE_BINARY_DIR}/path.h @ONLY)

add_subdirectory(json)
add_subdirectory(json_reader)
add_subdirectory(transport_catalogue)
add_subdirectory(renderer)
add_subdirectory(transport_router)

find_package(benchmark REQUIRED)
add_subdirectory(benchmark)
