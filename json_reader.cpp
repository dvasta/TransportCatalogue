#include "json_reader.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <string_view>
#include <iomanip>
#include <cstdlib>
#include <memory>

#include "geo.h"
#include "domain.h"
#include "json_builder.h"

namespace json_reader {

using namespace std::literals;
using namespace json;
using domain::Stop;
using domain::Bus;

JsonReader::JsonReader(std::istream& in, std::ostream& out)
    : document_(json::Load(in)), out_(out) {
}

void ParseStop(const Dict& stop_info, catalogue::TransportCatalogue& catalogue) {
    const std::string& name = stop_info.at("name"s).AsString();
    const double& lat = stop_info.at("latitude"s).AsDouble();
    const double& lng = stop_info.at("longitude"s).AsDouble();
    catalogue.AddStop(Stop(name, geo::Coordinates{lat, lng}));
}

void SetDistance(const Node* stop_info_node, catalogue::TransportCatalogue& catalogue) {
    if (stop_info_node->IsNull()) {
        return;
    }
    const Dict& stop_info = stop_info_node->AsDict();
    if (stop_info.count("road_distances"s) == 0) {
        return;
    }
    const std::string& name = stop_info.at("name"s).AsString();
    const Stop* first_stop = catalogue.GetStop(name);
    for (const auto& [stop, dist_node] : stop_info.at("road_distances"s).AsDict()) {
        const Stop* second_stop = catalogue.GetStop(stop);
        int distance = dist_node.AsDouble();
        catalogue.SetDistance(first_stop, second_stop, distance);
    }
}

void ParseBus(const Node* bus_info_node, catalogue::TransportCatalogue& catalogue) {
    if (bus_info_node->IsNull()) {
        return;
    }
    const Dict& bus_info = bus_info_node->AsDict();
    const std::string& name = bus_info.at("name"s).AsString();
    const bool is_round = bus_info.at("is_roundtrip"s).AsBool();
    std::vector<const Stop*> stops;
    for (const Node& stop_node : bus_info.at("stops"s).AsArray()) {
        const std::string& stop = stop_node.AsString();
        stops.push_back(catalogue.GetStop(stop));
    }
    catalogue.AddBus(Bus(name, stops, is_round));
}

void JsonReader::FillCatalogue(catalogue::TransportCatalogue& catalogue) const {
    const Array& base = document_.GetRoot().AsDict().at("base_requests"s).AsArray();
    std::vector<const Node*> stops_in;
    std::vector<const Node*> buses_in;
    for (const Node& elem_node : base) {
        const Dict& elem = elem_node.AsDict();
        if (elem.at("type"s) == "Stop"s) {
            ParseStop(elem, catalogue);
            stops_in.push_back(&elem_node);
        }
        if (elem.at("type"s) == "Bus"s) {
            buses_in.push_back(&elem_node);
        }
    }
    for (const Node* stop_dists : stops_in) {
        SetDistance(stop_dists, catalogue);
    }
    for (const Node* bus_in : buses_in) {
        ParseBus(bus_in, catalogue);
    }
}

void SetUnderlayerColor(renderer::MapRenderer& renderer, const Node& color_node) {
    if (color_node.IsString()) {
        renderer.SetUnderlayerColor(color_node.AsString());
    } else if (const Array& color_array = color_node.AsArray(); color_array.size() == 3) {
        renderer.SetUnderlayerColor(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
    } else {
        renderer.SetUnderlayerColor(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
    }
}

void SetColorPallete(renderer::MapRenderer& renderer, const Node& color_node) {
    if (color_node.IsString()) {
        renderer.SetColorPalette(color_node.AsString());
    } else if (const Array& color_array = color_node.AsArray(); color_array.size() == 3) {
        renderer.SetColorPalette(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
    } else {
        renderer.SetColorPalette(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
    }
}

void JsonReader::FillRenderer(renderer::MapRenderer& renderer) const {
    const Dict& settings = document_.GetRoot().AsDict().at("render_settings"s).AsDict();
    renderer.SetWidth(settings.at("width"s).AsDouble());
    renderer.SetHeight(settings.at("height"s).AsDouble());
    renderer.SetPadding(settings.at("padding"s).AsDouble());
    renderer.SetStopRadius(settings.at("stop_radius"s).AsDouble());
    renderer.SetLineWidth(settings.at("line_width"s).AsDouble());
    renderer.SetBusLabelFontSize(settings.at("bus_label_font_size"s).AsInt());
    renderer.SetStopLabelFontSize(settings.at("stop_label_font_size"s).AsInt());
    renderer.SetUnderlayerWidth(settings.at("underlayer_width"s).AsDouble());

    SetUnderlayerColor(renderer, settings.at("underlayer_color"s));

    const Array& bus_label_offset = settings.at("bus_label_offset"s).AsArray();
    renderer.SetBusLabelOffset(bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble());

    const Array& stop_label_offset = settings.at("stop_label_offset"s).AsArray();
    renderer.SetStopLabelOffset(stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble());

    for (const Node& color_node : settings.at("color_palette"s).AsArray()) {
        SetColorPallete(renderer, color_node);
    }
}

void JsonReader::FillRoutingSettings(router::RoutingSettings& routing_settings) const {
    const Dict& settings = document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    routing_settings.bus_wait_time = settings.at("bus_wait_time"s).AsDouble();
    routing_settings.bus_velocity = settings.at("bus_velocity"s).AsDouble() * 1000 / 60;
}

inline const std::string id_key{"request_id"};

json::Dict BusRequests(const handler::RequestHandler& handler, const Dict& request) {
    const int id_value = request.at("id"s).AsInt();
    json::Builder result{};
    result.StartDict()
            .Key(id_key).Value(id_value);
    const std::string& name = request.at("name"s).AsString();
    if (const auto& bus_stat_opt = handler.GetBusStat(name); bus_stat_opt) {
        result
            .Key("curvature"s).Value(bus_stat_opt->curvature)
            .Key("route_length"s).Value(bus_stat_opt->distance)
            .Key("stop_count"s).Value(static_cast<int>(bus_stat_opt->number_stops))
            .Key("unique_stop_count"s).Value(static_cast<int>(bus_stat_opt->uniq_stops));
    } else {
        result.Key("error_message"s).Value("not found"s);
    }
    result.EndDict();
    return result.Build().AsDict();
}

json::Dict StopRequests(const handler::RequestHandler& handler, const Dict& request) {
    const int id_value = request.at("id"s).AsInt();
    json::Builder result{};
    result.StartDict()
            .Key(id_key).Value(id_value);
    if (const auto& buses_names =
            handler.GetBusesByStop(request.at("name"s).AsString()); buses_names) {
        result.Key("buses"s).StartArray();
        for (std::string_view bus_name : *buses_names) {
            result.Value(std::string(bus_name));
        }
        result.EndArray();
    } else {
        result.Key("error_message"s).Value("not found"s);
    }
    result.EndDict();
    return result.Build().AsDict();
}

json::Dict MapRequests(const handler::RequestHandler& handler, const Dict& request) {
    const int id_value = request.at("id"s).AsInt();
    json::Builder result{};
    result.StartDict()
            .Key(id_key).Value(id_value)
            .Key("map"s).Value(handler.RenderMap())
            .EndDict();
    return result.Build().AsDict();
}

json::Dict RouteRequests(const handler::RequestHandler& handler, const Dict& request) {
    const int id_value = request.at("id"s).AsInt();
    json::Builder result{};
    result.StartDict()
            .Key(id_key).Value(id_value);
    const auto& items_opt = handler.GetRoute(
            request.at("from"s).AsString(), request.at("to"s).AsString());
    if (items_opt) {
        result
            .Key("total_time"s).Value(items_opt->total_time)
            .Key("items"s).StartArray();
        for (const auto& item : items_opt->items) {
            if (item.type == domain::RouteItem::Type::WAIT) {
                result.StartDict()
                    .Key("type"s).Value("Wait"s)
                    .Key("stop_name"s).Value(item.name)
                    .Key("time"s).Value(item.time)
                    .EndDict();
            } else if (item.type == domain::RouteItem::Type::BUS) {
                result.StartDict()
                    .Key("type"s).Value("Bus"s)
                    .Key("bus"s).Value(item.name)
                    .Key("time"s).Value(item.time)
                    .Key("span_count"s).Value(*item.span_count)
                    .EndDict();
            }
        }
        result.EndArray();
    } else {
        result.Key("error_message"s).Value("not found"s);
    }
    result.EndDict();
    return result.Build().AsDict();
}

void JsonReader::ProcessRequests(const handler::RequestHandler& handler) {
    json::Builder result{};
    result.StartArray();
    const json::Array& stat = document_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    for (const json::Node& elem_node : stat) {
        const json::Dict& request = elem_node.AsDict();
        if (request.at("type"s) == "Bus"s) {
            result.Value(BusRequests(handler, request));
        } else if (request.at("type"s) == "Stop"s) {
            result.Value(StopRequests(handler, request));
        } else if (request.at("type"s) == "Map"s) {
            result.Value(MapRequests(handler, request));
        } else if (request.at("type"s) == "Route"s) {
            result.Value(RouteRequests(handler, request));
        }
    }
    result.EndArray();
    Print(Document{result.Build()}, out_);
}

} //end namespace catalogue::input
