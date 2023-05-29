#include "transport_router.h"

#include <iostream>


namespace router {

using domain::Bus;
using domain::Stop;
using Graph = graph::DirectedWeightedGraph<double>;
using Edge = graph::Edge<double>;
using namespace std::literals;

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue,
        const RoutingSettings& routing_settings)
    : catalogue_(catalogue)
    , routing_settings_(routing_settings)
    , stop_vertex_id_()
    , edge_id_route_info_()
    , graph_(CreateGraph())
    , router_(graph_) {
}

double TransportRouter::GetTripTimeFromGraph(size_t edge_id) const {
    return graph_.GetEdge(edge_id).weight - routing_settings_.bus_wait_time;
}

std::vector<domain::RouteItem> TransportRouter::CreateRouteItems(const std::vector<size_t>& edge_ids) const {
    std::vector<domain::RouteItem> route_items;
    for (size_t edge_id : edge_ids) {
        const RouteInfo& route_info = edge_id_route_info_.at(edge_id);
        route_items.emplace_back(
                domain::RouteItem::Type::WAIT,
                route_info.stop_name,
                routing_settings_.bus_wait_time);
        route_items.emplace_back(
                domain::RouteItem::Type::BUS,
                route_info.bus_name,
                GetTripTimeFromGraph(edge_id),
                route_info.span_count);
    }
    return route_items;
}
 
std::optional<domain::RouteInfo> TransportRouter::BuildRoute(
        std::string_view from, std::string_view to) const {
    const auto& route_opt =
            router_.BuildRoute(stop_vertex_id_.at(from), stop_vertex_id_.at(to));
    if (!route_opt) {
        return std::nullopt;
    }
    return domain::RouteInfo{route_opt->weight, CreateRouteItems(route_opt->edges)};
}
 

void TransportRouter::FillGraphWithStops() {
    size_t count = 0;
    for (const Stop& stop : catalogue_.GetAllStops()) {
        stop_vertex_id_.emplace(stop.name, count++);
    }
}

size_t TransportRouter::GetGraphVertexId (std::string_view name) const {
    return stop_vertex_id_.at(name);
}

double TransportRouter::ComputeTime(const Stop* from, const Stop* to) const {
    return catalogue_.GetDistance(from, to) / routing_settings_.bus_velocity;
}

void TransportRouter::AddGraphEdge(Graph& graph, Edge edge, RouteInfo route_info) {
    size_t edge_id = graph.AddEdge(std::move(edge));
    edge_id_route_info_.emplace(edge_id, std::move(route_info));
}

void TransportRouter::FillGraphWithRoutes(Graph& graph) {
    for (const Bus* bus : catalogue_.GetAllBuses()) {
        const bool is_roundtrip = bus->is_roundtrip;
        const auto& stops = bus->stops;
        for (size_t i = 0; i + 1 < stops.size(); ++i) {
            const size_t vertex_from = GetGraphVertexId(stops[i]->name);
            double time = routing_settings_.bus_wait_time;
            const Stop* stop_from = stops[i];
            std::optional<double> back_time;
            if (!is_roundtrip) {
                back_time = time;
            }
            int span_count = 0;
            for (size_t j = i + 1; j < stops.size(); ++j) {
                const Stop* stop_to = stops[j];
                time += ComputeTime(stop_from, stop_to);
                if (!is_roundtrip) {
                    *back_time += ComputeTime(stop_to, stop_from);
                }
                stop_from = stop_to;
                ++span_count;
                if (stops[i] == stop_to) {
                    continue;
                }
                const size_t vertex_to = GetGraphVertexId(stop_to->name);
                AddGraphEdge(graph,
                        Edge{vertex_from, vertex_to, time},
                        RouteInfo{stops[i]->name, bus->name, span_count});
                if (!is_roundtrip) {
                    AddGraphEdge(graph,
                            Edge{vertex_to, vertex_from, *back_time},
                            RouteInfo{stops[j]->name, bus->name, span_count});
                }
            }
        }
    }
}

Graph TransportRouter::CreateGraph() {
    Graph graph(catalogue_.GetAllStopsSize());
    FillGraphWithStops();
    FillGraphWithRoutes(graph);
    return graph;
}


} // namespace router
