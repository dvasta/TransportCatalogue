#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <functional>

#include "domain.h"

namespace catalogue {

using domain::Stop;
using domain::Bus;

class TransportCatalogue {
public:
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    void SetDistance(const Stop* stop_first, const Stop* stop_second, int distance);

    const Stop* GetStop(std::string_view stop_name) const;
    const Bus* GetBus(std::string_view bus_name) const;

    int GetDistance(const Stop* stop_first, const Stop* stop_second) const;
    const domain::BusStat GetRouteInformation(const Bus* bus) const;
    const std::set<std::string_view>& GetBusesByStop(const Stop* stop) const;
    std::vector<const Stop*> GetAllValidStops() const;
    const std::deque<domain::Stop>& GetAllStops() const;
    std::vector<const Bus*> GetAllBuses() const;
    size_t GetAllStopsSize() const;

private:
    struct PairStopHasher {
        size_t operator()(std::pair<const Stop*, const Stop*> pair_stop) const;

    private:
        std::hash<const void*> stop_hasher;
    };

    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stops_names_;
    std::unordered_map<std::string_view, const Bus*> buses_names_;
    std::unordered_map<const Stop*, std::set<std::string_view>> buses_by_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopHasher> distance_;
};

} //end namespace catalogue
