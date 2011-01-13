#include "titlebar.h"
#include "screen.h"
#include "client.h"

Titlebar::Titlebar(const char* txt, Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values) :
	Window(parent, x, y, w, h, border, mask, values)
{
	_type = WINDOW_TYPE_TITLEBAR;
	_text = string(txt);
}

Titlebar::~Titlebar()
{
}

void Titlebar::draw()
{
	// fill the background
	cairo_surface_t *surface = cairo_xcb_surface_create(Screen::conn(), id(), Screen::visual(), width(), height());
	cairo_t *cr = cairo_create(surface);
	cairo_rectangle(cr, 0, 0, width(), height());
	cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_set_line_width(cr, 1.0);
	cairo_move_to(cr, 0.0, height());
	cairo_line_to(cr, width(), height());
	cairo_stroke(cr);
	cairo_destroy(cr);

	// draw the text
	cairo_surface_t *text_surface = cairo_xcb_surface_create(Screen::conn(), id(), Screen::visual(), width(), height());
	cr = cairo_create(text_surface);

	// nice couple pixel padding between the text and the edge
	cairo_move_to(cr, 2, 2);
	
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, _text.c_str(), -1);
	PangoFontDescription *font_description = pango_font_description_from_string(CLIENT_TITLEBAR_FONT);
	pango_layout_set_font_description(layout, font_description);
	pango_font_description_free(font_description);
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	pango_cairo_show_layout(cr, layout);
	cairo_destroy(cr);
}

void Titlebar::setText(const char *txt)
{
	_text = string(txt);
	draw();
}
