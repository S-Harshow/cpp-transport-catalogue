 
project(test_json)

aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME}  ${SRC_LIST} ${GTestFiles})
add_test(NAME libjson COMMAND ${PROJECT_NAME} )
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
        sub::json)

