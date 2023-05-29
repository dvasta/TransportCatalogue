#include <iostream>
#include <fstream>
#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std::literals;

int main() {
    using namespace catalogue;
    json_reader::JsonReader json_reader(std::cin, std::cout);

    TransportCatalogue catalogue;
    json_reader.FillCatalogue(catalogue);

    renderer::MapRenderer renderer;
    json_reader.FillRenderer(renderer);

    router::RoutingSettings routing_settings;
    json_reader.FillRoutingSettings(routing_settings);

    router::TransportRouter router(catalogue, routing_settings);
    handler::RequestHandler handler(catalogue, renderer, router);
    json_reader.ProcessRequests(handler);
}
