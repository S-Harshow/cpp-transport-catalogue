//#include "stat_reader.h"
//#include <algorithm>
//#include <iostream>
//#include <ostream>

// using namespace std;

// namespace transport::io::detail {
// optional<Response> getStopInfo(TransportCatalogue *const catalog,
//                                const Query &request) {
//   if (request.name.empty()) {
//     return nullopt;
//   }
//   string stop_stat{};
//   if (auto stopInfo = catalog->getStopInfo(request.name); stopInfo) {
//     if (0 == stopInfo->buses.size()) {
//       stop_stat = "no buses"s;
//     } else {
//       stop_stat = "buses"s;
//       for (string_view bus : stopInfo->buses) {
//         stop_stat += " " + string(bus);
//       }
//     }
//   } else {
//     stop_stat = "not found"s;
//   }
//   return {{ResponseType::STOP, request.name, stop_stat}};
// }
// optional<Response> getBusInfo(TransportCatalogue *const catalog,
//                               const Query &request) {
//   if (request.name.empty()) {
//     return nullopt;
//   }
//   string bus_stat{};
//   if (auto busInfo = catalog->getBusInfo(request.name); busInfo) {
//     ostringstream output;
//     output << to_string(busInfo->stops) << " stops on route, "s
//            << to_string(busInfo->unique_stops) << " unique stops, "s
//            << busInfo->routelength << " route length, "s
//            << (busInfo->routelength / busInfo->geolength) << " curvature"s;
//     bus_stat = output.str();
//   } else {
//     bus_stat = "not found"s;
//   }
//   return {{ResponseType::BUS, request.name, bus_stat}};
// }
// } // namespace transport::io::detail

// namespace transport::io {

// std::ostream &operator<<(std::ostream &ostream,
//                          const ResponseType responseType) {
//   ostream << static_cast<int>(responseType);
//   return ostream;
// }

// Response::Response(const ResponseType type, const std::string &name,
//                    const std::string &data)
//     : type(type), name(name), data(data) {}

// string Response::toString() const {
//   string result{};
//   switch (type) {
//   case ResponseType::BUS:
//     result = "Bus "s + name + ": "s + data;
//     break;
//   case ResponseType::STOP:
//     result = "Stop "s + name + ": "s + data;
//   default:
//     break;
//   }
//   return result;
// }

// std::ostream &operator<<(ostream &ostream, const Response &response) {
//   ostream << response.toString() << endl;
//   return ostream;
// }
//// ostream &operator<<(ostream &ostream, const vector<Response> &responses) {
////   if (!responses.empty()) {
////     for (const Response &response : responses) {
////       ostream << response.toString() << endl;
////     }
////   }
////   return ostream;
//// }
// std::optional<std::vector<Response>>
// executeRequests(TransportCatalogue *const catalog,
//                 const std::vector<Query> &requests) {
//   vector<Response> result{};
//   for (const Query &request : requests) {
//     if (!request.hasData()) {
//       optional<Response> response;
//       switch (request.type) {
//       case QueryType::STOP:
//         response = detail::getStopInfo(catalog, request);
//         break;
//       case QueryType::BUS:
//         response = detail::getBusInfo(catalog, request);
//         break;
//       default:
//         break;
//       }
//       if (response) {
//         result.push_back(*response);
//       }
//     }
//   }
//   return result;
// }
// } // namespace transport::io
