#include "domain.h"

namespace domain {

using namespace std::literals;

Stop::Stop(std::string name, geo::Coordinates coordinates)
    : name(name), coordinates(coordinates) {
}

Bus::Bus(std::string name, std::vector<const Stop*> stops, bool is)
    : name(name)
    , stops(stops)
    , is_roundtrip(is) {
}

RouteItem::RouteItem(const Type& type, const std::string& stop, double time)
    : type(type)
    , name(stop)
    , time(time) {
}

RouteItem::RouteItem(const Type& type, const std::string& bus, double time, int span_count)
    : type(type)
    , name(bus)
    , time(time)
    , span_count(span_count) {
}

} //end namespace domain
