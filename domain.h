/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#pragma once

#include <set>
#include <string>
#include <vector>

#include "geo.h"

struct Stop {
	geo::Coordinates coords;
	std::string name;
};


struct Route {
	bool circled;
	std::string name;
	std::vector<const Stop*> stop_list;
};

struct RouteStat {
    size_t num_of_stops;
    size_t num_of_unique_stops;
    size_t length;
    double curvness;
};

struct StopStat {
	StopStat(std::set<std::string>);
	StopStat();
	const std::set<std::string> buses;
};

