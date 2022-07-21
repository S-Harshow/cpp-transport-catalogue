
project(test_json_reader)

aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME}  ${SRC_LIST} ${GTestFiles})
add_test(NAME librenderer COMMAND ${PROJECT_NAME} )
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
        sub::appc
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_BINARY_DIR}
)
