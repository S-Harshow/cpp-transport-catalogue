 
project (catalog_lib)

aux_source_directory(. SRC_LIST)

# Add a library with the above sources
add_library(${PROJECT_NAME} ${SRC_LIST})
add_library(sub::catalog ALIAS ${PROJECT_NAME})
