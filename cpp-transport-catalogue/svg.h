#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace svg {

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};
std::ostream &operator<<(std::ostream &out, StrokeLineCap line);

std::ostream &operator<<(std::ostream &out, StrokeLineJoin line);

struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double x = 0;
  double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с
 * отступами. Хранит ссылку на поток вывода, текущее значение и шаг отступа
 * при выводе элемента
 */
struct RenderContext {
  explicit RenderContext(std::ostream &out);
  RenderContext(std::ostream &out, int indent_step, int indent = 0);
  [[nodiscard]] RenderContext Indented() const;
  void RenderIndent() const;
  std::ostream &out;
  int indent_step = 0;
  int indent = 0;
};

struct Rgb {
  Rgb() = default;
  Rgb(uint8_t red, uint8_t green, uint8_t blue);
  explicit Rgb(std::string_view color);
  uint8_t red{};
  uint8_t green{};
  uint8_t blue{};
};

struct Rgba {
  Rgba() = default;
  Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);
  explicit Rgba(std::string_view color);
  uint8_t red{};
  uint8_t green{};
  uint8_t blue{};
  double opacity{1.};
};

struct ColorPrinter {
  std::ostream &out;
  void operator()(std::monostate) const;
  void operator()(const std::string &color) const;
  void operator()(svg::Rgb rgb) const;
  void operator()(svg::Rgba rgba) const;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
inline const Color NoneColor{std::monostate{}};
std::ostream &operator<<(std::ostream &out, const Color &color);
std::istream &operator>>(std::istream &in_, Color &color);

template <typename Owner> class PathProps {
public:
  // устанавливает значение свойства fill
  Owner &SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
  }
  // устанавливает значение свойства stroke
  Owner &SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
  }

  // устанавливает значение свойства stroke-width
  Owner &SetStrokeWidth(double width) {
    width_ = width;
    return AsOwner();
  }

  // устанавливает значение свойства stroke-linecap
  Owner &SetStrokeLineCap(StrokeLineCap line_cap) {
    line_cap_ = line_cap;
    return AsOwner();
  }

  // устанавливает значение свойства stroke-linejoin
  Owner &SetStrokeLineJoin(StrokeLineJoin line_join) {
    line_join_ = line_join;
    return AsOwner();
  }

protected:
  ~PathProps() = default;

  void RenderAttrs(std::ostream &out) const {
    using namespace std::literals;

    if (fill_color_) {
      out << " fill=\""sv << *fill_color_ << "\""sv;
    }
    if (stroke_color_) {
      out << " stroke=\""sv << *stroke_color_ << "\""sv;
    }
    if (width_) {
      out << " stroke-width=\""sv << *width_ << "\""sv;
    }
    if (line_cap_) {
      out << " stroke-linecap=\""sv << (*line_cap_) << "\""sv;
    }
    if (line_join_) {
      out << " stroke-linejoin=\""sv << (*line_join_) << "\""sv;
    }
  }

private:
  Owner &AsOwner() {
    // static_cast безопасно преобразует *this к Owner&,
    // если класс Owner — наследник PathProps
    return static_cast<Owner &>(*this);
  }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> width_;
  std::optional<StrokeLineCap> line_cap_;
  std::optional<StrokeLineJoin> line_join_;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
  void Render(svg::RenderContext context) const;
  virtual ~Object() = default;

private:
  virtual void RenderObject(const RenderContext &context) const = 0;
};
/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
  Circle &SetCenter(Point center);
  Circle &SetRadius(double radius);

private:
  void RenderObject(const RenderContext &context) const override;
  Point center_{};
  double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
  // Добавляет очередную вершину к ломаной линии
  Polyline &AddPoint(Point point);

private:
  void RenderObject(const RenderContext &context) const override;

  std::deque<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
  // Задаёт координаты опорной точки (атрибуты x и y)
  Text &SetPosition(Point pos);

  // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
  Text &SetOffset(Point offset);

  // Задаёт размеры шрифта (атрибут font-size)
  Text &SetFontSize(uint32_t size);

  // Задаёт название шрифта (атрибут font-family)
  Text &SetFontFamily(std::string font_family);

  // Задаёт толщину шрифта (атрибут font-weight)
  Text &SetFontWeight(std::string font_weight);

  // Задаёт текстовое содержимое объекта (отображается внутри тега text)
  Text &SetData(std::string data);

private:
  void RenderObject(const RenderContext &context) const override;
  [[nodiscard]] std::string TextFormat() const;

  Point pos_{};
  Point offset_{};
  uint32_t font_size_ = 1;
  std::string font_weight_{};
  std::string font_family_{};
  std::string data_{};
};

class ObjectContainer {
public:
  template <typename Obj> void Add(Obj obj) {
    AddPtr(std::make_unique<Obj>(std::move(obj)));
  }
  virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;
  virtual ~ObjectContainer() = default;
};

class Drawable {
public:
  virtual void Draw(ObjectContainer &container) const = 0;
  virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
  /*
   Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
   Пример использования:
   Document doc;
   doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
  */
  template <typename Obj> void Add(Obj obj) {
    AddPtr(std::make_unique<Obj>(std::move(obj)));
  }
  // Добавляет в svg-документ объект-наследник svg::Object
  void AddPtr(std::unique_ptr<Object> &&obj) override;

  // Выводит в ostream svg-представление документа
  void Render(std::ostream &out) const;

private:
  std::deque<std::unique_ptr<Object>> objects_{};
};

} // namespace svg
