cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME sprint10_transport_catalogue)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TRANSPORT_CATALOGUE_HEADERS
    geo.h                       # объявляет координаты на земной поверхности и вычисляет расстояние между ними
    domain.h                    # классы основных сущностей, описывают автобусы и остановки
    transport_catalogue.h       # модуль транспортного каталога
    request_handler.h           # обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом
    json.h                      # ваша улучшенная библиотека для парсинга и вывода JSON
    json_reader.h               # выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов

    input_reader.h # deprecated
    stat_reader.h # deprecated
    svg.h
    map_renderer.h
    test_framework.h
    test_json.h                 # модуль тестирования библиотеки json
    test_transport_catalogue.h
)

set(TRANSPORT_CATALOGUE_SOURCES
    main.cpp                    # основной модуль
    geo.cpp                     # объявляет координаты на земной поверхности и вычисляет расстояние между ними
    domain.cpp                  # классы основных сущностей, описывают автобусы и остановки
    transport_catalogue.cpp     # модуль транспортного каталога
    request_handler.cpp         # обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом
    json.cpp                    # ваша улучшенная библиотека для парсинга и вывода JSON
    json_reader.cpp             # выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов
    input_reader.cpp # deprecated
    stat_reader.cpp # deprecated
    svg.cpp
    map_renderer.cpp
    test_framework.cpp
    test_json.cpp               # модуль тестирования библиотеки json
    test_transport_catalogue.cpp
    test_map_renderer.cpp
)

add_executable(${PROJECT_NAME} ${TRANSPORT_CATALOGUE_SOURCES} ${TRANSPORT_CATALOGUE_HEADERS})
