project(test_catalogue)

aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME}  ${SRC_LIST} ${GTestFiles})
add_test(NAME libcatalog COMMAND ${PROJECT_NAME} )
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
        sub::catalog
        sub::appc
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_BINARY_DIR}
)
