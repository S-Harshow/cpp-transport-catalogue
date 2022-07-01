#include "json_reader.h"
#include "json.h"
#include "json_builder.h"
#include <algorithm>
#include <sstream>

using namespace std;
using namespace transport;
using namespace io::json::detail;
namespace {
inline constexpr const char *JSON_ERROR_MESSAGE = "error_message";
inline constexpr const char *JSON_ERROR_NOT_FOUND = "not found";
inline constexpr const char *JSON_BUS_CURVATURE = "curvature";
inline constexpr const char *JSON_BUS_ROUTE_LENGTH = "route_length";
inline constexpr const char *JSON_BUS_STOP_COUNT = "stop_count";
inline constexpr const char *JSON_BUS_UNIQUE_STOP_COUNT = "unique_stop_count";
inline constexpr const char *JSON_STOP_BUSES = "buses";

inline constexpr const char *JSON_REQUEST_ID = "id";
inline constexpr const char *JSON_RESPONSE_ID = "request_id";
inline constexpr const char *JSON_BASE_REQUESTS = "base_requests";
inline constexpr const char *JSON_STAT_REQUESTS = "stat_requests";
inline constexpr const char *JSON_RENDER_SETTTINGS = "render_settings";
inline constexpr const char *JSON_REQUEST_TYPE = "type";
inline constexpr const char *JSON_REQUEST_BUS = "Bus";
inline constexpr const char *JSON_REQUEST_STOP = "Stop";
inline constexpr const char *JSON_REQUEST_MAP = "Map";
inline constexpr const char *JSON_RESPONSE_MAP = "map";
inline constexpr const char *JSON_NAME = "name";
inline constexpr const char *JSON_LATITUDE = "latitude";
inline constexpr const char *JSON_LONGITUDE = "longitude";
inline constexpr const char *JSON_ROAD_DISTANCES = "road_distances";
inline constexpr const char *JSON_STOPS = "stops";
inline constexpr const char *JSON_IS_ROUNDTRIP = "is_roundtrip";

inline constexpr const char *JSON_RENDER_WIDTH = "width";
inline constexpr const char *JSON_RENDER_HEIGHT = "height";
inline constexpr const char *JSON_RENDER_PADDING = "padding";
inline constexpr const char *JSON_RENDER_LINE_WIDTH = "line_width";
inline constexpr const char *JSON_RENDER_STOP_RADIUS = "stop_radius";
inline constexpr const char *JSON_RENDER_BUS_LABEL_FONT_SIZE =
    "bus_label_font_size";
inline constexpr const char *JSON_RENDER_BUS_LABEL_OFFSET = "bus_label_offset";
inline constexpr const char *JSON_RENDER_STOP_LABEL_FRONT_SIZE =
    "stop_label_font_size";
inline constexpr const char *JSON_RENDER_STOP_LABEL_OFFSET =
    "stop_label_offset";
inline constexpr const char *JSON_RENDER_UNDERLAYER_COLOR = "underlayer_color";
inline constexpr const char *JSON_RENDER_UNDERLAYER_WIDTH = "underlayer_width";
inline constexpr const char *JSON_RENDER_COLOR_PALETTE = "color_palette";
} // namespace
RequestType ParserBase::GetType() const { return RequestType::Base; }

RequestType ParserStat::GetType() const { return RequestType::Stat; }

template <typename Type> inline Type getValue(const Node &node) {
  if constexpr (std::is_same_v<Type, double>) {
    if (node.IsDouble()) {
      return node.AsDouble();
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
    Dict son_map = iter->second.AsDict();
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

std::optional<Requests> ParserRenderSettings::parseSection(const Node &node) {
  if (!node.IsDict()) {
    return {};
  }
  renderer::RenderSettings result{};
  auto settings = node.AsDict();

  auto width = getValue<double>(settings, JSON_RENDER_WIDTH);
  auto height = getValue<double>(settings, JSON_RENDER_HEIGHT);
  auto padding = getValue<double>(settings, JSON_RENDER_PADDING);
  result.setSize(width, height, padding);

  auto stop_radius = getValue<double>(settings, JSON_RENDER_STOP_RADIUS);
  auto stop_label_font_size =
      getValue<uint>(settings, JSON_RENDER_STOP_LABEL_FRONT_SIZE);
  auto stop_label_offset =
      getArray<double, 2>(settings, JSON_RENDER_STOP_LABEL_OFFSET);
  result.setStopExterior(stop_radius, stop_label_font_size, stop_label_offset);

  auto bus_label_font_size =
      getValue<uint>(settings, JSON_RENDER_BUS_LABEL_FONT_SIZE);
  auto bus_label_offset =
      getArray<double, 2>(settings, JSON_RENDER_BUS_LABEL_OFFSET);
  result.setBusExterior(bus_label_font_size, bus_label_offset);

  auto underlayer_width =
      getValue<double>(settings, JSON_RENDER_UNDERLAYER_WIDTH);

  svg::Color underlayer_color = svg::NoneColor;
  if (const auto iter = settings.find(JSON_RENDER_UNDERLAYER_COLOR);
      iter != settings.end()) {
    underlayer_color = getColor(iter->second);
  }
  result.setUnderlayerExterior(underlayer_width, underlayer_color);

  auto line_width = getValue<double>(settings, JSON_RENDER_LINE_WIDTH);
  result.setLineWidth(line_width);

  std::vector<svg::Color> color_palette{};
  if (const auto iter = settings.find(JSON_RENDER_COLOR_PALETTE);
      iter != settings.end() && iter->second.IsArray()) {
    Array palette = iter->second.AsArray();
    color_palette.reserve(palette.size());
    transform(palette.begin(), palette.end(), back_inserter(color_palette),
              [](const auto &color_node) { return getColor(color_node); });
  }
  result.setColorPalette(color_palette);

  if (result.isValid()) {
    return make_optional<Requests>({{result, 0}});
  }
  return nullopt;
}

RequestType ParserRenderSettings::GetType() const {
  return RequestType::Render;
}

//} // namespace transport::io::json::detail

/*--------------------------- JsonOutputter ----------------------------------*/
transport::JsonOutputter::JsonOutputter(std::ostream &output_stream)
    : output_stream_(output_stream) {
  output_stream_.unsetf(ios::fixed);
}

void transport::JsonOutputter::SetResponses(const Responses &responses) {
  Array root_array{};
  transform(responses.begin(), responses.end(), back_inserter(root_array),
            [](const Response &response) {
              return std::visit(JsonNodeBuilder{response.requestId},
                                response.value);
            });
  if (!root_array.empty()) {
    // output_stream_.setf(ios::scientific);
    //     Print(Document(Node(root_array)), output_stream_);
    Print(json::Document(json::Builder{}.Value(root_array).Build()),
          output_stream_);
  }
}

/*--------------------------- JsonInputter ----------------------------------*/
transport::JsonInputter::JsonInputter(std::istream &input_stream)
    : input_stream_(input_stream) {
  //  ParseInput();
}

void transport::JsonInputter::ParseInput() {
  //  if (input_stream_.rdbuf()->in_avail() == 0) {
  //    return;
  //  }
  Dict root;
  try {
    Document request_doc(Load(input_stream_));
    const Node &root_node = request_doc.GetRoot();
    if (root_node.IsDict()) {
      root = root_node.AsDict();
    }
  } catch (const std::exception &e) {
    std::cout << "A standard exception was caught, with message '" << e.what()
              << "'" << std::endl;
    return;
  }
  for_each(root.begin(), root.end(), [this](const auto &section) {
    // Выбрать парсер по названию элемента словаря
    auto parser = ParserBuilder::CreateParser(section.first);
    if (auto requests = parser->parseSection(section.second)) {
      requests_repo_.emplace(parser->GetType(), requests.value());
    }
  });
}

std::optional<transport::Requests>
transport::JsonInputter::GetRequests(RequestType request_type) const {
  if (const auto iter = requests_repo_.find(request_type);
      iter != requests_repo_.end()) {
    return {iter->second};
  }
  return nullopt;
}

// namespace transport::io::json::detail {

/*--------------------------- JsonNodeBuilder -------------------------------*/
Node JsonNodeBuilder::operator()(const StopStat &info) const {
  Array info_buses{};
  transform(info.buses.begin(), info.buses.end(), back_inserter(info_buses),
            [](const std::string_view bus) { return Node(std::string(bus)); });
  return Builder{}
      .StartDict()
      .Key(JSON_RESPONSE_ID)
      .Value(request_id)
      .Key(JSON_STOP_BUSES)
      .Value(info_buses)
      .EndDict()
      .Build();
  //  Dict info_map{};
  //  info_map.emplace(JSON_RESPONSE_ID, Node(request_id));
  //  Array info_buses{};
  //  transform(info.buses.begin(), info.buses.end(), back_inserter(info_buses),
  //            [](const std::string_view bus) { return Node(std::string(bus));
  //            });
  //  info_map.emplace(JSON_STOP_BUSES, info_buses);
  //  return {info_map};
}

Node JsonNodeBuilder::operator()(const BusStat &info) const {
  return Builder{}
      .StartDict()
      .Key(JSON_RESPONSE_ID)
      .Value(request_id)
      .Key(JSON_BUS_CURVATURE)
      .Value(info.routelength / info.geolength)
      .Key(JSON_BUS_ROUTE_LENGTH)
      .Value(static_cast<double>(info.routelength))
      .Key(JSON_BUS_STOP_COUNT)
      .Value(static_cast<int>(info.stops))
      .Key(JSON_BUS_UNIQUE_STOP_COUNT)
      .Value(static_cast<int>(info.unique_stops))
      .EndDict()
      .Build();

  //  Dict info_map{};
  //  info_map.emplace(JSON_RESPONSE_ID, Node(request_id));
  //  info_map.emplace(JSON_BUS_CURVATURE, Node(info.routelength /
  //  info.geolength)); info_map.emplace(JSON_BUS_ROUTE_LENGTH,
  //                   Node(static_cast<double>(info.routelength)));
  //  info_map.emplace(JSON_BUS_STOP_COUNT, Node(static_cast<int>(info.stops)));
  //  info_map.emplace(JSON_BUS_UNIQUE_STOP_COUNT,
  //                   Node(static_cast<int>(info.unique_stops)));
  //  return {info_map};
}
Node JsonNodeBuilder::operator()(const MapStat &info) const {
  return Builder{}
      .StartDict()
      .Key(JSON_RESPONSE_ID)
      .Value(request_id)
      .Key(JSON_RESPONSE_MAP)
      .Value(info.map_string)
      .EndDict()
      .Build();
  //  Dict info_map{};
  //  info_map.emplace(JSON_RESPONSE_ID, Node(request_id));
  //  info_map.emplace(JSON_RESPONSE_MAP, Node(info.map_string));
  //  return {info_map};
}

Node JsonNodeBuilder::operator()(const std::monostate & /*unused*/) const {
  return Builder{}
      .StartDict()
      .Key(JSON_RESPONSE_ID)
      .Value(request_id)
      .Key(JSON_ERROR_MESSAGE)
      .Value(std::string(JSON_ERROR_NOT_FOUND))
      .EndDict()
      .Build();

  //  Dict info_map{};
  //  info_map.emplace(JSON_RESPONSE_ID, Node(request_id));
  //  info_map.emplace(JSON_ERROR_MESSAGE,
  //  Node(std::string(JSON_ERROR_NOT_FOUND))); return {info_map};
}

/*--------------------------- ParserBuilder --------------------------------*/
std::shared_ptr<Parser>
ParserBuilder::CreateParser(const std::string &parser_type) {
  if (JSON_BASE_REQUESTS == parser_type) {
    return std::make_shared<ParserBase>();
  }
  if (JSON_STAT_REQUESTS == parser_type) {
    return std::make_shared<ParserStat>();
  }
  if (JSON_RENDER_SETTTINGS == parser_type) {
    return std::make_shared<ParserRenderSettings>();
  }

  return {nullptr};
}

/*------------------------------ Parser -----------------------------------*/
std::optional<Requests> Parser::parseSection(const Node &map) {
  if (!map.IsArray()) {
    return {};
  }
  Requests result{};
  Array array = map.AsArray();
  for (const auto &request_node : array) {
    if (!request_node.IsDict()) {
      continue;
    }
    const Dict &request = request_node.AsDict();
    auto request_type = getValue<string>(request, JSON_REQUEST_TYPE);
    if (request_type.empty()) {
      continue;
    }
    if (JSON_REQUEST_BUS == request_type) {
      if (auto query = parseBusNode(request); query.has_value()) {
        result.emplace_back(query.value());
      }
    }
    if (JSON_REQUEST_STOP == request_type) {
      if (auto query = parseStopNode(request); query.has_value()) {
        result.emplace_back(query.value());
      }
    }
    if (JSON_REQUEST_MAP == request_type) {
      if (auto query = parseMapNode(request); query.has_value()) {
        result.emplace_back(query.value());
      }
    }
  }
  return {result};
}

std::optional<Request> Parser::parseBusNode(const Dict &map) {
  auto name = getValue<string>(map, JSON_NAME);
  if (name.empty()) {
    return nullopt;
  }
  BusQuery result(name);
  result.is_roundtrip = getValue<bool>(map, JSON_IS_ROUNDTRIP);
  result.stops = getVector<string>(map, JSON_STOPS);
  return make_optional<Request>({result, 0});
}

std::optional<Request> Parser::parseStopNode(const Dict &map) {

  auto name = getValue<string>(map, JSON_NAME);
  if (name.empty()) {
    return nullopt;
  }
  StopQuery result(name);
  result.latitude = getValue<double>(map, JSON_LATITUDE);
  result.longitude = getValue<double>(map, JSON_LONGITUDE);
  result.road_distances = getMap<int>(map, JSON_ROAD_DISTANCES);
  return make_optional<Request>({result, 0});
}

std::optional<Request> Parser::parseMapNode(const Dict & /*map*/) {
  MapQuery result(
      ""); // Пока самого запроса хватит для постоения карты всех маршрутов
  return make_optional<Request>({result, 0}); // Заглушка
}

std::optional<Request> ParserStat::parseBusNode(const Dict &map) {
  if (auto request = Parser::parseBusNode(map); request.has_value()) {
    request->requestId = getValue<int>(map, JSON_REQUEST_ID);
    return request;
  }
  return nullopt;
}

std::optional<Request> ParserStat::parseStopNode(const Dict &map) {
  if (auto request = Parser::parseStopNode(map); request.has_value()) {
    request->requestId = getValue<int>(map, JSON_REQUEST_ID);
    return request;
  }
  return nullopt;
}

std::optional<Request> ParserStat::parseMapNode(const Dict &map) {
  if (auto request = Parser::parseMapNode(map); request.has_value()) {
    request->requestId = getValue<int>(map, JSON_REQUEST_ID);
    return request;
  }
  return nullopt;
}
//} // namespace transport::io::json::detail
