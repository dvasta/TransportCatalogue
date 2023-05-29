#include "request_handler.h"

#include <deque>
#include <sstream>

namespace handler {

using namespace std::literals;

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& catalogue,
        const renderer::MapRenderer& renderer,
        const router::TransportRouter& router)
    : catalogue_(catalogue)
    , renderer_(renderer)
    , router_(router) {
    }

std::optional<domain::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    if (const domain::Bus* bus = catalogue_.GetBus(bus_name); bus) {
        return catalogue_.GetRouteInformation(bus);
    }
    return std::nullopt;
}

const std::set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    if (const domain::Stop* stop = catalogue_.GetStop(stop_name); stop) {
        return &catalogue_.GetBusesByStop(stop);
    }
    return nullptr;
}

std::string RequestHandler::RenderMap() const {
    std::ostringstream out;
    renderer_.GetMap(out, catalogue_.GetAllValidStops(), catalogue_.GetAllBuses());
    return out.str();
}

std::optional<domain::RouteInfo> RequestHandler::GetRoute(std::string_view from, std::string_view to) const {
    return router_.BuildRoute(from, to);
}

}
