#include "request_handler.h"
#include <map>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
using namespace std;


RequestHandler::RequestHandler (const TransportCatalogue& db, renderer::MapRenderer& renderer) : db_(db), renderer_(renderer)  {}

std::optional<const RouteStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	const Route* route_ptr = db_.GetRoute(bus_name);
	if (route_ptr == nullptr) {
		return {};
	} else {
		const RouteStat route_statistics = db_.GetRouteStat(route_ptr);
		return route_statistics;
	}
}

std::optional<const StopStat> RequestHandler::GetStopStat(const std::string_view& stop_name) const {
	const Stop* stop_ptr = db_.GetStop(stop_name);
	if (stop_ptr == nullptr){
		return {};
	} else {
		const StopStat stop_statistics = db_.GetStopStat(stop_ptr);
		return stop_statistics;
	}
}

svg::Document RequestHandler::RenderMap() {
	//svg::Document placeholder = svg::Document();   //if not comment this string then document will be constant and all objects inside it also will be constant what makes impossible to make unique pointers and very bad error
	return svg::Document();
}
