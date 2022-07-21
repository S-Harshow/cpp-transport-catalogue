
#include "svg.h"
#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

std::vector<std::string_view> SplitStringToVector(std::string_view line,
                                                  char separator) {
  vector<string_view> result{};
  string::size_type pos = 0;
  while (pos < string_view::npos) {
    auto next_pos = line.find(separator, pos);
    if (next_pos != string_view::npos) {
      result.push_back(line.substr(pos, next_pos - pos));
    } else {
      result.push_back(line.substr(pos));
      break;
    }
    pos = next_pos + 1;
  }
  return result;
}

namespace svg {

ostream &operator<<(ostream &ostream, const StrokeLineCap line) {
  switch (line) {
  case StrokeLineCap::BUTT:
    ostream << "butt"sv;
    break;
  case StrokeLineCap::ROUND:
    ostream << "round"sv;
    break;
  case StrokeLineCap::SQUARE:
    ostream << "square"sv;
  }
  return ostream;
}

ostream &operator<<(ostream &ostream, const StrokeLineJoin line) {
  switch (line) {
  case StrokeLineJoin::ARCS:
    ostream << "arcs"sv;
    break;
  case StrokeLineJoin::BEVEL:
    ostream << "bevel"sv;
    break;
  case StrokeLineJoin::MITER:
    ostream << "miter"sv;
    break;
  case StrokeLineJoin::MITER_CLIP:
    ostream << "miter-clip"sv;
    break;
  case StrokeLineJoin::ROUND:
    ostream << "round"sv;
  }
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, Point point) {
  ostream << point.x << "," << point.y;
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const Color &color) {
  visit(ColorPrinter{ostream}, color);
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream,
                         const std::vector<Color> &colors) {
  ostream << "{"sv;
  bool first{true};
  for (const auto &item : colors) {
    if (!first) {
      ostream << ", "sv;
    }
    first = false;
    ostream << item;
  }
  return ostream << "}"sv;
}

istream &operator>>(std::istream &istream, Color &color) {
  string line;
  istream >> line;
  if (line[0] == '[' && line[line.size() - 1] == '[') {
    line.erase(0, 1);
    line.erase(line.size() - 1, 1);
    if (2 == count(line.begin(), line.end(), ',')) {
      color = Rgb(line);
    }
    if (3 == count(line.begin(), line.end(), ',')) {
      color = Rgba(line);
    }
  }
  auto alpha_cout = static_cast<unsigned long>(
      std::count_if(line.begin(), line.end(),
                    [](unsigned char ch_) { return std::isalpha(ch_); }));
  if (line.size() == alpha_cout) {
    color = line;
  }
  return istream;
}

void Object::Render(svg::RenderContext context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
  RenderObject(context);
  context.out << std::endl;
}

// ---------- Circle ------------------

Circle &Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle &Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
  out << "r=\""sv << radius_ << "\""sv;
  // Выводим атрибуты, унаследованные от PathProps
  RenderAttrs(context.out);
  out << "/>"sv;
}

// ---------- Text ------------------
Text &Text::SetPosition(Point pos) {
  pos_ = pos;
  return *this;
}

Text &Text::SetOffset(Point offset) {
  offset_ = offset;
  return *this;
}

Text &Text::SetFontSize(uint32_t size) {
  font_size_ = size;
  return *this;
}

Text &Text::SetFontFamily(std::string font_family) {
  font_family_ = move(font_family);
  return *this;
}

Text &Text::SetFontWeight(std::string font_weight) {
  font_weight_ = move(font_weight);
  return *this;
}

Text &Text::SetData(std::string data) {
  data_ = move(data);
  return *this;
}

void Text::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<text"sv;
  // Выводим атрибуты, унаследованные от PathProps
  RenderAttrs(context.out);
  out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
  out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
  out << "font-size=\""sv << font_size_ << "\""sv;
  if (!font_family_.empty()) {
    out << " font-family=\""sv << font_family_ << "\""sv;
  }
  if (!font_weight_.empty()) {
    out << " font-weight=\""sv << font_weight_ << "\""sv;
  }
  out << ">"sv << TextFormat() << "</text>"sv;
}

string Text::TextFormat() const {
  string result = data_;
  for (size_t pos = 0; pos < result.size(); ++pos) {
    switch (result[pos]) {
    case '&':
      result.insert(pos + 1, "amp;");
      pos += 4;
      break;
    case '\'':
      result.erase(pos, 1);
      result.insert(pos, "&apos;");
      pos += 6;
      break;
    case '"':
      result.erase(pos, 1);
      result.insert(pos, "&quot;");
      pos += 6;
      break;
    case '<':
      result.erase(pos, 1);
      result.insert(pos, "&lt;");
      pos += 4;
      break;
    case '>':
      result.erase(pos, 1);
      result.insert(pos, "&gt;");
      pos += 4;
      break;
    default:
      break;
    }
  }
  return result;
}

// ---------- Polyline------------------
Polyline &Polyline::AddPoint(Point point) {
  points_.push_back(point);
  return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<polyline points=\""sv;
  for (auto iter = points_.begin(); iter != points_.end(); ++iter) {
    if (iter != points_.begin()) {
      out << " ";
    }
    out << iter->x << "," << iter->y;
  }
  out << "\""sv;
  // Выводим атрибуты, унаследованные от PathProps
  RenderAttrs(context.out);
  out << "/>"sv;
}

// ---------- Document------------------
void Document::AddPtr(std::unique_ptr<Object> &&obj) {
  if (obj == nullptr) {
    return;
  }
  objects_.push_back(move(obj));
}

void Document::Render(std::ostream &out) const {
  RenderContext ctx(out, 2, 2);
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << endl;
  out.unsetf(ios::fixed);
  for (const auto &object : objects_) {
    object->Render(ctx);
  }
  out << "</svg>"sv;
  out.flush();
}

// ---------- Rgb------------------
Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue)
    : red(red), green(green), blue(blue) {}

Rgb::Rgb(std::string_view color) {
  // Цвет 255, 16, 12 из трёх целых чисел диапазона [0, 255]
  if (color.empty()) {
    return;
  }
  vector<string_view> rgb_vector = SplitStringToVector(color, ',');
  if (3 != rgb_vector.size()) {
    return;
  }
  Rgb tmp;
  try {
    tmp.red = static_cast<uint8_t>(std::stoi(string(rgb_vector.at(0))));
    tmp.green = static_cast<uint8_t>(std::stoi(string(rgb_vector.at(1))));
    tmp.blue = static_cast<uint8_t>(std::stoi(string(rgb_vector.at(2))));

  } catch (...) {
    return;
  }

  swap(*this, tmp);
}

// ---------- Rgba------------------
Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
    : red(red), green(green), blue(blue), opacity(opacity) {}

Rgba::Rgba(std::string_view color) {
  // Цвет 255, 16, 12
  if (color.empty()) {
    return;
  }
  Rgba tmp;
  vector<string_view> rgba_vector = SplitStringToVector(color, ',');
  if (4 != rgba_vector.size()) {
    return;
  }
  try {
    tmp.red = static_cast<uint8_t>(std::stoi(string(rgba_vector.at(0))));
    tmp.green = static_cast<uint8_t>(std::stoi(string(rgba_vector.at(1))));
    tmp.blue = static_cast<uint8_t>(std::stoi(string(rgba_vector.at(2))));
    tmp.opacity = std::stod(string(rgba_vector.at(3)));
  } catch (...) {
    return;
  }
  swap(*this, tmp);
}
// ---------- ColorPrinter------------------
void ColorPrinter::operator()(monostate /*noused*/) const { out << "none"sv; }

void ColorPrinter::operator()(const std::string &color) const { out << color; }

void ColorPrinter::operator()(Rgb rgb) const {
  out << "rgb("sv << to_string(rgb.red) << ","sv << to_string(rgb.green)
      << ","sv << to_string(rgb.blue) << ")"sv;
}

void ColorPrinter::operator()(Rgba rgba) const {
  out << "rgba("sv << to_string(rgba.red) << ","sv << to_string(rgba.green)
      << ","sv << to_string(rgba.blue) << ","sv << rgba.opacity << ")"sv;
}

RenderContext::RenderContext(std::ostream &out) : out(out) {}

RenderContext::RenderContext(std::ostream &out, int indent_step, int indent)
    : out(out), indent_step(indent_step), indent(indent) {}

RenderContext RenderContext::Indented() const {
  return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
  for (int i = 0; i < indent; ++i) {
    out.put(' ');
  }
}

} // namespace svg
