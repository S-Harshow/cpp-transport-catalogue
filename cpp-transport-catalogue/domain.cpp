#include "domain.h"
#include <ostream>

using namespace std;

namespace transport::renderer {
/*--------------------------- RenderSettings ---------------------------------*/
double RenderSettings::width() const { return width_; }

double RenderSettings::height() const { return height_; }

double RenderSettings::padding() const { return padding_; }

double RenderSettings::lineWidth() const { return line_width_; }

double RenderSettings::stopRadius() const { return stop_radius_; }

int RenderSettings::busLabelFontSize() const { return bus_label_font_size_; }

int RenderSettings::stopLabelFontSize() const { return stop_label_font_size_; }

double RenderSettings::underlayerWidth() const { return underlayer_width_; }

const svg::Color &RenderSettings::nextColor() const {
  if (color_palette_.size() == paletteIndex_) {
    paletteIndex_ = 0;
  }
  return color_palette_.at(paletteIndex_++);
}

void RenderSettings::resetColorPalette() const { paletteIndex_ = 0; }

svg::Point RenderSettings::busLabelOffset() const {
  svg::Point result;
  result.x = bus_label_offset_.at(0);
  result.y = bus_label_offset_.at(1);
  return result;
}

svg::Point RenderSettings::stopLabelOffset() const {
  svg::Point result;
  result.x = stop_label_offset_.at(0);
  result.y = stop_label_offset_.at(1);
  return result;
}

const svg::Color &RenderSettings::underlayerColor() const {
  return underlayer_color_;
}

bool RenderSettings::isValid() const {
  if (0 > width_ || MaxRenderSize < width_) {
    return false;
  }
  if (0 > height_ || MaxRenderSize < height_) {
    return false;
  }
  if (0 > padding_ || std::min(width_, height_) < padding_) {
    return false;
  }
  if (0 > line_width_ || MaxRenderSize < line_width_) {
    return false;
  }
  if (0 > stop_radius_ || MaxRenderSize < stop_radius_) {
    return false;
  }
  if (0 > bus_label_font_size_ || MaxRenderSize < bus_label_font_size_) {
    return false;
  }
  if (-MaxRenderSize > bus_label_offset_[0] ||
      MaxRenderSize < bus_label_offset_[0]) {
    return false;
  }
  if (-MaxRenderSize > bus_label_offset_[1] ||
      MaxRenderSize < bus_label_offset_[1]) {
    return false;
  }
  if (0 > stop_label_font_size_ || MaxRenderSize < stop_label_font_size_) {
    return false;
  }
  if (-MaxRenderSize > stop_label_offset_[0] ||
      MaxRenderSize < stop_label_offset_[0]) {
    return false;
  }
  if (-MaxRenderSize > stop_label_offset_[1] ||
      MaxRenderSize < stop_label_offset_[1]) {
    return false;
  }
  if (0 > underlayer_width_ || MaxRenderSize < underlayer_width_) {
    return false;
  }
  if (0 == color_palette_.size()) {
    return false;
  }
  return true;
}

void RenderSettings::setSize(double newWidth, double newHeight,
                             double newPadding) {
  if (0 > newWidth || MaxRenderSize < newWidth) {
    return;
  }
  if (0 > newHeight || MaxRenderSize < newHeight) {
    return;
  }
  if (0 > newPadding || std::min(newWidth, newHeight) < newPadding) {
    return;
  }
  width_ = newWidth;
  height_ = newHeight;
  padding_ = newPadding;
}

void RenderSettings::setStopExterior(
    double newStop_radius, int newStop_label_font_size,
    std::array<double, 2> newStop_label_offset) {
  if (0 > newStop_radius || MaxRenderSize < newStop_radius) {
    return;
  }
  if (0 > newStop_label_font_size || MaxRenderSize < newStop_label_font_size) {
    return;
  }
  if (-MaxRenderSize > newStop_label_offset[0] ||
      MaxRenderSize < newStop_label_offset[0]) {
    return;
  }
  if (-MaxRenderSize > newStop_label_offset[1] ||
      MaxRenderSize < newStop_label_offset[1]) {
    return;
  }
  stop_radius_ = newStop_radius;
  stop_label_offset_ = newStop_label_offset;
  stop_label_font_size_ = newStop_label_font_size;
}

void RenderSettings::setBusExterior(int newBus_label_font_size,
                                    std::array<double, 2> newBus_label_offset) {
  if (0 > newBus_label_font_size || MaxRenderSize < newBus_label_font_size) {
    return;
  }
  if (-MaxRenderSize > newBus_label_offset[0] ||
      MaxRenderSize < newBus_label_offset[0]) {
    return;
  }
  if (-MaxRenderSize > newBus_label_offset[1] ||
      MaxRenderSize < newBus_label_offset[1]) {
    return;
  }
  bus_label_font_size_ = newBus_label_font_size;
  bus_label_offset_ = newBus_label_offset;
}

void RenderSettings::setUnderlayerExterior(
    double newUnderlayer_width, const svg::Color &newUnderlayer_color) {
  if (0 > newUnderlayer_width || MaxRenderSize < newUnderlayer_width) {
    return;
  }
  underlayer_width_ = newUnderlayer_width;
  underlayer_color_ = newUnderlayer_color;
}

void RenderSettings::setLineWidth(double newLine_width) {
  if (0 > newLine_width || MaxRenderSize < newLine_width) {
    return;
  }
  line_width_ = newLine_width;
}

void RenderSettings::setColorPalette(
    const std::vector<svg::Color> &newColor_palette) {
  color_palette_ = newColor_palette;
}

} // namespace transport::renderer

namespace transport {

/*--------------------------- StopStat --------------------------------------*/
StopStat::StopStat(const std::string &name) : name(name) {}
StopStat::StopStat(const std::string &name,
                   const std::vector<std::string_view> &buses)
    : name(name), buses(buses.begin(), buses.end()) {}
/*--------------------------- BusStat ---------------------------------------*/
BusStat::BusStat(std::string_view name) : name(/*std::string(name)*/ name) {}

/*--------------------------- BusQuery --------------------------------------*/
BusQuery::BusQuery(std::string_view name) : name(name) {}

/*--------------------------- StopQuery -------------------------------------*/
StopQuery::StopQuery(std::string_view name) : name(name) {}

/*--------------------------- BusInfo ---------------------------------------*/
BusInfo::BusInfo(const std::string &name) : name(name) {}

/*--------------------------- StopInfo --------------------------------------*/
StopInfo::StopInfo(const std::string &name, detail::Coordinates coordinates)
    : name(name), coordinates(coordinates) {}

/*--------------------------- MapQuery --------------------------------------*/
MapQuery::MapQuery(std::string_view name) : name(name) {}

/*--------------------------- MapStat ---------------------------------------*/
MapStat::MapStat(std::string_view name) : name(name) {}

/*--------------------------- Request ---------------------------------------*/
Request::Request(const RequestValue &value, int request_id)
    : value(value), requestId(request_id) {}

/*--------------------------- Response --------------------------------------*/
Response::Response(const ResponseValue &value, int request_id)
    : value(value), requestId(request_id) {}

void RequestToStream::operator()(const StopQuery &info) const {
  ostream << info;
}

void RequestToStream::operator()(const BusQuery &info) const {
  ostream << info;
}
void RequestToStream::operator()(const MapQuery &info) const {
  ostream << info;
}
void RequestToStream::operator()(const renderer::RenderSettings &info) const {
  ostream << info;
}

void RequestToStream::operator()(std::monostate /*unused*/) const {}

/*--------------------------- stream ----------------------------------------*/
std::ostream &operator<<(std::ostream &ostream, const Response &response) {
  if (response.requestId == 0) {
    ostream << "request_id"sv << response.requestId << ": "sv << response.value
            << std::endl;
  } else {
    ostream << response.value << std::endl;
  }
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const BusStat &value) {
  ostream << "Bus "sv << value.name << ": "sv << std::to_string(value.stops)
          << " stops on route, "sv << std::to_string(value.unique_stops)
          << " unique stops, "sv << std::to_string(value.routelength)
          << " route length, "sv
          << std::to_string(double(value.routelength) / double(value.geolength))
          << " curvature"sv;
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const StopStat &value) {
  ostream << "Stop "sv << value.name << ": "sv << value.buses;
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const MapStat &value) {
  ostream << "Map "sv << value.name << ": "sv << value.map_string;
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const ResponseValue &value) {
  std::visit(ResponseToStream{ostream}, value);
  return ostream;
}

void ResponseToStream::operator()(const StopStat &info) const {
  ostream << info;
}

void ResponseToStream::operator()(const BusStat &info) const {
  ostream << info;
}
void ResponseToStream::operator()(const MapStat &info) const {
  ostream << info;
}

void ResponseToStream::operator()(std::monostate /*unused*/) const {}

std::ostream &operator<<(std::ostream &ostream, const StopQuery &query) {
  ostream << "Stop "sv << query.name << ": "sv << std::to_string(query.latitude)
          << ", "sv << std::to_string(query.longitude);
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const BusQuery &query) {
  ostream << "Bus "sv << query.name << ": "sv << query.stops;
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const RequestValue &value) {
  std::visit(RequestToStream{ostream}, value);
  return ostream;
}

} // namespace transport
