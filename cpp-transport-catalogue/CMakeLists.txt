cmake_minimum_required(VERSION 3.5)


set(PROJECT_NAME cpp-transport-catalogue)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TRANSPORT_CATALOGUE_SOURCES
    main.cpp
    input_reader.cpp
    stat_reader.cpp
    transport_catalogue.cpp
    test_example_functions.cpp
)
set(TRANSPORT_CATALOGUE_HEADERS
    test_framework.h
    geo.h
    input_reader.h
    stat_reader.h
    transport_catalogue.h
    test_example_functions.h
)

add_executable(${PROJECT_NAME} ${TRANSPORT_CATALOGUE_SOURCES} ${TRANSPORT_CATALOGUE_HEADERS})

# Функция find_package ищет TBBConfig, используя переменные
# CMAKE_PREFIX_PATH и TBB_DIR.
#message( "Including TBB on MyLib build" )

#find_package(TBB REQUIRED tbb)
# "TBB::tbb" можно использовать вместо "${TBB_IMPORTED_TARGETS}"
#target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)
#message(PACKAGE_FIND_NAME)
