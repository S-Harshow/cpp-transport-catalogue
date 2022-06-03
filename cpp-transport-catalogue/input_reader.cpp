#include "input_reader.h"
#include "stat_reader.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

namespace transport::io::detail {
using namespace std::literals;
inline constexpr auto WHITESPACE = " \f\n\r\t\v"sv;
const double MIN_LATITUDE = -90.0;
const double MAX_LATITUDE = 90.0;
const double MIN_LONGITUDE = -180.0;
const double MAX_LONGITUDE = 180.0;
} // namespace transport::io::detail

namespace transport::io {
bool Query::empty() const { return name.empty() || data.empty(); }

bool Query::hasData() const { return !data.empty(); }

std::ostream &operator<<(std::ostream &ostream, const QueryType queryType) {
  ostream << static_cast<int>(queryType);
  return ostream;
}

void fillCatalogue(transport::TransportCatalogue *const catalog,
                   const std::vector<Query> &requests) {
  for (const Query &query : requests) {
    switch (query.type) {
    case QueryType::STOP:
      detail::addStop(catalog, query);
      break;
    case QueryType::BUS:
      detail::addBus(catalog, query);
      break;
    default:
      break;
    }
  }
}

} // namespace transport::io

namespace transport::io::detail {
// возвращает true если в строке беззнаковое целое
bool isUint(std::string_view str) {
  return str.find_first_not_of("0123456789") == std::string::npos;
}

string_view Trim(std::string_view text) {
  string_view result(text);
  if (!result.empty()) {
    result.remove_prefix(
        std::min(result.find_first_not_of(WHITESPACE), result.size()));
  }
  if (!result.empty()) {
    auto trim_pos = result.find_last_not_of(WHITESPACE);
    if (trim_pos != string_view::npos) {
      result.remove_suffix(result.size() - trim_pos - 1);
    }
  }
  return result;
}

pair<string_view, string_view> SplitString(std::string_view line,
                                           std::string_view separator) {
  auto pos = line.find(separator);
  if (pos == string_view::npos) {
    return {Trim(line), string_view()};
  }
  auto left = Trim(line.substr(0, pos));
  return (pos < line.size() - separator.size())
             ? make_pair(left, Trim(line.substr(pos + separator.size())))
             : make_pair(left, string_view());
}

vector<string_view> SplitStringToVector(string_view line, char separator) {
  vector<string_view> result{};
  string::size_type pos = 0;
  while (pos < string_view::npos) {
    auto next_pos = line.find(separator, pos);
    if (next_pos != string_view::npos) {
      result.push_back(Trim(line.substr(pos, next_pos - pos)));
    } else {
      result.push_back(Trim(line.substr(pos)));
      break;
    }
    pos = next_pos + 1;
  }
  return result;
}

::transport::detail::Coordinates GetCoordinates(std::string_view coordinates) {
  ::transport::detail::Coordinates result{};
  //  pair<string_view, string_view> coordinates_pair =
  pair coordinates_pair = SplitString(coordinates, ",");
  if (!coordinates_pair.first.empty() && !coordinates_pair.second.empty()) {
    double lat = stod(string(coordinates_pair.first));
    double lng = stod(string(coordinates_pair.second));
    if ((lat >= MIN_LATITUDE && lat <= MAX_LATITUDE) &&
        (lng >= MIN_LONGITUDE && lng < MAX_LONGITUDE)) {
      result.lat = lat;
      result.lng = lng;
    }
  }
  return result;
}

unordered_map<string_view, unsigned long>
GetNearbyStops(std::string_view line) {
  unordered_map<string_view, unsigned long> result{};
  // D1m to stop1, D2m to stop2
  vector<string_view> line_v = SplitStringToVector(line);
  if (3 > line_v.size()) {
    return result;
  }
  for_each(next(line_v.begin(), 2), line_v.end(),
           [&result](string_view nearbyStop) {
             // D1m to stop1
             auto [dist, stop_name] = SplitString(nearbyStop, "m to ");
             if (isUint(dist)) {
               result.emplace(stop_name, std::stoul(string(dist)));
             }
           });

  return result;
}

vector<string_view> GetStops(std::string_view stops) {
  vector<string_view> result{};
  bool is_lap = stops.find('>') != string::npos;
  result = SplitStringToVector(stops, (is_lap ? '>' : '-'));
  if (!is_lap) {
    long size = static_cast<long>(result.size());
    result.resize(2 * size - 1);
    copy(result.begin(), result.begin() + size - 1, result.begin() + size);
    reverse(result.begin() + size, result.end());
  }
  return result;
}

Query GetQuery(std::string_view line) {
  Query result;
  // FIXME формализация запроса из стоки (потенциальная лапша)
  pair query_pair = SplitString(line, " ");
  auto [name, data] = SplitString(query_pair.second, ":");
  if ("Stop"s == query_pair.first) {
    result.type = QueryType::STOP;
    result.name = name;
    result.data = data;
  } else if ("Bus"s == query_pair.first) {
    result.type = QueryType::BUS;
    result.name = name;
    result.data = data;
  }
  return result;
}

void addStop(transport::TransportCatalogue *const catalog, const Query &query) {
  if (!query.empty()) {
    ::transport::detail::Coordinates coordinates =
        detail::GetCoordinates(query.data);
    unordered_map<string_view, unsigned long> nearbyStops =
        detail::GetNearbyStops(query.data);
    catalog->addStop(query.name, coordinates, nearbyStops);
  }
}

void addBus(transport::TransportCatalogue *const catalog, const Query &query) {
  if (!query.empty()) {
    vector<string_view> stops = detail::GetStops(query.data);
    catalog->addBus(query.name, stops);
  }
}
} // namespace transport::io::detail

namespace transport::io::stream {
std::vector<Query> read(istream &input_stream) {
  std::vector<Query> result{};
  // читаю количество запросов на наполнение каталога
  int query_count = 0;
  input_stream >> query_count;
  string line;
  getline(input_stream, line);
  // читаю построчно запросы на наполнение каталога и сохраняю их в массив
  for (int i = 0; i < query_count; ++i) {
    getline(input_stream, line);
    result.push_back(detail::GetQuery(detail::Trim(line)));
  }
  return result;
}
} // namespace transport::io::stream
