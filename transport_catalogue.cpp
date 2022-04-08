#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>


#include "transport_catalogue.h"
#include "geo.h"

using namespace std;

size_t PairStopsHash::operator() (const std::pair<const Stop*, const Stop*>& stops_pair) const  {
	hash<const Stop*> test;
	return test(stops_pair.first) + 43 * test(stops_pair.second);
}

void TransportCatalogue::AddRoute(Route& route) {
	routes_deq.push_back(route);
	string route_name = routes_deq.back().name;
	routes[route_name] = &routes_deq.back();
}

void TransportCatalogue::AddStop(Stop& stop) {
	stops_deq.push_back(move(stop));
	string stop_name = stops_deq.back().name;
	stops[stop_name] = &stops_deq.back();
}

const Route* TransportCatalogue::GetRoute(string_view route_name) const {
	try {
		const Route* result = routes.at(static_cast<string>(route_name));
		return result;
	} catch (const std::out_of_range& e) {
		return nullptr;
	}
}

const Stop* TransportCatalogue::GetStop(string_view stop_name) const {
	try {
		const Stop* result = stops.at(static_cast<string>(stop_name));
		return result;
	} catch (const std::out_of_range& e) {
		return nullptr;
	}
}

RouteStat TransportCatalogue::GetRouteStat(const Route* route) const {
	RouteStat result;
	if (route == nullptr) {
		return result;
	}
	double total_dist_straight = 0;
	size_t total_dist_curved = 0;
	set<const Stop*> unique_stops;
	if (route->stop_list.size() > 0) {
		unique_stops.insert(route->stop_list[0]);
	}

	if (route->stop_list.size() > 1) {
		for (size_t i = 1; i < route->stop_list.size(); i++) {
			const geo::Coordinates c1 = {route->stop_list[i - 1]->coords.lat, route->stop_list[i - 1]->coords.lng};
			const geo::Coordinates c2 = {route->stop_list[i]->coords.lat, route->stop_list[i]->coords.lng};
			total_dist_straight += geo::ComputeDistance(c1, c2);
			total_dist_curved += GetStopsDistance(route->stop_list[i-1], route->stop_list[i]);
			unique_stops.insert(route->stop_list[i]);
		}
	}

	double curvness = total_dist_curved / total_dist_straight;
	result.curvness = curvness;
	result.length = total_dist_curved;
	result.num_of_stops = route->stop_list.size();
	result.num_of_unique_stops = unique_stops.size();
	return result;
}

const StopStat TransportCatalogue::GetStopStat(const Stop* stop) const {
	if (stop_to_buses.find(stop) != stop_to_buses.end()) {
		const StopStat result = StopStat(stop_to_buses.at(stop));
		return result;
	} else {
		return StopStat();
	}
}

void TransportCatalogue::AddBusRouteToStop(string bus_name, const Stop* stop_ptr) {
	stop_to_buses[stop_ptr].insert(bus_name);
}

void TransportCatalogue::SetStopsDistance(const Stop* this_stop, const Stop* next_stop, size_t distance) {
	stop_to_stop_dist[make_pair(this_stop, next_stop)] = distance;
}

size_t TransportCatalogue::GetStopsDistance(const Stop* this_stop, const Stop* next_stop) const {
	size_t result = 0;
	if (stop_to_stop_dist.find(make_pair(this_stop, next_stop)) != stop_to_stop_dist.end()) {
		result = stop_to_stop_dist.at(make_pair(this_stop, next_stop));
	} else if (stop_to_stop_dist.find(make_pair(next_stop, this_stop)) != stop_to_stop_dist.end()) {
		result = stop_to_stop_dist.at(make_pair(next_stop, this_stop));
	}
	return result;
}

std::unordered_map<std::string, const Route*>& TransportCatalogue::GetAllRoutes() {
	return routes;
}

std::unordered_map<std::string, const Stop*>& TransportCatalogue::GetAllStops() {
	return stops;
}

void TransportCatalogue::PrintEverything() {
	cout << "***********ROUTES*************" << endl;
	for(auto route : routes) {
		const Route* route_pointer = route.second;
			cout << route_pointer->name << ": " << route_pointer->circled << " ";
			for (auto stop: route_pointer->stop_list) {
				cout << stop-> name << " ";
			}
			cout << endl;
	}

	cout << endl << endl << "***********STOPS*************" << endl;
	for(auto stop : stops) {
		const Stop* stop_pointer = stop.second;
		cout << stop_pointer->name << " " << stop_pointer->coords.lat << " " << stop_pointer->coords.lng << endl;
	}

	cout << endl << endl << "***********DISTANCES*************" << endl;
	for(auto dist_pair : stop_to_stop_dist) {
		cout << dist_pair.first.first->name << " " << dist_pair.first.second->name << " " << dist_pair.second << endl;
	}
}

namespace detail {

	const string WHITESPACE = " \t";

	string_view Rtrim(string_view s) {
		size_t end = s.find_last_not_of(WHITESPACE);
		return (end == string::npos) ? "" : s.substr(0, end + 1);
	}

	string_view Ltrim(string_view s) {
		size_t start = s.find_first_not_of(WHITESPACE);
		return (start == string::npos) ? "" : s.substr(start);
	}

	bool IsValidWord(string_view word) {
		return all_of(word.begin(), word.end(), [](char c) {
			return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == ' '));
		});
	}

	tuple<string, size_t> ExtractWordFromString(const string_view str, char delimiter, size_t start_pos) {
		size_t delimiter_pos = str.find(delimiter, start_pos);
		if (delimiter_pos == string::npos) {
			return make_tuple(static_cast<string>(str.substr(start_pos)), string::npos);
		}
		size_t substr_len = delimiter_pos - start_pos;
		string result = static_cast<string>(str.substr(start_pos, substr_len));
		return make_tuple(result, delimiter_pos);
	}
}
