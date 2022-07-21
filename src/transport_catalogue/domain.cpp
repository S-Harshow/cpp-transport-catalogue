#include "domain.h"
#include <ostream>

using namespace std;
namespace transport {
namespace renderer {
/*--------------------------- RenderSettings ---------------------------------*/
double RenderSettings::width() const { return width_; }

double RenderSettings::height() const { return height_; }

double RenderSettings::padding() const { return padding_; }

double RenderSettings::lineWidth() const { return line_width_; }

double RenderSettings::stopRadius() const { return stop_radius_; }

uint32_t RenderSettings::busLabelFontSize() const {
  return bus_label_font_size_;
}

uint32_t RenderSettings::stopLabelFontSize() const {
  return stop_label_font_size_;
}

double RenderSettings::underlayerWidth() const { return underlayer_width_; }

const svg::Color &RenderSettings::nextColor() const {
  if (color_palette_.size() == paletteIndex_) {
    paletteIndex_ = 0;
  }
  return color_palette_.at(paletteIndex_++);
}

void RenderSettings::resetColorPalette() const { paletteIndex_ = 0; }

svg::Point RenderSettings::busLabelOffset() const { return bus_label_offset_; }

svg::Point RenderSettings::stopLabelOffset() const {
  return stop_label_offset_;
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
  if (MaxRenderSize < bus_label_font_size_) {
    return false;
  }
  if (-MaxRenderSize > bus_label_offset_.x ||
      MaxRenderSize < bus_label_offset_.x) {
    return false;
  }
  if (-MaxRenderSize > bus_label_offset_.y ||
      MaxRenderSize < bus_label_offset_.y) {
    return false;
  }
  if (MaxRenderSize < stop_label_font_size_) {
    return false;
  }
  if (-MaxRenderSize > stop_label_offset_.x ||
      MaxRenderSize < stop_label_offset_.x) {
    return false;
  }
  if (-MaxRenderSize > stop_label_offset_.y ||
      MaxRenderSize < stop_label_offset_.y) {
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

RenderSettings::operator bool() const { return isValid(); }

RenderSettings::operator std::string() const {
  ostringstream ostream;
  ostream << "\"render_settings\": {\"width\": "sv << width_
          << ",\"height\": "sv << height_ << ",\"padding\": "sv << padding_
          << ",\"stop_radius\": "sv << stop_radius_ << ",\"line_width\": "sv
          << line_width_ << ",\"bus_label_font_size\": "sv
          << bus_label_font_size_ << ",\"bus_label_offset\": "sv
          << bus_label_offset_ << ",\"stop_label_font_size\": "sv
          << stop_label_font_size_ << ",\"stop_label_offset\": "sv
          << stop_label_offset_ << ",\"underlayer_color\": "sv
          << underlayer_color_ << ",\"underlayer_width\": "sv
          << underlayer_width_ << ",\"color_palette\": "sv << color_palette_
          << "]}"sv << endl;
  return ostream.str();
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
    double newStop_radius, uint32_t newStop_label_font_size,
    std::pair<double, double> newStop_label_offset) {
  if (0 > newStop_radius || MaxRenderSize < newStop_radius) {
    return;
  }
  if (MaxRenderSize < newStop_label_font_size) {
    return;
  }
  if (-MaxRenderSize > newStop_label_offset.first ||
      MaxRenderSize < newStop_label_offset.first) {
    return;
  }
  if (-MaxRenderSize > newStop_label_offset.second ||
      MaxRenderSize < newStop_label_offset.second) {
    return;
  }
  stop_radius_ = newStop_radius;
  stop_label_offset_ = {newStop_label_offset.first,
                        newStop_label_offset.second};
  stop_label_font_size_ = newStop_label_font_size;
}

void RenderSettings::setBusExterior(
    uint32_t newBus_label_font_size,
    std::pair<double, double> newBus_label_offset) {
  if (MaxRenderSize < newBus_label_font_size) {
    return;
  }
  if (-MaxRenderSize > newBus_label_offset.first ||
      MaxRenderSize < newBus_label_offset.first) {
    return;
  }
  if (-MaxRenderSize > newBus_label_offset.second ||
      MaxRenderSize < newBus_label_offset.second) {
    return;
  }
  bus_label_font_size_ = newBus_label_font_size;
  bus_label_offset_ = {newBus_label_offset.first, newBus_label_offset.second};
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
    const vector<svg::Color> &newColor_palette) {
  color_palette_ = newColor_palette;
}

ostream &operator<<(std::ostream &ostream, const RenderSettings &info) {
  ostream << static_cast<string>(info) << endl;
  return ostream;
}

} // namespace renderer

/*--------------------------- StopStat --------------------------------------*/
StopStat::StopStat(const std::string &name) : name(name) {}

StopStat::StopStat(const std::string &name, const std::vector<string> &buses)
    : name(name), buses(buses.begin(), buses.end()) {}

/*--------------------------- BusStat ---------------------------------------*/
BusStat::BusStat(const std::string &name) : name(/*std::string(name)*/ name) {}

/*--------------------------- BusQuery --------------------------------------*/
BusData::BusData(const string &name) : name(name) {}

/*--------------------------- StopQuery -------------------------------------*/
StopData::StopData(const string &name) : name(name) {}

/*--------------------------- BusInfo ---------------------------------------*/
BusInfo::BusInfo(string_view name) : name(name), is_roundtrip{false} {}

/*--------------------------- StopInfo --------------------------------------*/
StopInfo::StopInfo(string_view name, detail::Coordinates coordinates)
    : name(name), coordinates(coordinates) {}

/*--------------------------- MapStat ---------------------------------------*/
MapStat::MapStat(const string &name) : name(name) {}

std::ostream &operator<<(std::ostream &ostream, const BusStat &value) {
  ostream << "Bus "sv << value.name << ": "sv
          << std::to_string(value.stops_count) << " stops on route, "sv
          << std::to_string(value.unique_stops_count) << " unique stops, "sv
          << std::to_string(value.routelength) << " route length, "sv
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

std::ostream &operator<<(std::ostream &ostream, const StopData &query) {
  ostream << "Stop "sv << query.name << ": "sv
          << std::to_string(query.coordinates.lat) << ", "sv
          << std::to_string(query.coordinates.lng);
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const BusData &query) {
  ostream << "Bus "sv << query.name << ": "sv << query.stops;
  return ostream;
}

} // namespace transport
