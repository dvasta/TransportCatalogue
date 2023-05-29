#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
    case StrokeLineCap::BUTT:
        out << "butt"s;
        break;
    case StrokeLineCap::ROUND:
        out << "round"s;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"s;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
    case StrokeLineJoin::ARCS:
        out << "arcs"s;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"s;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"s;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"s;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"s;
        break;
    }
    return out;
}

void OstreamColorPrinter::operator()(std::monostate) const {
    out << "none"s;
}

void OstreamColorPrinter::operator()(std::string color) const {
    out << color;
}

void OstreamColorPrinter::operator()(Rgb rgb_color) const {
    out << "rgb("s;
    out << static_cast<int>(rgb_color.red) << ","s;
    out << static_cast<int>(rgb_color.green) << ","s;
    out << static_cast<int>(rgb_color.blue) << ")"s;
}

void OstreamColorPrinter::operator()(Rgba rgba_color) const {
    out << "rgba("s;
    out << static_cast<int>(rgba_color.red) << ","s;
    out << static_cast<int>(rgba_color.green) << ","s;
    out << static_cast<int>(rgba_color.blue) << ","s;
    out << rgba_color.opacity << ")"s;
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
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;

    bool flag = 0;
    for (const Point& point : points_) {
        if (flag) {
            out << " "sv;
        }
        out << point.x << ","sv << point.y;
        flag = 1;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text --------------------

Text& Text::SetPosition(Point pos) {
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
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_= data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << size_ << "\""sv;
    if (font_family_) {
        out << " font-family=\""sv << *font_family_ << "\""sv;
    }
    if (font_weight_) {
        out << " font-weight=\""sv << *font_weight_ << "\""sv;
    }
    out << ">"sv;
    out << data_ << "</text>"sv;
}

// ---------- Document ----------------

void Document::AddPtr(std::unique_ptr<Object>&& ptr) {
    ptr_objects_.emplace_back(std::move(ptr));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (auto& obj : ptr_objects_) {
        obj.get()->Render(RenderContext(out, 2, 2));
    }
    out << "</svg>"sv;
}

}  // namespace svg
