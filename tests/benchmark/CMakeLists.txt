 
project(benchmark)

aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME}  ${SRC_LIST} )
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
        benchmark::benchmark
        sub::catalog
        sub::appc
        sub::json)
    target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${CMAKE_BINARY_DIR}
    )
