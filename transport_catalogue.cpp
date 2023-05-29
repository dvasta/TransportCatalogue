#include "transport_catalogue.h"

#include "geo.h"

#include <iostream>
#include <memory>
#include <algorithm>

namespace catalogue {
using domain::Bus;
using domain::Stop;

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(std::move(stop));
    stops_names_.emplace(stops_.back().name, &stops_.back());
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(std::move(bus));
    buses_names_.emplace(buses_.back().name, &buses_.back());
    for (const Stop* stop : buses_.back().stops) {
        buses_by_stop_[stop].emplace(buses_.back().name);
    }
}

void TransportCatalogue::SetDistance(const Stop* stop_first, const Stop* stop_second, int distance) {
    distance_.emplace(std::make_pair(stop_first, stop_second), distance);
}

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
    return (stops_names_.count(stop_name) > 0)
            ? stops_names_.at(stop_name)
            : nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const {
    return (buses_names_.count(bus_name) > 0)
            ? buses_names_.at(bus_name)
            : nullptr;
}

int TransportCatalogue::GetDistance(const Stop* stop_first, const Stop* stop_second) const {
    auto stop_pair = std::make_pair(stop_first, stop_second);
    if (distance_.count(stop_pair) == 0) {
        if (stop_first == stop_second) {
            return 0;
        }
        stop_pair = std::make_pair(stop_second, stop_first);
    }
    return distance_.at(stop_pair);
}

const domain::BusStat TransportCatalogue::GetRouteInformation(const Bus* bus) const {
    size_t number_stops = bus->stops.size();
    int distance = 0;
    double geo_dist = 0;
    const Stop* from_stop = bus->stops[0];
    std::unordered_set<const Stop*> unique_stops;
    unique_stops.emplace(from_stop);
    for (size_t i = 1; i < number_stops; ++i) {
        const Stop* to_stop = bus->stops[i];
        unique_stops.emplace(to_stop);
        geo_dist += geo::ComputeDistance(from_stop->coordinates, to_stop->coordinates);
        distance += GetDistance(from_stop, to_stop);
        if (!bus->is_roundtrip) {
            distance += GetDistance(to_stop, from_stop);
        }
        from_stop = to_stop;
    }
    if (!bus->is_roundtrip) {
        geo_dist *= 2;
        number_stops += number_stops - 1;
    }
    return {number_stops, unique_stops.size(), distance, distance / geo_dist};
}

const std::set<std::string_view>& TransportCatalogue::GetBusesByStop(const Stop* stop) const {
    static const std::set<std::string_view> null_buses;
    if (buses_by_stop_.count(stop) == 0) {
        return null_buses;
    }
    return buses_by_stop_.at(stop);
}

std::vector<const Stop*> TransportCatalogue::GetAllValidStops() const {
    std::vector<const Stop*> result;
    std::transform(buses_by_stop_.cbegin(), buses_by_stop_.cend(),
                std::inserter(result, result.begin()),
                [](const auto& stop_buses) {
                    return stop_buses.first;
                });
    return result;
}

const std::deque<domain::Stop>& TransportCatalogue::GetAllStops() const {
    return stops_;
}

std::vector<const Bus*> TransportCatalogue::GetAllBuses() const {
    std::vector<const Bus*> result;
    std::transform(buses_names_.cbegin(), buses_names_.cend(),
                std::inserter(result, result.begin()),
                [](const auto& name_bus) {
                    return name_bus.second;
                });
    return result;
}

size_t TransportCatalogue::GetAllStopsSize() const {
    return stops_.size();
}

size_t TransportCatalogue::PairStopHasher::operator()(std::pair<const Stop*, const Stop*> pair_stop) const {
    return stop_hasher(pair_stop.first) + stop_hasher(pair_stop.second) * 37;
}

} //end namespace catalogue
