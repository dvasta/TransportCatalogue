#include "map_renderer.h"

#include <algorithm>
#include <deque>

namespace renderer {
using namespace std::literals;

void MapRenderer::SetWidth(double width) {
    width_ = width;
}

void MapRenderer::SetHeight(double height) {
    height_ = height;
}

void MapRenderer::SetPadding(double padding) {
    padding_ = padding;
}

void MapRenderer::SetLineWidth(double line_width) {
    line_width_ = line_width;
}

void MapRenderer::SetStopRadius(double stop_radius) {
    stop_radius_ = stop_radius;
}

void MapRenderer::SetBusLabelFontSize(int bus_label_font_size) {
    bus_label_font_size_ = bus_label_font_size;
}

void MapRenderer::SetBusLabelOffset(double x, double y) {
    bus_label_offset_ = svg::Point(x, y);
}

void MapRenderer::SetStopLabelFontSize(int stop_label_font_size) {
    stop_label_font_size_ = stop_label_font_size;
}

void MapRenderer::SetStopLabelOffset(double x, double y) {
    stop_label_offset_ = svg::Point(x, y);
}

void MapRenderer::SetUnderlayerColor(std::string color) {
    underlayer_color_ = std::move(color);
}

void MapRenderer::SetUnderlayerColor(int r, int g, int b) {
    underlayer_color_ = svg::Rgb(r, g, b);
}

void MapRenderer::SetUnderlayerColor(int r, int g, int b, double opacity) {
    underlayer_color_ = svg::Rgba(r, g, b, opacity);
}

void MapRenderer::SetUnderlayerWidth(double underlayer_width) {
    underlayer_width_ = underlayer_width;
}

void MapRenderer::SetColorPalette(std::string color) {
    color_palette_.push_back(std::move(color));
}

void MapRenderer::SetColorPalette(int r, int g, int b) {
    color_palette_.push_back(svg::Rgb(r, g, b));
}

void MapRenderer::SetColorPalette(int r, int g, int b, double opacity) {
    color_palette_.push_back(svg::Rgba(r, g, b, opacity));
}

renderer::SphereProjector MapRenderer::CreateProjector(const std::vector<const domain::Stop*>& all_stops) const {
    std::vector<geo::Coordinates> all_coordinates;
    std::transform(all_stops.begin(), all_stops.end(),
                std::inserter(all_coordinates, all_coordinates.begin()),
                [](const domain::Stop* stop) {
                    return stop->coordinates;
                });
    return SphereProjector(all_coordinates.begin(), all_coordinates.end(),
            width_, height_, padding_);
}

void MapRenderer::AddRoutes(const std::vector<const domain::Bus*>& all_buses,
        const renderer::SphereProjector& projector,
        svg::Document& doc) const {
    size_t color_id = 0;

    svg::Polyline base_polyline;
    base_polyline.SetStrokeWidth(line_width_)
            .SetFillColor(svg::NoneColor)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    svg::Text base_text;
    base_text.SetOffset(bus_label_offset_)
            .SetFontSize(bus_label_font_size_)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s);

    std::vector<svg::Polyline> polylines;
    std::vector<svg::Text> texts;

    for (const domain::Bus* bus : all_buses) {
        if (bus->stops.size() == 0) {
            continue;
        }

        const svg::Color color = color_palette_[color_id++];
        if (color_id >= color_palette_.size()) {
            color_id = 0;
        }

        svg::Polyline polyline{base_polyline};
        polyline.SetStrokeColor(color);

        std::deque<svg::Point> points;
        for (const auto& stop : bus->stops) {
            points.push_front(projector(stop->coordinates));
            polyline.AddPoint(points.front());
        }
        svg::Point end_point = points.front();
        points.pop_front();
        if (!bus->is_roundtrip) {
            for (const auto& point : points) {
                polyline.AddPoint(point);
            }
        }
        polylines.push_back(polyline);

        svg::Text text{base_text};
        text.SetFillColor(color)
                .SetPosition(points.back())
                .SetData(bus->name);
        svg::Text text_underlayer{text};
        text_underlayer.SetFillColor(underlayer_color_)
                .SetStrokeColor(underlayer_color_)
                .SetStrokeWidth(underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        texts.push_back(text_underlayer);
        texts.push_back(text);

        if (bus->is_roundtrip || (bus->stops.back() == bus->stops.front())) {
            continue;
        }

        text_underlayer.SetPosition(end_point);
        texts.push_back(text_underlayer);
        text.SetPosition(end_point);
        texts.push_back(text);
    }

    for (const auto& polyline: polylines) {
        doc.Add(polyline);
    }
    for (const auto& text: texts) {
        doc.Add(text);
    }
}

void MapRenderer::AddStops(const std::vector<const domain::Stop*>& all_stops,
        const renderer::SphereProjector& projector,
        svg::Document& doc) const {
    svg::Circle base_circle;
    base_circle.SetFillColor("white"s)
            .SetRadius(stop_radius_);
    svg::Text base_text;
    base_text.SetOffset(stop_label_offset_)
            .SetFontSize(stop_label_font_size_)
            .SetFontFamily("Verdana"s);

    std::vector<svg::Circle> circles;
    std::vector<svg::Text> texts;
    for (const domain::Stop* stop : all_stops) {
        svg::Point point = projector(stop->coordinates);

        svg::Circle circle{base_circle};
        circle.SetCenter(point);
        circles.push_back(circle);

        svg::Text text{base_text};
        text.SetFillColor("black"s)
                .SetPosition(point)
                .SetData(stop->name);
        svg::Text text_underlayer{text};
        text_underlayer.SetFillColor(underlayer_color_)
                .SetStrokeColor(underlayer_color_)
                .SetStrokeWidth(underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        texts.push_back(text_underlayer);
        texts.push_back(text);
    }

    for (const auto& circle: circles) {
        doc.Add(circle);
    }
    for (const auto& text: texts) {
        doc.Add(text);
    }
}

void MapRenderer::GetMap(std::ostream& out,
        std::vector<const domain::Stop*> all_stops,
        std::vector<const domain::Bus*> all_buses) const {
    const renderer::SphereProjector projector = CreateProjector(all_stops);

    svg::Document doc;

    std::sort(all_buses.begin(), all_buses.end(),
            [](const domain::Bus* left, const domain::Bus* rigth) {
        return left->name < rigth->name;
    });
    AddRoutes(all_buses, projector, doc);

    std::sort(all_stops.begin(), all_stops.end(),
            [](const domain::Stop* left, const domain::Stop* rigth) {
        return left->name < rigth->name;
    });
    AddStops(all_stops, projector, doc);

    doc.Render(out);
}

/* ------------- SphereProjector ---------------- */

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

} // end namespace renderer
