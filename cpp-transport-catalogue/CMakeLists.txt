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

set(TRANSPORT_CATALOGUE_HEADERS
    geo.h                       # объявляет координаты на земной поверхности и вычисляет расстояние между ними
    domain.h                    # классы основных сущностей, описывают автобусы и остановки
    transport_catalogue.h       # модуль транспортного каталога
    request_handler.h           # обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом
    json.h                      # ваша улучшенная библиотека для парсинга и вывода JSON
    json_reader.h               # выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов
    json_builder.h
    svg.h
    map_renderer.h
    router.h
    ranges.h
    graph.h
    transport_router.h
)

set(TRANSPORT_CATALOGUE_SOURCES
    main.cpp                    # основной модуль
    geo.cpp                     # объявляет координаты на земной поверхности и вычисляет расстояние между ними
    domain.cpp                  # классы основных сущностей, описывают автобусы и остановки
    transport_catalogue.cpp     # модуль транспортного каталога
    request_handler.cpp         # обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом
    json.cpp                    # ваша улучшенная библиотека для парсинга и вывода JSON
    json_reader.cpp             # выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов
    json_builder.cpp
    svg.cpp
    map_renderer.cpp
    transport_router.cpp
)

find_package(TBB REQUIRED tbb)

add_executable(${PROJECT_NAME} ${TRANSPORT_CATALOGUE_SOURCES} ${TRANSPORT_CATALOGUE_HEADERS})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  TBB::tbb
 )
## Test
#if(NOT ${PROJECT_NAME}_NO_TESTS)
#    enable_testing()
#    add_subdirectory(tests)
#endif()

