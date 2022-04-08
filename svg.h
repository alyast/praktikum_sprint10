#pragma once

/*
 * Место для вашей svg-библиотеки
 */
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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

std::ostream& operator << (std::ostream& out, StrokeLineCap item);
std::ostream& operator << (std::ostream& out, StrokeLineJoin item);

struct Rgb {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
    Rgb (uint8_t red_c, uint8_t green_c, uint8_t blue_c):
        red(red_c),
        green(green_c),
        blue(blue_c) {};
    Rgb () = default;
};

struct Rgba {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	double opacity;
    Rgba (uint8_t red_c, uint8_t green_c, uint8_t blue_c, double opacity_c):
        red(red_c),
        green(green_c),
        blue(blue_c),
        opacity(opacity_c) {};
    Rgba () = default;
};

//using Color = std::string;
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

struct ColorPrinter {
public:
	std::ostream& out;
	void operator() (std::monostate) const {
		out << "none";
	}

	void operator() (std::string color_name) const {
		out << color_name;
	}

	void operator() (Rgb color_val) const {
		out << "rgb(" + std::to_string(color_val.red) + "," + std::to_string(color_val.green) + "," + std::to_string(color_val.blue) + ")";
	}

	void operator() (Rgba color_val) const {
        //out << std::setprecision(1);
		out << "rgba(" + std::to_string(color_val.red) + "," + std::to_string(color_val.green) + "," + std::to_string(color_val.blue) + "," <<  color_val.opacity << ")";
	}
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
    	stroke_linecap_ = line_cap;
    	return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
    	stroke_linejoin_ = line_join;
    	return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(ColorPrinter{out},*fill_color_);
            out << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{out}, *stroke_color_);
            out << "\""sv;
        }
        if (stroke_width_) {
        	out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_linecap_) {
        	if (*stroke_linecap_ == StrokeLineCap::BUTT) {
        		out << " stroke-linecap=\"butt\""sv;
        	} else if (*stroke_linecap_ == StrokeLineCap::ROUND) {
        		out << " stroke-linecap=\"round\""sv;
        	} else if (*stroke_linecap_ == StrokeLineCap::SQUARE) {
        		out << " stroke-linecap=\"square\""sv;
        	}
        }
        if (stroke_linejoin_) {
        	if (*stroke_linejoin_ == StrokeLineJoin::ARCS) {
        		out << " stroke-linejoin=\"arcs\""sv;
        	} else if (*stroke_linejoin_ == StrokeLineJoin::BEVEL) {
        		out << " stroke-linejoin=\"bevel\""sv;
        	} else if (*stroke_linejoin_ == StrokeLineJoin::MITER) {
        		out << " stroke-linejoin=\"miter\""sv;
        	} else if (*stroke_linejoin_ == StrokeLineJoin::MITER_CLIP) {
        		out << " stroke-linejoin=\"miter-clip\""sv;
        	} else if (*stroke_linejoin_ == StrokeLineJoin::ROUND) {
        		out << " stroke-linejoin=\"round\""sv;
        	}
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};


class Object;

class ObjectContainer {
public:
	template <typename Obj>
	void Add(Obj obj);
	virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::vector<std::unique_ptr<Object>> all_obj_ptrs_;
	~ObjectContainer() = default;
};

class Drawable {
public:
	virtual void Draw (ObjectContainer& container) const = 0;
	virtual ~Drawable() = default;
};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject (const RenderContext& context) const override;
    std::vector<Point> points;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject (const RenderContext& context) const override;
    Point pos_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
    bool font_family_set_ = false;
    bool font_weight_set_ = false;
};

class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document

};

template <typename Obj>
void ObjectContainer::Add(Obj obj) {
    all_obj_ptrs_.emplace_back(std::make_unique<Obj>(std::move(obj)));
}

const Color NoneColor{};
}  // namespace svg
