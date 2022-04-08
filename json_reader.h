#pragma once
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace json_reader {

	struct BusInput {
		std::string name;
		std::vector<std::string> stops;
		bool roundtrip;
	};

	struct StopInput {
		std::string name;
		double latitude;
		double longitude;
		std::map<std::string, size_t> distances;
	};

	struct StatRequest {
		size_t id;
		std::string type;
		std::string name;
	};

	void ReadAllRequests(std::istream& is, TransportCatalogue& tc);
	void ReadInputRequests(json::Node& root, TransportCatalogue& tc);
	void ReadStatRequests(json::Node& root, TransportCatalogue& tc, renderer::MapRenderer& mr);
	void ReadRenderSettings(json::Node& root, renderer::MapRenderer& mr);
	void AddDataToTransportCatalogue(std::vector<StopInput>& stops_info, std::vector<BusInput>& buses_info, TransportCatalogue& tc);
	void ProcessStatRequests(std::vector<StatRequest>& stat_requests_info, TransportCatalogue& tc, renderer::MapRenderer& mr);
	svg::Color GetColor(json::Node& color_node);
	std::string RenderTransportCatalogue(TransportCatalogue& tc, renderer::MapRenderer& mr);
}
