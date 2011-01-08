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

	// GEOMETRY
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

	// SIZE HINTS
	xcb_size_hints_t hints;
	if (!xcb_get_wm_normal_hints_reply(conn, xcb_get_wm_normal_hints_unchecked(conn, win), &hints, NULL)) {
		cout << "ERROR: Couldn't get size hints." << endl;
	}

	// user specified geometry
	// hints.flags & XCB_SIZE_HINT_US_POSITION

	if (hints.flags & XCB_SIZE_HINT_P_MIN_SIZE) {
		_min_width = hints.min_width;
		_min_height = hints.min_height;
	}

	// TITLE
	xcb_get_text_property_reply_t prop;
	xcb_get_property_cookie_t cookie = xcb_get_wm_name(conn, _id);
	uint8_t got_reply;
	got_reply = xcb_get_wm_name_reply(conn, cookie, &prop, NULL);
	if (!got_reply || prop.name_len == 0) {
		cout << "ERROR: No name for client" << endl;
		_title = "(none)";
	} else {
		cout << "DEBUG: Client Name = " << prop.name << endl;
		_title = string(prop.name);
	}
	xcb_get_text_property_reply_wipe(&prop);

	setupFrame();
	setupTitlebar();
	debug();

	_clients.push_back(this);
}

Client::~Client()
{
}

void Client::revert()
{
	cout << "attempting to reparent..." << endl;
	xcb_reparent_window(Screen::instance()->connection(), _id, Screen::instance()->screen()->root, _x, _y);
	cout << "done reparenting..." << endl;
}

void Client::debug()
{
	cout << "DEBUG: client = " << _id << endl;
	cout << "DEBUG:   x = " << _x << endl;
	cout << "DEBUG:   y = " << _y << endl;
	cout << "DEBUG:   width = " << _width << endl;
	cout << "DEBUG:   height = " << _height << endl;
	cout << "DEBUG:   min_width = " << _min_width << endl;
	cout << "DEBUG:   min_height = " << _min_height << endl;
	cout << "DEBUG:   max_width = " << _max_width << endl;
	cout << "DEBUG:   max_height = " << _max_height << endl;
}

int Client::count()
{
	return((int)_clients.size());
}

list<Client *> Client::clients()
{
	return(_clients);
}

void Client::setupTitlebar()
{
	drawText(_title.c_str(), _frame, 0, 0, _width, 16);

	xcb_connection_t *conn = Screen::instance()->connection();
	xcb_screen_t *screen = Screen::instance()->screen();
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	_titlebar = xcb_generate_id(conn);
	xcb_create_window(conn,
				XCB_COPY_FROM_PARENT,
				_titlebar,
				_frame,
				_x, _y,
				_width, CLIENT_TITLEBAR_HEIGHT,
				0,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				screen->root_visual,
				CLIENT_WINDOW_MASK, values);
	xcb_map_window(conn, _titlebar);
	drawText(_title.c_str(), _titlebar, 2, 2, _width, CLIENT_TITLEBAR_HEIGHT);
	xcb_flush(conn);
}

void Client::setupFrame()
{
	xcb_connection_t *conn = Screen::instance()->connection();
	xcb_screen_t *screen = Screen::instance()->screen();
	uint32_t values[2] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	_frame = xcb_generate_id(conn);
	xcb_create_window(conn,
				XCB_COPY_FROM_PARENT,
				_frame,
				screen->root,
				_x, _y - CLIENT_TITLEBAR_HEIGHT,
				_width, _height + CLIENT_TITLEBAR_HEIGHT,
				1,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				screen->root_visual,
				CLIENT_WINDOW_MASK, values);
	xcb_map_window(conn, _frame);
	//drawText("test text", _frame, 0, 0, 100, 16);
	xcb_reparent_window(conn, _id, _frame, 0, CLIENT_TITLEBAR_HEIGHT);
}

void Client::drawText(const char * str, xcb_window_t win, int x, int y, int w, int h)
{
	xcb_connection_t *conn = Screen::instance()->connection();
	xcb_screen_t *screen = Screen::instance()->screen();
	xcb_visualtype_t *visual = draw_screen_default_visual(screen);
	cairo_surface_t *surface = cairo_xcb_surface_create(conn, win, visual, w, h);
	cairo_t *cr = cairo_create(surface);

	// nice couple pixel padding between the text and the edge
	cairo_move_to(cr, (double)x, (double)y);
	
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, str, -1);
	PangoFontDescription *font_description = pango_font_description_from_string(CLIENT_TITLEBAR_FONT);
	pango_layout_set_font_description(layout, font_description);
	pango_font_description_free(font_description);
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
	pango_cairo_show_layout(cr, layout);
	cairo_destroy(cr);
}
