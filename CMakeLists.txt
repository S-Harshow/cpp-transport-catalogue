##
##      Информация о проекте
##
cmake_minimum_required(VERSION 3.13)

cmake_policy(SET CMP0048 NEW)
project(transport_catalogue VERSION 0.12 LANGUAGES CXX)

string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%SZ")
enable_testing()
##
## C++ flag
##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(BUILD_TAG ${PROJECT_NAME}_${PROJECT_VERSION}_${BUILD_DATE})
add_compile_options(-DBUILD_TAG=${BUILD_TAG})
message(BUILD_TAG=${BUILD_TAG})

##
##      Опции компиляции
##
if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_compile_options(
        -Werror

        -Wall
        -Wextra
        -Wpedantic

        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX)
endif()

##
## debug / release
##
if (CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "DEBUG")
    message("Debug mode")
    set(CMAKE_CXX_FLAGS_DEBUG   "$ENV{CXXFLAGS} -Wall -ggdb3 -O0 -g")
else()
    message("Release mode")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -ggdb3 -Os")
endif()
message("COMPILE_OPTIONS: ${COMPILE_OPTIONS}")

## sub dir
add_subdirectory(src)

## Test
if(NOT ${PROJECT_NAME}_NO_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

