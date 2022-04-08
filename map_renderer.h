#pragma once
#include <vector>

#include "domain.h"
#include "geo.h"
#include "svg.h"
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {

	struct Offset {
		double x;
		double y;
	};

	struct RenderSettings {
		double width;
		double height;
		double padding;
		double line_width;
		double stop_radius;
		size_t bus_label_font_size;
		Offset bus_label_offset;
		size_t stop_label_font_size;
		Offset stop_label_offset;
		svg::Color underlayer_color;
		double underlayer_width;
		std::vector<svg::Color> color_palette;
	};

	struct SvgCoords {
		double x;
		double y;
	};

	class MapRenderer {
	public:
		MapRenderer();
		MapRenderer(RenderSettings rs);
		void SetSettings(RenderSettings rs);
		RenderSettings GetSettings();
		double GetHeight();
		double GetWidth();
		double GetPadding();
		double GetLineWidth();
		double GetStopRadius();
		size_t GetBusLabelFontSize();
		Offset GetBusLabelOffset();
		size_t GetStopLabelFontSize();
		Offset GetStopLabelOffset();
		svg::Color GetUnderlayerColor();
		double GetUnderlayerWidth();
		std::vector<svg::Color> GetColorPalette();
	private:
		RenderSettings rs_;
	};
}
