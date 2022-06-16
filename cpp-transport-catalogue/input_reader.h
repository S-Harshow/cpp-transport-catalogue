//#pragma once

//#include "transport_catalogue.h"
//#include <iostream>
//#include <istream>
//#include <ostream>
//#include <string>
//#include <vector>

// namespace transport::io {
//// типы запросов
// enum class QueryType { STOP, BUS };
// std::ostream &operator<<(std::ostream &, QueryType);

//// структура запроса
// struct Query {
//   [[nodiscard]] bool empty() const;
//   [[nodiscard]] bool hasData() const;
//   QueryType type{};
//   std::string name{};
//   std::string data{};
// };

//// наполнение транспортного каталога информацией
// void fillCatalogue(transport::TransportCatalogue *, const std::vector<Query>
// &); } // namespace transport::io

// namespace transport::io::detail {

//// возвращает true если в строке беззнаковое целое
// bool isUint(std::string_view);

//// убирает пустые знаки
// std::string_view Trim(std::string_view);

//// разделяет строку на две части, разделенные separator
// std::pair<std::string_view, std::string_view>
//     SplitString(std::string_view, std::string_view = ":"s);

//// разделяет строку на части, разделенные separator, и возвращает их в векторе
// std::vector<std::string_view> SplitStringToVector(std::string_view, char =
// ',');

//// выделяет координаты из строки запроса
//::transport::detail::Coordinates GetCoordinates(std::string_view);

//// выделяет список остановок разделенных ',' или '>' (в виде кругового
///движения)
// std::vector<std::string_view> GetStops(std::string_view);

//// выделяет список соседних остановок и расстояние до них из строки запроса
// std::unordered_map<std::string_view, unsigned long>
//     GetNearbyStops(std::string_view);

//// формализует запрос из строки
// Query GetQuery(std::string_view);

//// выполнение запроса на добавление остановки
// void addStop(transport::TransportCatalogue *, const Query &);
//// выполнение запроса на добавление автобуса
// void addBus(transport::TransportCatalogue *, const Query &);

//} // namespace transport::io::detail

// namespace transport::io::stream {
// std::vector<io::Query> read(std::istream &);
// } // namespace transport::io::stream
