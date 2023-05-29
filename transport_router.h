#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include "transport_catalogue.h"
#include "domain.h"
#include "graph.h"
#include "router.h"

namespace router {

struct RoutingSettings {
    double bus_wait_time;
    double bus_velocity;
};

class TransportRouter {
public:
    TransportRouter(const catalogue::TransportCatalogue& catalogue,
            const RoutingSettings& routing_settings);

    std::optional<domain::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

private:
    struct RouteInfo {
        const std::string& stop_name;
        const std::string& bus_name;
        int span_count;
    };

    using Graph = graph::DirectedWeightedGraph<double>;
    using Edge = graph::Edge<double>;

    const catalogue::TransportCatalogue& catalogue_;
    const RoutingSettings& routing_settings_;
    std::unordered_map<std::string_view, size_t> stop_vertex_id_;
    std::unordered_map<size_t, RouteInfo> edge_id_route_info_;
    Graph graph_;
    graph::Router<double> router_;

    std::vector<domain::RouteItem> CreateRouteItems(const std::vector<size_t>& edge_ids) const;
    double GetTripTimeFromGraph(size_t edge_id) const;
    double ComputeTime(const domain::Stop* from, const domain::Stop* to) const;
    size_t GetGraphVertexId(std::string_view name) const;

    void AddGraphEdge(Graph& graph, Edge edge, RouteInfo route_info);
    void FillGraphWithStops();
    void FillGraphWithRoutes(Graph& graph);
    Graph CreateGraph();
};

} // namespace router
