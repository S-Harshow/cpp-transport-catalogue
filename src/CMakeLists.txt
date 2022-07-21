 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(LIB_HEADER_FILES
    request_handler.h           # обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом
    json_reader.h               # выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов
    map_renderer.h
    ranges.h
    graph.h
    router.h
    transport_router.h
)

set(LIB_SOURCE_FILES
    request_handler.cpp         # обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом
    json_reader.cpp             # выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов
    map_renderer.cpp
    transport_router.cpp
)

find_package(TBB REQUIRED tbb)

## sub dir
add_subdirectory(json)
add_subdirectory(svg)
add_subdirectory(transport_catalogue)

aux_source_directory(. SRC_LIST)

add_library(lib_appc ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
add_library(sub::appc ALIAS lib_appc)
target_link_libraries(lib_appc
    PRIVATE
    TBB::tbb
    sub::json
    sub::svg
    sub::catalog
)

add_executable(appc main.cpp)

target_include_directories(appc
    PUBLIC
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(appc
  PRIVATE
  TBB::tbb
    sub::appc
)

message(PACKAGE_FIND_NAME)


