#pragma once

#include <vector>
#include <algorithm>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace renderer {

using domain::Stop;
using domain::Bus;

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    void SetWidth(double);
    void SetHeight(double);
    void SetPadding(double);
    void SetLineWidth(double);
    void SetStopRadius(double);
    void SetBusLabelFontSize(int);
    void SetBusLabelOffset(double, double);
    void SetStopLabelFontSize(int);
    void SetStopLabelOffset(double, double);
    void SetUnderlayerWidth(double);
    void SetUnderlayerColor(std::string color);
    void SetUnderlayerColor(int r, int g, int b);
    void SetUnderlayerColor(int r, int g, int b, double opacity);

    void SetColorPalette(std::string color);
    void SetColorPalette(int r, int g, int b);
    void SetColorPalette(int r, int g, int b, double opacity);

    void GetMap(std::ostream&, std::vector<const Stop*>, std::vector<const Bus*>) const;

private:
    double width_ = 0.0;                    // ширина в пикселях
    double height_ = 0.0;                   // высота в пикселях
    double padding_ = 0.0;                  // отступ от краев документа
    double line_width_ = 0.0;               // толщина линий маршрутов
    double stop_radius_ = 0.0;              // радиус окружностей остановок
    int bus_label_font_size_ = 0;           // размер текста названия маршрутов
    svg::Point bus_label_offset_;           // смещение надписи названия маршрута
    int stop_label_font_size_ = 0;          // размер текста названия остановок
    svg::Point stop_label_offset_;          // смещение надписи названия остановки
    svg::Color underlayer_color_;           // цвет тени текстов
    double underlayer_width_ = 0.0;         // толщина тени текстов
    std::vector<svg::Color> color_palette_; //цветовая палитра

    SphereProjector CreateProjector(const std::vector<const Stop*>& all_stops) const;
    void AddRoutes(const std::vector<const Bus*>&, const renderer::SphereProjector&, svg::Document&) const;
    void AddStops(const std::vector<const Stop*>&, const renderer::SphereProjector&, svg::Document&) const;
};

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin,
                PointInputIt points_end,
                double max_width, double max_height, double padding)
    : padding_(padding) {
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
              return lhs.lng < rhs.lng;
          });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    const auto [bottom_it, top_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
              return lhs.lat < rhs.lat;
          });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

} // end namespace renderer
