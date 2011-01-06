#include "client.h"
#include "screen.h"
#include <malloc.h>

list<Client *> Client::_clients;

Client::Client(xcb_window_t win)
{
	xcb_connection_t *conn = Screen::instance()->connection();

	_id = win;
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;

	xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry(conn, win);
	xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn, geomCookie, NULL);
	if (geometry) {
		_x = geometry->x;
		_y = geometry->y;
		_width = geometry->width;
		_height = geometry->height;
	}
	free(geometry);

	_min_width = 0;
	_min_height = 0;
	_max_width = Screen::instance()->screen()->width_in_pixels;
	_max_height = Screen::instance()->screen()->height_in_pixels;

	xcb_size_hints_t hints;
	if (!xcb_get_wm_normal_hints_reply(conn, xcb_get_wm_normal_hints_unchecked(conn, win), &hints, NULL)) {
		cout << "Couldn't get size hints." << endl;
	}

	// user specified geometry
	// hints.flags & XCB_SIZE_HINT_US_POSITION

	if (hints.flags & XCB_SIZE_HINT_P_MIN_SIZE) {
		_min_width = hints.min_width;
		_min_height = hints.min_height;
	}
	setupTitlebar();
	debug();

	_clients.push_back(this);
}

Client::~Client()
{
}

void Client::debug()
{
	cout << "client = " << _id << endl;
	cout << "  x = " << _x << endl;
	cout << "  y = " << _y << endl;
	cout << "  width = " << _width << endl;
	cout << "  height = " << _height << endl;
	cout << "  min_width = " << _min_width << endl;
	cout << "  min_height = " << _min_height << endl;
	cout << "  max_width = " << _max_width << endl;
	cout << "  max_height = " << _max_height << endl;
}

int Client::count()
{
	return((int)_clients.size());
}

void Client::setupTitlebar()
{
	xcb_connection_t *conn = Screen::instance()->connection();
	xcb_screen_t *screen = Screen::instance()->screen();
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	xcb_window_t _titlebar = xcb_generate_id(conn);
	xcb_create_window(conn,
				XCB_COPY_FROM_PARENT,
				_titlebar,
				screen->root,
				_x, _y - 21,
				100, 21,
				0,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				screen->root_visual,
				mask, values);
	xcb_map_window(conn, _titlebar);

	xcb_visualtype_t *visual = draw_screen_default_visual(screen);
	cairo_surface_t *surface = cairo_xcb_surface_create(conn, _titlebar, visual, 100, 21);
	cairo_t *cr = cairo_create(surface);

	cairo_rectangle(cr, 0.0, 0.0, 100, 21);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_fill(cr);
	//cairo_move_to(cr, 10.0, 20.0);
	
//	cairo_show_text(cr, "hi");
	
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, "Text", 4);
	//pango_layout_set_width(layout, pango_units_from_double(100.0));
	//pango_layout_set_height(layout, pango_units_from_double(21.0));
	//pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_NONE);
	//pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
	PangoFontDescription *font_description = pango_font_description_from_string("sans 10");
	pango_layout_set_font_description(layout, font_description);
	pango_font_description_free(font_description);

	int height = 0;
	int width = 0;
	
	cairo_save(cr);
	cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
	pango_cairo_update_layout(cr, layout);
	pango_layout_get_size(layout, &width, &height);
	cout << "pango size = " << width << "x" << height << endl;

	pango_cairo_show_layout(cr, layout);
	cairo_restore(cr);

	cairo_destroy(cr);

/*
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_EVENT_MASK_EXPOSURE;
	uint32_t values[2] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	_titlebar = xcb_generate_id(conn);
	xcb_create_window(conn,
				XCB_COPY_FROM_PARENT,
				_titlebar,
				screen->root,
				20, 500,
				100, 21,
				10,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				screen->root_visual,
				mask, values);
	xcb_map_window(conn, _titlebar);

	*/
	xcb_flush(conn);
}
