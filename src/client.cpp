#include "client.h"
#include "screen.h"
#include <malloc.h>
#include "log.h"

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
		LOG_ERROR("Counldn't get size hints.");
	}

	// user specified geometry
	// hints.flags & XCB_SIZE_HINT_US_POSITION

	if (hints.flags & XCB_SIZE_HINT_P_MIN_SIZE) {
		_min_width = hints.min_width;
		_min_height = hints.min_height;
	}

	setupFrame();
	setupTitlebar();
	debug();

	_clients.push_back(this);
}

Client::~Client()
{
	if (_closeButton) {
		delete(_closeButton);
	}
	if (_titlebar) {
		delete(_titlebar);
	}
	if (_frame) {
		delete(_frame);
	}
	_clients.remove(this);
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
	client->revert();
	_clients.remove(client);
	delete(client);
}

void Client::revert()
{
	LOG_DEBUG("Attempting to reparent");
	xcb_reparent_window(_conn, _id, _screen->root, _x, _y);
	uint32_t values[1] = {1};
	xcb_configure_window(_conn, _id, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
	delete(_titlebar);
	_titlebar = 0;
	delete(_frame);
	_frame = 0;
	xcb_flush(_conn);
	LOG_DEBUG("Done reparenting");
}

void Client::map()
{
	if (!_mapped) {
		_frame->map();
		_titlebar->map();
		_closeButton->map();

		if (!_reparented) {
			LOG_DEBUG("Reparenting client to frame");
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
	
	LOG_DEBUG("Client unmap - " << (_mapped ? "mapped" : "not mapped") << (_reparented ? " reparented" : " not reparented"));
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
			XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
			XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
			XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
			XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
			XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
			XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
		};
		xcb_change_window_attributes(Screen::conn(), _id, XCB_CW_EVENT_MASK, win_vals);
	} else {
		LOG_ERROR("Call to client reparent that was already done!");
	}
}

void Client::updateTitle(const char *str)
{
	_titlebar->setText(str);
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
	LOG_DEBUG("client = " << _id);
	LOG_DEBUG("  x = " << _x);
	LOG_DEBUG("  y = " << _y);
	LOG_DEBUG("  width = " << _width);
	LOG_DEBUG("  height = " << _height);
	LOG_DEBUG("  min_width = " << _min_width);
	LOG_DEBUG("  min_height = " << _min_height);
	LOG_DEBUG("  max_width = " << _max_width);
	LOG_DEBUG("  max_height = " << _max_height);
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

	// TITLE
	xcb_get_text_property_reply_t prop;
	xcb_get_property_cookie_t cookie = xcb_get_wm_name(_conn, _id);
	uint8_t got_reply;
	got_reply = xcb_get_wm_name_reply(_conn, cookie, &prop, NULL);
	if (!got_reply || prop.name_len == 0) {
		LOG_ERROR("No name for client");
		_titlebar = new Titlebar("", _frame, 0, 0, _width, CLIENT_TITLEBAR_HEIGHT, 0, mask, values);
	} else {
		LOG_DEBUG("Client name = " << prop.name);
		_titlebar = new Titlebar(prop.name, _frame, 0, 0, _width, CLIENT_TITLEBAR_HEIGHT, 0, mask, values);
	}
	xcb_get_text_property_reply_wipe(&prop);

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
