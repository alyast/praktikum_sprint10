#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator << (std::ostream& out, StrokeLineCap item) {
    if (item == StrokeLineCap::BUTT) {
        out << "butt";
    } else if (item == StrokeLineCap::ROUND) {
        out << "round";
    } else if (item == StrokeLineCap::SQUARE) {
        out << "square";
    }
    return out;
}

std::ostream& operator << (std::ostream& out, StrokeLineJoin item) {
    if (item == StrokeLineJoin::ARCS) {
        out << "arcs";
    } else if (item == StrokeLineJoin::BEVEL) {
        out << "bevel";
    } else if (item == StrokeLineJoin::MITER) {
        out << "miter";
    } else if (item == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip";
    } else if (item == StrokeLineJoin::ROUND) {
        out << "round";
    }
    return out;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
	all_obj_ptrs_.emplace_back(std::move(obj));
}


void Document::Render(std::ostream& out) const {
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
	out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
	RenderContext ctx(out, 2, 0);
	RenderContext ctx_ind(ctx.Indented());
	for(auto& obj_ptr: all_obj_ptrs_){
		(obj_ptr.get())->Render(ctx_ind);
	}
	out << "</svg>";
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
	points.push_back(point);
	return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
	auto& out = context.out;
	out << "<polyline points=\""sv;
	for (size_t i=0; i < points.size(); i++) {
		if (i!=0) {
			out << " "sv;
		}
		out << points[i].x << ","sv << points[i].y;
	}
	out << "\""sv;
	RenderAttrs(out);
	out << "/>"sv;
}

Text& Text::SetPosition (Point pos) {
	pos_ = pos;
	return *this;
}

Text& Text::SetOffset(Point offset) {
	offset_ = offset;
	return *this;
}

Text& Text::SetFontSize(uint32_t size) {
	size_ = size;
	return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_set_ = true;
	font_family_ = font_family;
	return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_set_ = true;
	font_weight_ = font_weight;
	return *this;
}

Text& Text::SetData(std::string data) {
	data_ = data;
	return *this;
}

void Text::RenderObject (const RenderContext& context) const {
	auto& out = context.out;
	out << "<text";
	RenderAttrs(out);
	out << " x=\""sv << pos_.x <<"\" y=\""sv << pos_.y << "\" "sv;
	out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
	out << "font-size=\""sv << size_ << "\""sv;
    if (font_family_set_) {
        out << " font-family=\""sv <<  font_family_ << "\""sv;
    }
    if (font_weight_set_) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
	for (char letter: data_){
		if (letter=='\"') {
			out << "&quot;";
		} else if (letter=='\'') {
			out << "&apos;";
		} else if (letter=='<') {
			out << "&lt;";
		} else if (letter=='>') {
			out << "&gt;";
		} else if (letter=='&') {
			out << "&amp;";
		} else {
			out << letter;
		}
	}
	out << "</text>";
}

}  // namespace svg
