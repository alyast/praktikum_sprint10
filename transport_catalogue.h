// transport dictionary class
//
#pragma once
#include <cmath>
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "geo.h"
#include "domain.h"

/*
struct RenderData{
	std::unordered_map<std::string, const Route*>* routes_ptr;
	std::unordered_map<std::string, const Stop*>* stops_ptr;
	std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, PairStopsHash>* between_stops_dist_ptr;
};
*/

class PairStopsHash {
public:
	size_t operator() (const std::pair<const Stop*, const Stop*>& stops_pair) const;
};

class TransportCatalogue {
private:
	std::unordered_map<std::string, const Route*> routes;
	std::unordered_map<std::string, const Stop*> stops;
	std::unordered_map<const Stop*, std::set<std::string>> stop_to_buses;
	std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, PairStopsHash> stop_to_stop_dist;
	std::deque<Route> routes_deq;
	std::deque<Stop> stops_deq;
public:
	void AddRoute(Route& route);
	void AddStop(Stop& stop);
	const Route* GetRoute(std::string_view bus) const;
	const Stop* GetStop(std::string_view stop) const;
	RouteStat GetRouteStat(const Route* route) const;
	const StopStat GetStopStat(const Stop* stop) const;
	void AddBusRouteToStop(std::string bus_name, const Stop* stop_ptr);
	void SetStopsDistance(const Stop* this_stop, const Stop* next_stop, size_t distance);
	size_t GetStopsDistance(const Stop* this_stop, const Stop* next_stop) const;
	void PrintEverything();
	std::unordered_map<std::string, const Route*>& GetAllRoutes();
	std::unordered_map<std::string, const Stop*>& GetAllStops();
};


namespace detail {
	std::string_view Ltrim(const std::string_view s);
	std::string_view Rtrim(const std::string_view s);

	bool IsValidWord(std::string_view word);
	std::tuple<std::string, size_t> ExtractWordFromString(const std::string_view s, char delimiter, size_t start_pos);
}


