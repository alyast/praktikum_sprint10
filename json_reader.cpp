#include <fstream>
#include <fstream>

#include "json_reader.h"

using namespace std;

namespace json_reader {

	void ReadAllRequests(istream& is, TransportCatalogue& tc) {
		renderer::MapRenderer mr;
		json::Document input_doc = json::Load(is);
		json::Node root_node = input_doc.GetRoot();
		ReadRenderSettings(root_node, mr);
		ReadInputRequests(root_node, tc);
		ReadStatRequests(root_node, tc, mr);
	}

	void ReadInputRequests(json::Node& root_node, TransportCatalogue& tc) {
		vector<BusInput> buses_info;
		vector<StopInput> stops_info;
		json::Dict root_dict = root_node.AsMap();
		json::Node base_requests = root_dict.at("base_requests");
		json::Array base_requests_arr = base_requests.AsArray();
		json::Array stat_requests_arr = base_requests.AsArray();
		for(json::Node& base_request : base_requests_arr) {
			json::Dict base_request_map = base_request.AsMap();
			if (base_request.AsMap().at("type").AsString() == "Bus") {
				BusInput new_bus;
				new_bus.name = base_request_map.at("name").AsString();
				new_bus.roundtrip = base_request_map.at("is_roundtrip").AsBool();
				json::Array stops_arr = base_request_map.at("stops").AsArray();
				for (json::Node& stop_node : stops_arr) {
					new_bus.stops.push_back(stop_node.AsString());
				}
				buses_info.push_back(new_bus);
			} else if (base_request.AsMap().at("type").AsString() == "Stop") {
				StopInput new_stop;
				new_stop.name = base_request_map.at("name").AsString();
				new_stop.latitude = base_request_map.at("latitude").AsDouble();
				new_stop.longitude = base_request_map.at("longitude").AsDouble();
				json::Dict distances;
				try {
					distances = base_request_map.at("road_distances").AsMap();
				}
				catch (const out_of_range& err) {
				}
				for(auto& [key, value] : distances) {
					new_stop.distances[key] = value.AsInt();
				}
				stops_info.push_back(new_stop);
			}
		}
		AddDataToTransportCatalogue(stops_info, buses_info, tc);
	}

	void ReadStatRequests(json::Node& root_node, TransportCatalogue& tc, renderer::MapRenderer& mr) {
		vector<StatRequest> stat_requests_info;
		json::Dict root_dict = root_node.AsMap();
		json::Node stat_requests = root_dict.at("stat_requests");
		json::Array stat_requests_arr = stat_requests.AsArray();
		for(json::Node& stat_request : stat_requests_arr) {
			StatRequest stat_request_info;
			json::Dict stat_request_map = stat_request.AsMap();
			stat_request_info.id = stat_request_map.at("id").AsInt();
			stat_request_info.type = stat_request_map.at("type").AsString();
			if ((stat_request_info.type == "Stop") || (stat_request_info.type == "Bus")) {
				stat_request_info.name = stat_request_map.at("name").AsString();
			}
			stat_requests_info.push_back(stat_request_info);
		}
		ProcessStatRequests(stat_requests_info, tc, mr);
	}

	void ReadRenderSettings(json::Node& root_node, renderer::MapRenderer& mr) {
		json::Dict root_dict = root_node.AsMap();
		json::Node render_settings_node = root_dict.at("render_settings");
		json::Dict render_settings_dict = render_settings_node.AsMap();
		renderer::RenderSettings rs;
		rs.width = render_settings_dict.at("width").AsDouble();
		rs.height = render_settings_dict.at("height").AsDouble();
		rs.padding = render_settings_dict.at("padding").AsDouble();
		rs.line_width = render_settings_dict.at("line_width").AsDouble();
		rs.stop_radius = render_settings_dict.at("stop_radius").AsDouble();
		rs.bus_label_font_size = render_settings_dict.at("bus_label_font_size").AsInt();
		rs.bus_label_offset.x = render_settings_dict.at("bus_label_offset").AsArray()[0].AsDouble();
		rs.bus_label_offset.y = render_settings_dict.at("bus_label_offset").AsArray()[1].AsDouble();
		rs.stop_label_font_size = render_settings_dict.at("stop_label_font_size").AsInt();
		rs.stop_label_offset.x = render_settings_dict.at("stop_label_offset").AsArray()[0].AsDouble();
		rs.stop_label_offset.y = render_settings_dict.at("stop_label_offset").AsArray()[1].AsDouble();
		rs.underlayer_color = GetColor(render_settings_dict.at("underlayer_color"));
		rs.underlayer_width = render_settings_dict.at("underlayer_width").AsDouble();
		json::Array color_pallete_arr = render_settings_dict.at("color_palette").AsArray();
		for (json::Node& color_pallete_node : color_pallete_arr) {
			rs.color_palette.push_back(GetColor(color_pallete_node));
		}

		    //cout << "Input padding is: " << rs.padding << endl;

		mr.SetSettings(rs);
	}

	void AddDataToTransportCatalogue(vector<StopInput>& stops_info, vector<BusInput>& buses_info, TransportCatalogue& tc) {
		for (StopInput& stop_info : stops_info) {
			Stop stop_to_add;
			stop_to_add.name = stop_info.name;
			geo::Coordinates coords;
			coords.lat = stop_info.latitude;
			coords.lng = stop_info.longitude;
			stop_to_add.coords = coords;
			tc.AddStop(stop_to_add);
		}

		for (StopInput& stop_info : stops_info) {
			const Stop* stop_ptr = tc.GetStop(stop_info.name);
			for (auto& [neighbour_stop_name, distance] : stop_info.distances) {
				tc.SetStopsDistance(stop_ptr, tc.GetStop(neighbour_stop_name), distance);
			}
		}

		for (BusInput& bus_info : buses_info) {
			Route route_to_add;
			route_to_add.name = bus_info.name;
			route_to_add.circled = bus_info.roundtrip;
			for (auto& stop_in_route_name : bus_info.stops) {
				route_to_add.stop_list.push_back(tc.GetStop(stop_in_route_name));
			}
			for(const Stop* stop_ptr : route_to_add.stop_list) {
				tc.AddBusRouteToStop(route_to_add.name, stop_ptr);
			}

			if (!route_to_add.circled) {
				for (int i = route_to_add.stop_list.size() - 2; i >= 0; i--) {
					route_to_add.stop_list.push_back(route_to_add.stop_list[i]);
				}
			}
			tc.AddRoute(route_to_add);
		}
	}

	void ProcessStatRequests(vector<StatRequest>& stat_requests_info, TransportCatalogue& tc, renderer::MapRenderer& mr) {
        cout << setprecision(6);
		RequestHandler rh = RequestHandler(tc, mr);

		json::Array requests_output;
		for (StatRequest& stat_request_info : stat_requests_info) {
			json::Dict result;
			json::Node id_node = json::Node(static_cast<int>(stat_request_info.id));
			result["request_id"] = id_node;

			//cout << "Current stat request id is: " << stat_request_info.id << endl;

			if(stat_request_info.type == "Stop") {
				auto stop_statistics = rh.GetStopStat(stat_request_info.name);
				if (!stop_statistics) {
					json::Node error_message_node = json::Node("not found"s);
					result["error_message"] = error_message_node;
				} else {
					json::Array buses_list_node;
					const StopStat stop_statistics_value = *stop_statistics;
					set<string> buses_for_stop_set = stop_statistics_value.buses;
					for (string bus_name : buses_for_stop_set) {
						buses_list_node.push_back(bus_name);
					}
					result["buses"] = buses_list_node;
				}
				//cout << "Done stop" << endl;

			} else if (stat_request_info.type == "Bus") {

				auto bus_statistics = rh.GetBusStat(stat_request_info.name);
				if (!bus_statistics) {
					json::Node error_message_node = json::Node("not found"s);
					result["error_message"] = error_message_node;
				} else {
					json::Node curvature_node = json::Node((*bus_statistics).curvness);
                    json::Node route_length_node;
					if ((*bus_statistics).length < 1000000) {
						route_length_node = json::Node(static_cast<int>((*bus_statistics).length));
					} else {
						route_length_node = json::Node(static_cast<double>((*bus_statistics).length));
					}
					json::Node stop_count_node = json::Node(static_cast<int>((*bus_statistics).num_of_stops));
					json::Node unique_stop_count_node = json::Node(static_cast<int>((*bus_statistics).num_of_unique_stops));
					result["curvature"] = curvature_node;
					result["route_length"] = route_length_node;
					result["stop_count"] = stop_count_node;
					result["unique_stop_count"] = unique_stop_count_node;
				}
				//cout << "Done bus" << endl;

			} else if (stat_request_info.type == "Map") {
				result["map"] = RenderTransportCatalogue(tc, mr);
				//cout << "Done map" << endl;
			}
			requests_output.push_back(result);
		}
		//cout << requests_output.size() << endl;
		json::Node node_to_print = json::Node(requests_output);
		json::Document doc_to_print = json::Document(node_to_print);
		//ofstream outfile;
		//outfile.open("example6_my_solution.txt");
		json::Print(doc_to_print, cout);
		//json::Print(doc_to_print, outfile);
	}

	svg::Color GetColor(json::Node& color_node){
		if (color_node.IsString()) {
			return color_node.AsString();
		} else if (color_node.IsArray()) {
			json::Array color_array = color_node.AsArray();
			if (color_array.size() == 3) {
				svg::Rgb rgb_color_result;
				rgb_color_result.red = color_array[0].AsInt();
				rgb_color_result.green = color_array[1].AsInt();
				rgb_color_result.blue = color_array[2].AsInt();
				return rgb_color_result;
			} else if (color_array.size() == 4) {
				svg::Rgba rgba_color_result;
				rgba_color_result.red = color_array[0].AsInt();
				rgba_color_result.green = color_array[1].AsInt();
				rgba_color_result.blue = color_array[2].AsInt();
				rgba_color_result.opacity = color_array[3].AsDouble();
				return rgba_color_result;
			}
		}
		svg::Color empty_val = monostate{};
		return empty_val;
	}

	string RenderTransportCatalogue(TransportCatalogue& tc, renderer::MapRenderer& mr) {
		unordered_map<string, const Stop*>& tc_stops = tc.GetAllStops();
		double min_lat = 0;
        double min_lon = 0;
        double max_lat = 0;
        double max_lon = 0;
		bool first_iter = true;
		for (auto& [name, stop] : tc_stops) {
            const StopStat stop_stat = tc.GetStopStat(tc.GetStop(name));
			if (stop_stat.buses.size() > 0) {
                if (first_iter) {
                    min_lat = stop->coords.lat;
                    max_lat = stop->coords.lat;
                    min_lon = stop->coords.lng;
                    max_lon = stop->coords.lng;
                    first_iter = !first_iter;
                } else {
                    min_lat = (stop->coords.lat < min_lat) ? stop->coords.lat : min_lat;
                    min_lon = (stop->coords.lng < min_lon) ? stop->coords.lng : min_lon;
                    max_lat = (stop->coords.lat > max_lat) ? stop->coords.lat : max_lat;
                    max_lon = (stop->coords.lng > max_lon) ? stop->coords.lng : max_lon;
                }
            }
		}

		renderer::RenderSettings rs = mr.GetSettings();
		bool good_width_zoom = (max_lon > min_lon);
		bool good_height_zoom = (max_lat > min_lat);
		double width_zoom_coef = good_width_zoom ? ((rs.width- 2 * rs.padding) / (max_lon - min_lon)) : 0;
		double height_zoom_coef = good_height_zoom ? ((rs.height - 2 * rs.padding) / (max_lat - min_lat)) : 0;

		double zoom_coef;
		if ((good_width_zoom) && (good_height_zoom)) {
			zoom_coef = (width_zoom_coef < height_zoom_coef) ? width_zoom_coef : height_zoom_coef;
		} else if (!good_width_zoom) {
			zoom_coef = height_zoom_coef;
		} else if (!good_height_zoom) {
			zoom_coef = width_zoom_coef;
		} else {
			zoom_coef = 0;
		}

		map<string, renderer::SvgCoords> stops_svg_coords;
		for (auto& [name, stop] : tc_stops) {
			renderer::SvgCoords stop_coords;
			stop_coords.x = (stop->coords.lng - min_lon) * zoom_coef + rs.padding;
			stop_coords.y = (max_lat - stop->coords.lat) * zoom_coef + rs.padding;
			stops_svg_coords[name] = stop_coords;
		}

		stringstream buffer;
		svg::Document doc = svg::Document();

		unordered_map<string, const Route*>& tc_unordered_routes = tc.GetAllRoutes();
		map<string,  const Route*> tc_routes;

		for (auto& [name, route] : tc_unordered_routes) {
			tc_routes[name] = route;
		}

		size_t color_index = 0;

		for (auto& [name, route] : tc_routes) {
			std::vector<const Stop*> stop_list = route->stop_list;
			if (stop_list.size() > 0) {
				vector<renderer::SvgCoords> svg_coords_for_polyline;
				for (auto stop : stop_list) {
					svg_coords_for_polyline.push_back(stops_svg_coords.at(stop->name));
				}
				svg::Polyline route_polyline;
				route_polyline.SetFillColor(monostate{});
				route_polyline.SetStrokeWidth(rs.line_width);
				route_polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				route_polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				for (auto coord: svg_coords_for_polyline) {
					svg::Point path_point = svg::Point(coord.x, coord.y);
					route_polyline.AddPoint(path_point);
				}
				svg::Color route_color = rs.color_palette[color_index];
				route_polyline.SetStrokeColor(route_color);
				doc.Add(route_polyline);
				color_index = (color_index < rs.color_palette.size() - 1) ? (color_index + 1) : 0;
			}
		}

		color_index = 0;
		for (auto& [name, route] : tc_routes) {
			std::vector<const Stop*> stop_list = route->stop_list;
			if (stop_list.size() > 0) {
				svg::Text label_common_props;
				svg::Point label_offset = svg::Point(rs.bus_label_offset.x, rs.bus_label_offset.y);
				label_common_props.SetOffset(label_offset);
				label_common_props.SetFontSize(rs.bus_label_font_size);
				label_common_props.SetFontFamily("Verdana");
				label_common_props.SetFontWeight("bold");
				string first_stop = stop_list[0]->name;
				label_common_props.SetData(name);
				svg::Point first_stop_coords = svg::Point(stops_svg_coords.at(first_stop).x, stops_svg_coords.at(first_stop).y);
				label_common_props.SetPosition(first_stop_coords);
				svg::Text label_route = label_common_props;
				svg::Text label_route_back = label_common_props;
				svg::Color route_color = rs.color_palette[color_index];
				label_route.SetFillColor(route_color);
				label_route_back.SetFillColor(rs.underlayer_color);
				label_route_back.SetStrokeColor(rs.underlayer_color);
				label_route_back.SetStrokeWidth(rs.underlayer_width);
				label_route_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				label_route_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				doc.Add(label_route_back);
				doc.Add(label_route);
				string last_stop = stop_list[stop_list.size() / 2]->name;
				if ((!route->circled) && (last_stop != first_stop)) {
                //if ((!route->circled)) {
					svg::Point last_stop_coords = svg::Point(stops_svg_coords.at(last_stop).x, stops_svg_coords.at(last_stop).y);
					label_route.SetPosition(last_stop_coords);
					label_route_back.SetPosition(last_stop_coords);
					doc.Add(label_route_back);
					doc.Add(label_route);
				}
				color_index = (color_index < rs.color_palette.size() - 1) ? (color_index + 1) : 0;
			}
		}


		for (auto& [name, coords] : stops_svg_coords) {
			const StopStat stop_stat = tc.GetStopStat(tc.GetStop(name));
			if (stop_stat.buses.size() > 0) {
				svg::Point circle_center = svg::Point(coords.x, coords.y);
				svg::Circle stop_circle = svg::Circle();
				stop_circle.SetCenter(circle_center);
				stop_circle.SetRadius(rs.stop_radius);
				stop_circle.SetFillColor("white"s);
				doc.Add(stop_circle);
			}
		}

		for (auto& [name, coords] : stops_svg_coords) {
			const StopStat stop_stat = tc.GetStopStat(tc.GetStop(name));
			if (stop_stat.buses.size() > 0) {
				svg::Point circle_center = svg::Point(coords.x, coords.y);
				svg::Text label_common_props;
				label_common_props.SetPosition(circle_center);
				svg::Point text_offset;
				text_offset.x = rs.stop_label_offset.x;
				text_offset.y = rs.stop_label_offset.y;
				label_common_props.SetOffset(text_offset);
				label_common_props.SetFontSize(rs.stop_label_font_size);
				label_common_props.SetFontFamily("Verdana");
				label_common_props.SetData(name);
				svg::Text stop_label = label_common_props;
				svg::Text stop_background = label_common_props;

				stop_background.SetFillColor(rs.underlayer_color);
				stop_background.SetStrokeColor(rs.underlayer_color);
				stop_background.SetStrokeWidth(rs.underlayer_width);
				stop_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				stop_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				stop_label.SetFillColor("black");

				doc.Add(stop_background);
				doc.Add(stop_label);
			}
		}

		doc.Render(buffer);
		string result = buffer.str();
		return result;
	}
}
