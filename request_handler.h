#pragma once

#include <optional>
#include <string_view>
#include <unordered_set>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "domain.h"

namespace handler {

class RequestHandler {
public:
    RequestHandler(const catalogue::TransportCatalogue& catalogue,
            const renderer::MapRenderer& renderer,
            const router::TransportRouter& router);

    std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;

    const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

    std::string RenderMap() const;

    std::optional<domain::RouteInfo> GetRoute(std::string_view from, std::string_view to) const;

private:
    const catalogue::TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRouter& router_;
};

} // end namespace
