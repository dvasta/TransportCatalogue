#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"
#include "json.h"

namespace json_reader {

class JsonReader {
public:
    JsonReader(std::istream& in, std::ostream& out);

    void FillCatalogue(catalogue::TransportCatalogue& catalogue) const;

    void FillRenderer(renderer::MapRenderer& renderer) const;

    void FillRoutingSettings(router::RoutingSettings& routing_settings) const;

    void ProcessRequests(const handler::RequestHandler& handler);

private:
    const json::Document document_;
    std::ostream& out_;
};

} //end namespace catalogue::input
