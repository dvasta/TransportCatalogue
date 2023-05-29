#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>

#include "geo.h"

namespace domain {

struct Stop {
    explicit Stop(std::string name, geo::Coordinates coordinates);
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    explicit Bus(std::string name, std::vector<const Stop*> stops, bool is);
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
};

struct BusStat {
    size_t number_stops;
    size_t uniq_stops;
    int distance;
    double curvature;
};

struct RouteItem {
    enum class Type {
        WAIT,
        BUS,
    };

    RouteItem(const Type& type, const std::string& stop, double time);
    RouteItem(const Type& type, const std::string& bus, double time, int span_count);
    Type type;
    const std::string& name;
    double time;
    std::optional<int> span_count;
};

struct RouteInfo {
    double total_time;
    std::vector<RouteItem> items;
};

} //end namespace domain
