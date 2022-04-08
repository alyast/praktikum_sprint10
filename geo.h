#pragma once

#include <cmath>
#include <iostream>

namespace geo {

	struct Coordinates {
		double lat;
		double lng;
	};

	double ComputeDistance(Coordinates from, Coordinates to);

} // end of namespace geo