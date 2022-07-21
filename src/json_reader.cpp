#include "json_reader.h"
#include "json/json_builder.h"
#include <algorithm>
#include <execution>
#include <limits>
#include <sstream>

using namespace std;

using namespace transport;
using namespace json;
using namespace io::json::detail;

namespace {
// об ошибках
inline constexpr const char *ERROR_FIELD = "error_message";
inline constexpr const char *ERROR_MESSAGE = "not found";

// названия запросов на внесение информации
inline constexpr const char *BASE_REQUESTS = "base_requests";
inline constexpr const char *RENDER_SETTTINGS = "render_settings";
inline constexpr const char *ROUTING_SETTINGS = "routing_settings";
inline constexpr const char *STAT_REQUESTS = "stat_requests";

// Названия полей. Общее для многих разделов
inline constexpr const char *TYPE_FIELD = "type";
inline constexpr const char *NAME_FIELD = "name";

// Названия полей. Раздел base_requests
inline constexpr const char *BASE_REQUESTS_STOPS_FIELD = "stops";
inline constexpr const char *BASE_REQUESTS_IS_ROUNDTRIP = "is_roundtrip";
inline constexpr const char *BASE_REQUESTS_LATITUDE = "latitude";
inline constexpr const char *BASE_REQUESTS_LONGITUDE = "longitude";
inline constexpr const char *BASE_REQUESTS_ROAD_DISTANCES = "road_distances";

// Названия полей. Раздел render_settings
inline constexpr const char *RENDER_SETTINGS_WIDTH = "width";
inline constexpr const char *RENDER_SETTINGS_HEIGHT = "height";
inline constexpr const char *RENDER_SETTINGS_PADDING = "padding";
inline constexpr const char *RENDER_SETTINGS_LINE_WIDTH = "line_width";
inline constexpr const char *RENDER_SETTINGS_STOP_RADIUS = "stop_radius";
inline constexpr const char *RENDER_SETTINGS_BUS_LABEL_FONT_SIZE =
    "bus_label_font_size";
inline constexpr const char *RENDER_SETTINGS_BUS_LABEL_OFFSET =
    "bus_label_offset";
inline constexpr const char *RENDER_SETTINGS_STOP_LABEL_FRONT_SIZE =
    "stop_label_font_size";
inline constexpr const char *RENDER_SETTINGS_STOP_LABEL_OFFSET =
    "stop_label_offset";
inline constexpr const char *RENDER_SETTINGS_UNDERLAYER_COLOR =
    "underlayer_color";
inline constexpr const char *RENDER_SETTINGS_UNDERLAYER_WIDTH =
    "underlayer_width";
inline constexpr const char *RENDER_SETTINGS_COLOR_PALETTE = "color_palette";

// Названия полей. Раздел routing_settings
inline constexpr const char *ROUTING_SETTINGS_BUS_WAIT = "bus_wait_time";
inline constexpr const char *ROUTING_SETTINGS_BUS_VELOCITY = "bus_velocity";

// Названия полей. Раздел stat_requests
inline constexpr const char *JSON_REQUEST_ID = "id";
// Названия полей. Раздел stat_requests_Route
inline constexpr const char *STATS_ROUTE_STOP_FROM = "from";
inline constexpr const char *STATS_ROUTE_STOP_TO = "to";

// Типы запросов/ответов
inline constexpr const char *REQUEST_BUS = "Bus";
inline constexpr const char *REQUEST_STOP = "Stop";
inline constexpr const char *REQUEST_MAP = "Map";
inline constexpr const char *REQUEST_ROUTE = "Route";

// Названия полей. Общее
inline constexpr const char *RESPONSE_ID = "request_id";

// Названия полей. Ответы на запрос Автобуса
inline constexpr const char *BUS_CURVATURE = "curvature";
inline constexpr const char *BUS_ROUTE_LENGTH = "route_length";
inline constexpr const char *BUS_STOP_COUNT = "stop_count";
inline constexpr const char *BUS_UNIQUE_STOP_COUNT = "unique_stop_count";

// Названия полей. Ответы на запрос Остановка
inline constexpr const char *STOP_BUSES = "buses";

// Названия полей. Ответы на запрос Карта
inline constexpr const char *RESPONSE_MAP = "map";

// Названия полей. Ответы на запрос Маршрут
inline constexpr const char *ROUTE_RESPONSE_TOTAL_TIME = "total_time";
inline constexpr const char *ROUTE_RESPONSE_TIME = "time";
inline constexpr const char *ROUTE_RESPONSE_ITEMS = "items";
inline constexpr const char *ROUTE_RESPONSE_STOP_NAME = "stop_name";
inline constexpr const char *ROUTE_RESPONSE_BUS_NAME = "bus";
inline constexpr const char *ROUTE_RESPONSE_SPAN_COUNT = "span_count";
inline constexpr const char *ROUTE_RESPONSE_WAIT = "Wait";

} // namespace

template <typename Type> inline Type getValue(const Node &node) {
  if constexpr (std::is_same_v<Type, double>) {
    if (node.IsDouble()) {
      return node.AsDouble();
    }
  }
  if constexpr (std::is_same_v<Type, float>) {
    if (node.IsDouble()) {
      return static_cast<float>(node.AsDouble());
    }
  }
  if constexpr (std::is_same_v<Type, int>) {
    if (node.IsInt()) {
      return node.AsInt();
    }
  }
  if constexpr (std::is_same_v<Type, uint>) {
    if (node.IsInt()) {
      return static_cast<uint>(node.AsInt());
    }
  }
  if constexpr (std::is_same_v<Type, std::string>) {
    if (node.IsString()) {
      return node.AsString();
    }
  }
  if constexpr (std::is_same_v<Type, bool>) {
    if (node.IsBool()) {
      return node.AsBool();
    }
  }
  return {};
}

template <typename Type>
inline Type getValue(const Dict &map, const string &json_var_name) {
  Type result{};
  if (const auto iter = map.find(json_var_name); iter != map.end()) {
    result = getValue<Type>(iter->second);
  }
  return result;
}

template <typename Type>
inline std::vector<Type> getVector(const Dict &map,
                                   const string &json_var_name) {
  std::vector<Type> result{};
  if (const auto iter = map.find(json_var_name);
      iter != map.end() && iter->second.IsArray()) {
    Array son_vector = iter->second.AsArray();
    result.reserve(son_vector.size());
    transform(son_vector.begin(), son_vector.end(), back_inserter(result),
              [](const auto &node) { return getValue<Type>(node); });
  }
  return result;
}

template <typename Type, size_t Count>
inline std::array<Type, Count> getArray(const Node &node) {
  std::array<Type, Count> result{};
  if (Array array = node.AsArray(); !array.empty()) {
    for (size_t i = 0; i < std::min(array.size(), Count); ++i) {
      result.at(i) = getValue<Type>(array.at(i));
    }
  }
  return result;
}

template <typename Type, size_t Count>
inline std::array<Type, Count> getArray(const Dict &map,
                                        const string &json_var_name) {
  std::array<Type, Count> result{};
  if (const auto iter = map.find(json_var_name);
      iter != map.end() && iter->second.IsArray()) {
    result = getArray<Type, Count>(iter->second);
  }
  return result;
}

template <typename Type>
inline std::map<std::string, Type> getMap(const Dict &map,
                                          const string &json_var_name) {
  std::map<std::string, Type> result{};
  if (const auto iter = map.find(json_var_name);
      iter != map.end() && iter->second.IsDict()) {
    const Dict son_map{iter->second.AsDict()};
    for (const auto &[name, node] : son_map) {
      result.emplace(name, getValue<Type>(son_map, name));
    }
  }
  return result;
}

inline svg::Color getColor(const Node &node) {
  // может строка?
  if (node.IsString()) {
    return node.AsString();
  }
  // тогда может массив
  if (node.IsArray()) {
    if (3 == node.AsArray().size()) {
      auto color_array =
          getArray<double, 3>(node); //гарантированно будет 3 элемента
      return svg::Rgb(static_cast<uint8_t>(color_array.at(0)),
                      static_cast<uint8_t>(color_array.at(1)),
                      static_cast<uint8_t>(color_array.at(2)));
    }
    if (4 == node.AsArray().size()) {
      auto color_array =
          getArray<double, 4>(node); //гарантированно будет 4 элемента
      return svg::Rgba(static_cast<uint8_t>(color_array.at(0)),
                       static_cast<uint8_t>(color_array.at(1)),
                       static_cast<uint8_t>(color_array.at(2)),
                       color_array.at(3));
    }
  }
  return svg::NoneColor;
}

uniqueQueryList ParserRenderSettings::parseSection(const Node &node) const {
  if (!node.IsDict()) {
    return {};
  }
  renderer::RenderSettings settings{};
  const auto &settings_dict = node.AsDict();

  const auto width{getValue<double>(settings_dict, RENDER_SETTINGS_WIDTH)};
  const auto height{getValue<double>(settings_dict, RENDER_SETTINGS_HEIGHT)};
  const auto padding{getValue<double>(settings_dict, RENDER_SETTINGS_PADDING)};
  settings.setSize(width, height, padding);

  const auto stop_radius =
      getValue<double>(settings_dict, RENDER_SETTINGS_STOP_RADIUS);
  const auto stop_label_font_size =
      getValue<uint>(settings_dict, RENDER_SETTINGS_STOP_LABEL_FRONT_SIZE);
  const auto stop_label_offset =
      getArray<double, 2>(settings_dict, RENDER_SETTINGS_STOP_LABEL_OFFSET);
  settings.setStopExterior(
      stop_radius, stop_label_font_size,
      make_pair(stop_label_offset[0], stop_label_offset[1]));

  const auto bus_label_font_size =
      getValue<uint>(settings_dict, RENDER_SETTINGS_BUS_LABEL_FONT_SIZE);
  const auto bus_label_offset =
      getArray<double, 2>(settings_dict, RENDER_SETTINGS_BUS_LABEL_OFFSET);
  settings.setBusExterior(bus_label_font_size,
                          make_pair(bus_label_offset[0], bus_label_offset[1]));

  const auto underlayer_width =
      getValue<double>(settings_dict, RENDER_SETTINGS_UNDERLAYER_WIDTH);

  svg::Color underlayer_color = svg::NoneColor;
  if (const auto iter = settings_dict.find(RENDER_SETTINGS_UNDERLAYER_COLOR);
      iter != settings_dict.end()) {
    underlayer_color = getColor(iter->second);
  }
  settings.setUnderlayerExterior(underlayer_width, underlayer_color);

  const auto line_width =
      getValue<double>(settings_dict, RENDER_SETTINGS_LINE_WIDTH);
  settings.setLineWidth(line_width);

  std::vector<svg::Color> color_palette{};
  if (const auto iter = settings_dict.find(RENDER_SETTINGS_COLOR_PALETTE);
      iter != settings_dict.end() && iter->second.IsArray()) {
    const Array palette = iter->second.AsArray();
    color_palette.reserve(palette.size());
    transform(palette.begin(), palette.end(), back_inserter(color_palette),
              [](const auto &color_node) { return getColor(color_node); });
  }
  settings.setColorPalette(color_palette);

  uniqueQueryList result{};
  if (settings.isValid()) {
    result.push_back(queries::map::RenderSettings::Factory()
                         .SetSettings(settings)
                         .Construct());
  }
  return result;
}

/*--------------------------- JsonOutputter ----------------------------------*/
transport::JsonOutputter::JsonOutputter(ostream &output_stream)
    : output_stream_(output_stream) {
  //  output_stream_.unsetf(ios::fixed);
}

void JsonOutputter::Register() {
  IOFactory<Outputter>::instance().Register("json"sv, JsonOutputter::Construct);
}

std::unique_ptr<Outputter> JsonOutputter::Construct(ostream &stream) {
  return make_unique<JsonOutputter>(stream);
}

void JsonOutputter::Send() {
  if (!root_array_.empty()) {
    Print(json::Document(json::Builder{}.Value(root_array_).Build()),
          output_stream_);
  }
  root_array_.clear();
}

void JsonOutputter::visit(queries::EmptyResponse *response) {
  root_array_.emplace_back(Builder{}
                               .StartDict()
                               .Key(RESPONSE_ID)
                               .Value(response->getId())
                               .Key(ERROR_FIELD)
                               .Value(std::string(ERROR_MESSAGE))
                               .EndDict()
                               .Build());
}

void JsonOutputter::visit(queries::bus::StatResponse *response) {
  root_array_.emplace_back(
      Builder{}
          .StartDict()
          .Key(RESPONSE_ID)
          .Value(response->getId())
          .Key(BUS_CURVATURE)
          .Value(static_cast<double>(response->getRouteLength() /
                                     response->getGeoLength()))
          .Key(BUS_ROUTE_LENGTH)
          .Value(response->getRouteLength())
          .Key(BUS_STOP_COUNT)
          .Value(response->getStopsCount())
          .Key(BUS_UNIQUE_STOP_COUNT)
          .Value(response->getUniqueStopsCount())
          .EndDict()
          .Build());
}

void JsonOutputter::visit(queries::stop::StatResponse *response) {
  Array info_buses{};
  transform(response->buses_cbegin(), response->buses_cend(),
            back_inserter(info_buses),
            [](const std::string &bus) { return Node(bus); });
  root_array_.emplace_back(Builder{}
                               .StartDict()
                               .Key(RESPONSE_ID)
                               .Value(response->getId())
                               .Key(STOP_BUSES)
                               .Value(info_buses)
                               .EndDict()
                               .Build());
}

void JsonOutputter::visit(queries::map::MapResponse *response) {
  root_array_.emplace_back(Builder{}
                               .StartDict()
                               .Key(RESPONSE_ID)
                               .Value(response->getId())
                               .Key(RESPONSE_MAP)
                               .Value(response->getMap())
                               .EndDict()
                               .Build());
}

void JsonOutputter::visit(queries::router::RouteResponse *response) {
  struct items_to_node {
    Node operator()(const RouteItemBus &bus_info) const {
      return Builder{}
          .StartDict()
          .Key(TYPE_FIELD)
          .Value(REQUEST_BUS)
          .Key(ROUTE_RESPONSE_BUS_NAME)
          .Value(string(bus_info.bus_name))
          .Key(ROUTE_RESPONSE_SPAN_COUNT)
          .Value(bus_info.span_count)
          .Key(ROUTE_RESPONSE_TIME)
          .Value(bus_info.time)
          .EndDict()
          .Build();
    };
    Node operator()(const RouteItemStop &stop_info) const {
      return Builder{}
          .StartDict()
          .Key(TYPE_FIELD)
          .Value(ROUTE_RESPONSE_WAIT)
          .Key(ROUTE_RESPONSE_STOP_NAME)
          .Value(string(stop_info.name))
          .Key(ROUTE_RESPONSE_TIME)
          .Value(stop_info.wait_time)
          .EndDict()
          .Build();
    };
  };

  Array items{};
  items.reserve(response->items_size());
  transform(
      response->items_cbegin(), response->items_cend(), back_inserter(items),
      [](const RouteItem &item) { return std::visit(items_to_node{}, item); });
  root_array_.emplace_back(Builder{}
                               .StartDict()
                               .Key(RESPONSE_ID)
                               .Value(response->getId())
                               .Key(ROUTE_RESPONSE_TOTAL_TIME)
                               .Value(response->getTotalTime())
                               .Key(ROUTE_RESPONSE_ITEMS)
                               .Value(items)
                               .EndDict()
                               .Build());
}

/*--------------------------- JsonInputter ----------------------------------*/
transport::JsonInputter::JsonInputter(istream &input_stream)
    : input_stream_(input_stream) {}

void JsonInputter::Register() {
  IOFactory<Inputter>::instance().Register("json"sv, JsonInputter::Construct);
}

std::unique_ptr<Inputter> JsonInputter::Construct(istream &stream) {
  return make_unique<JsonInputter>(stream);
}

uniqueQueryList::const_iterator transport::JsonInputter::cbegin() const {
  return requests_.cbegin();
}

uniqueQueryList::iterator JsonInputter::begin() { return requests_.begin(); }

uniqueQueryList::const_iterator JsonInputter::cend() const {
  return requests_.cend();
}

uniqueQueryList::iterator JsonInputter::end() { return requests_.end(); }

void transport::JsonInputter::Parse() {
  requests_.clear();
  if (!input_stream_) {
    input_stream_.clear();
    input_stream_.ignore();
    return;
  }
  if (input_stream_ && input_stream_.peek(), input_stream_.eof()) {
    return;
  }

  Dict root;
  try {
    Document request_doc(Load(input_stream_));
    const Node &root_node = request_doc.GetRoot();
    if (root_node.IsDict()) {
      root = root_node.AsDict();
    }
  } catch (const std::exception &e) {
    std::cout << "A standard exception was caught, with message '"sv << e.what()
              << "'"sv << std::endl;
    return;
  }

  input_stream_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for_each(root.begin(), root.end(), [this](const auto &section) {
    // Выбрать парсер по названию элемента словаря
    const auto &parser =
        transport::io::json::detail::ParserBuilder::CreateParser(section.first);
    auto requests = parser.parseSection(section.second);
    move(requests.begin(), requests.end(), back_inserter(requests_));
  });
}

/*--------------------------- ParserBuilder --------------------------------*/
const Parser &ParserBuilder::CreateParser(string_view parser_type) {
  static ParserBase base;
  static ParserStat stat;
  static ParserRenderSettings render_settings;
  static ParserRoutingSettings routing_settings;
  static std::unordered_map<std::string_view, const Parser &> factories = {
      {BASE_REQUESTS, base},
      {STAT_REQUESTS, stat},
      {RENDER_SETTTINGS, render_settings},
      {ROUTING_SETTINGS, routing_settings}};

  return factories.at(parser_type);
}

/*------------------------------ Parser -----------------------------------*/
uniqueQueryList ParserBase::parseSection(const Node &map) const {
  if (!map.IsArray()) {
    return {};
  }
  Array array = map.AsArray();
  uniqueQueryList result;

  for (const auto &request_node : array) {
    if (!request_node.IsDict()) {
      continue;
    }
    const Dict request = request_node.AsDict();
    auto request_type = getValue<string>(request, TYPE_FIELD);
    if (request_type.empty()) {
      continue;
    }
    if (REQUEST_BUS == request_type) {
      if (auto query = parseBusNode(request); query) {
        result.push_back(move(query));
      }
    }
    if (REQUEST_STOP == request_type) {
      if (auto query = parseStopNode(request); query != nullptr) {
        result.push_back(move(query));
      }
    }
  }
  return result;
}

uniqueQuery ParserBase::parseBusNode(const Dict &map) {
  const auto name = getValue<string>(map, NAME_FIELD);
  if (name.empty()) {
    return nullptr;
  }
  const auto is_roundtrip = getValue<bool>(map, BASE_REQUESTS_IS_ROUNDTRIP);
  const auto stops = getVector<string>(map, BASE_REQUESTS_STOPS_FIELD);
  return queries::bus::AddBusQuery::Factory()
      .SetName(name)
      .SetRoundTripMark(is_roundtrip)
      .SetStops(stops)
      .Construct();
}

uniqueQuery ParserBase::parseStopNode(const Dict &map) {
  const auto name = getValue<string>(map, NAME_FIELD);
  if (name.empty()) {
    return nullptr;
  }
  const auto latitude = getValue<double>(map, BASE_REQUESTS_LATITUDE);
  const auto longitude = getValue<double>(map, BASE_REQUESTS_LONGITUDE);
  const auto road_distances = getMap<double>(map, BASE_REQUESTS_ROAD_DISTANCES);
  return queries::stop::AddStopQuery::Factory()
      .SetName(name)
      .SetCoordinates(latitude, longitude)
      .SetDistances(road_distances)
      .Construct();
}

uniqueQueryList ParserStat::parseSection(const Node &map) const {
  if (!map.IsArray()) {
    return {};
  }
  uniqueQueryList result{};
  Array array = map.AsArray();
  for (const auto &request_node : array) {
    if (!request_node.IsDict()) {
      continue;
    }
    const Dict &request = request_node.AsDict();
    auto request_type{getValue<string>(request, TYPE_FIELD)};
    if (request_type.empty()) {
      continue;
    }
    if (REQUEST_BUS == request_type) {
      if (auto query = parseBusNode(request); query) {
        result.push_back(move(query));
      }
    }
    if (REQUEST_STOP == request_type) {
      if (auto query = parseStopNode(request); query) {
        result.push_back(move(query));
      }
    }
    if (REQUEST_MAP == request_type) {
      if (auto query = parseMapNode(request); query) {
        result.push_back(move(query));
      }
    }
    if (REQUEST_ROUTE == request_type) {
      if (auto query = parseRouteNode(request); query) {
        result.push_back(move(query));
      }
    }
  }
  return result;
}

uniqueQuery ParserStat::parseBusNode(const Dict &map) {
  const auto name = getValue<string>(map, NAME_FIELD);
  if (name.empty()) {
    return nullptr;
  }
  const auto requestId = getValue<int>(map, JSON_REQUEST_ID);
  return queries::bus::StatBusQuery::Factory()
      .SetName(name)
      .SetId(requestId)
      .Construct();
}

uniqueQuery ParserStat::parseStopNode(const Dict &map) {
  const auto name = getValue<string>(map, NAME_FIELD);
  if (name.empty()) {
    return nullptr;
  }
  const auto requestId = getValue<int>(map, JSON_REQUEST_ID);
  return queries::stop::StatStopQuery::Factory()
      .SetName(name)
      .SetId(requestId)
      .Construct();
}

uniqueQuery ParserStat::parseMapNode(const Dict &map) {
  // Пока самого запроса хватит для постоения карты всех маршрутов
  const auto requestId = getValue<int>(map, JSON_REQUEST_ID);
  return queries::map::MapRender::Factory().SetId(requestId).Construct();
}

uniqueQuery ParserStat::parseRouteNode(const Dict &map) {
  const auto stop_from = getValue<string>(map, STATS_ROUTE_STOP_FROM);
  const auto stop_to = getValue<string>(map, STATS_ROUTE_STOP_TO);
  const auto requestId = getValue<int>(map, JSON_REQUEST_ID);
  return queries::router::RouteQuery::Factory()
      .SetFromStop(stop_from)
      .SetToStop(stop_to)
      .SetId(requestId)
      .Construct();
}

uniqueQueryList ParserRoutingSettings::parseSection(const Node &node) const {
  if (!node.IsDict()) {
    return {};
  }
  const auto &settings_dict = node.AsDict();

  const auto bus_wait =
      getValue<double>(settings_dict, ROUTING_SETTINGS_BUS_WAIT);
  const auto bus_velocity =
      getValue<double>(settings_dict, ROUTING_SETTINGS_BUS_VELOCITY);

  auto query = queries::router::RoutingSettings::Factory()
                   .SetBusWaitTime(bus_wait)
                   .SetBusVelocity(bus_velocity)
                   .Construct();
  uniqueQueryList result{};
  result.push_back(move(query));
  return result;
}
