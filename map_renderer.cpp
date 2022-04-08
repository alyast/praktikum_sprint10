#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
using namespace std;

namespace renderer {
	MapRenderer::MapRenderer(): rs_({}) {}
	MapRenderer::MapRenderer(RenderSettings rs): rs_(rs){}
	void MapRenderer::SetSettings(RenderSettings rs) {rs_ = rs;}
	RenderSettings MapRenderer::GetSettings() {return rs_;}
	double MapRenderer::GetHeight() {return rs_.height;}
	double MapRenderer::GetWidth() {return rs_.width;}
	double MapRenderer::GetPadding() {return rs_.padding;}
	double MapRenderer::GetLineWidth() {return rs_.padding;}
	double MapRenderer::GetStopRadius() {return rs_.stop_radius;}
	size_t MapRenderer::GetBusLabelFontSize() {return rs_.bus_label_font_size;}
	Offset MapRenderer::GetBusLabelOffset() {return rs_.bus_label_offset;}
	size_t MapRenderer::GetStopLabelFontSize() {return rs_.bus_label_font_size;}
	Offset MapRenderer::GetStopLabelOffset() {return rs_.stop_label_offset;}
	svg::Color MapRenderer::GetUnderlayerColor() {return rs_.underlayer_color;}
	double MapRenderer::GetUnderlayerWidth() {return rs_.underlayer_width;}
	vector<svg::Color> MapRenderer::GetColorPalette() {return rs_.color_palette;}
}