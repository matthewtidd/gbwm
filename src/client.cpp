#include "client.h"
#include "screen.h"
#include <malloc.h>

list<Client *> Client::_clients;
xcb_visualtype_t* Client::_visual = 0;
xcb_connection_t* Client::_conn = 0;
xcb_screen_t* Client::_screen = 0;

Client::Client(xcb_window_t win)
{
	if (_conn == 0) {
		_conn = Screen::conn();
	}
	if (_screen == 0) {
		_screen = Screen::screen();
	}
	if (_visual == 0) {
		_visual = Screen::visual();
	}

	_id = win;
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
	_mapped = false;
	_reparented = false;

	// GEOMETRY
	xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry(_conn, win);
	xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(_conn, geomCookie, NULL);
	if (geometry) {
		_x = geometry->x;
		_y = geometry->y;
		_width = geometry->width;
		_height = geometry->height;
	}
	free(geometry);

	_min_width = 0;
	_min_height = 0;
	_max_width = _screen->width_in_pixels;
	_max_height = _screen->height_in_pixels;

	// SIZE HINTS
	xcb_size_hints_t hints;
	if (!xcb_get_wm_normal_hints_reply(_conn, xcb_get_wm_normal_hints_unchecked(_conn, win), &hints, NULL)) {
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
	xcb_get_property_cookie_t cookie = xcb_get_wm_name(_conn, _id);
	uint8_t got_reply;
	got_reply = xcb_get_wm_name_reply(_conn, cookie, &prop, NULL);
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
	delete(_closeButton);
}

Client *Client::getByWindow(xcb_window_t window)
{
	list<Client *> clients = Client::clients();
	list<Client *>::iterator iter;
	for (iter = clients.begin(); iter != clients.end(); iter++) {
		Client *c = (Client *)*iter;
		if (c->_id == window) {
			return(c);
		}
	}
	return(NULL);
}

void Client::destroy(Client *client)
{
	_clients.remove(client);
	delete(client);
}

void Client::revert()
{
	cout << "attempting to reparent..." << endl;
	xcb_reparent_window(_conn, _id, _screen->root, _x, _y);
	uint32_t values[1] = {1};
	xcb_configure_window(_conn, _id, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
	delete(_titlebar);
	delete(_frame);
	xcb_flush(_conn);
	cout << "done reparenting..." << endl;
}

void Client::map()
{
	if (!_mapped) {
		_frame->map();
		_titlebar->map();
		_closeButton->map();

		if (!_reparented) {
			xcb_reparent_window(_conn, _id, _frame->id(), 0, CLIENT_TITLEBAR_HEIGHT);
			xcb_flush(_conn);

			// remove the border
			uint32_t move_values[1] = { 0 };
			xcb_configure_window(_conn, _id, XCB_CONFIG_WINDOW_BORDER_WIDTH, move_values);
		}
		_mapped = true;
	}
}

void Client::unmap()
{
	// X sends us an unmap when we try to reparent, we need to ignore it
	cout << "Client unmap - " << (_mapped ? "mapped" : "not mapped") << (_reparented ? "reparented" : "not reparented") << endl;
	if (_mapped && _reparented) {
		_closeButton->unmap();
		_titlebar->unmap();
		_frame->unmap();
		xcb_flush(Screen::conn());
		_mapped = false;
	}
}

void Client::reparent()
{
	if (!_reparented) {
		_reparented = true;
		const uint32_t win_vals[] = {
			XCB_EVENT_MASK_PROPERTY_CHANGE |
			XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
		};
		xcb_change_window_attributes(Screen::conn(), _id, XCB_CW_EVENT_MASK, win_vals);
	} else {
		cout << "ERROR: Call to client reparent that was already done!" << endl;
	}
}

xcb_drawable_t Client::id()
{
	return(_id);
}

xcb_window_t Client::window() const
{
	return(_id);
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
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2] = {_screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	_titlebar = new Titlebar(_title.c_str(), _frame, 0, 0, _width, CLIENT_TITLEBAR_HEIGHT, 0, mask, values);

	_closeButton = new Button(this, BUTTON_CLOSE, _titlebar, _width - 1 - 12, 1, 12, 12, 0, mask, values);
	xcb_flush(_conn);
}

void Client::setupFrame()
{
	uint32_t values[3] = {_screen->white_pixel, _screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK;

	_frame = new Window(NULL, _x, _y, _width, _height + CLIENT_TITLEBAR_HEIGHT, 1, mask, values);

	xcb_flush(_conn);
}
