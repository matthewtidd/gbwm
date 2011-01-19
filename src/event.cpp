#include "event.h"
#include "client.h"
#include "window.h"
#include "log.h"
#include <iostream>

using namespace std;

Event* Event::_instance = 0;

extern void SignalHandler(int);

Event::Event()
{
	_error = false;
	_buttonPressed = 0;
	_i = 100;
	xcb_void_cookie_t cookie;
	xcb_generic_error_t *error;

	const uint32_t win_vals[] = {
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
	};
	cookie = xcb_change_window_attributes_checked(Screen::conn(), Screen::screen()->root, XCB_CW_EVENT_MASK, win_vals);
	error = xcb_request_check(Screen::conn(), cookie);
	if (error != NULL) {
		_error = true;
	} else {
		// example of how to register for EWMH properties
		// also requires a window with other properties set (including the wm name)
		// see http://specifications.freedesktop.org/wm-spec/wm-spec-latest.html
/*
		xcb_atom_t atom[] = {
			xcb_atom_get(Screen::conn(), "_NET_SUPPORTED"),
			xcb_atom_get(Screen::conn(), "_NET_WM_NAME"),
			xcb_atom_get(Screen::conn(), "WM_NAME"),
			xcb_atom_get(Screen::conn(), "XA_WM_NAME")
		};
		xcb_change_property(Screen::conn(), XCB_PROP_MODE_REPLACE, Screen::screen()->root, xcb_atom_get(Screen::conn(), "_NET_SUPPORTED"), ATOM, 32, 4, atom);
*/
		xcb_event_handlers_init(Screen::conn(), &_eh);

		xcb_event_set_key_press_handler(&_eh, Event::_handle_keypress, NULL);
		xcb_event_set_key_release_handler(&_eh, Event::_handle_keyrelease, NULL);
		xcb_event_set_button_press_handler(&_eh, Event::_handle_buttonpress, NULL);
		xcb_event_set_button_release_handler(&_eh, Event::_handle_buttonrelease, NULL);
		xcb_event_set_enter_notify_handler(&_eh, Event::_handle_enter, NULL);
		xcb_event_set_leave_notify_handler(&_eh, Event::_handle_leave, NULL);
		xcb_event_set_expose_handler(&_eh, Event::_handle_expose, NULL);
		xcb_event_set_create_notify_handler(&_eh, Event::_handle_create, NULL);
		xcb_event_set_destroy_notify_handler(&_eh, Event::_handle_destroy, NULL);
		xcb_event_set_unmap_notify_handler(&_eh, Event::_handle_unmap, NULL);
		xcb_event_set_map_notify_handler(&_eh, Event::_handle_map, NULL);
		xcb_event_set_map_request_handler(&_eh, Event::_handle_maprequest, NULL);
		xcb_event_set_reparent_notify_handler(&_eh, Event::_handle_reparent, NULL);
		xcb_event_set_configure_notify_handler(&_eh, Event::_handle_configure, NULL);
		xcb_event_set_configure_request_handler(&_eh, Event::_handle_configurerequest, NULL);
		xcb_event_set_property_notify_handler(&_eh, Event::_handle_property, NULL);
		xcb_event_set_client_message_handler(&_eh, Event::_handle_clientmessage, NULL);

		xcb_property_handlers_init(&_ph, &_eh);

		xcb_property_set_handler(&_ph, xcb_atom_get(Screen::conn(), "WM_NAME"), UINT_MAX, Event::_handle_property_wmname, NULL);
		//xcb_property_set_handler(&_ph, xcb_atom_get(Screen::conn(), "_NET_WM_NAME"), UINT_MAX, Event::_handle_property_netwmname, NULL);

	}

	if (_instance == 0) {
		_instance = this;
	}
}

Event::~Event()
{
}

Event* Event::instance()
{
	return(_instance);
}

void Event::loop()
{
	xcb_generic_event_t *e;
	while ((e = xcb_wait_for_event(Screen::conn()))) {
		_i = _i++;
		xcb_event_handle(&_eh, e);
		free(e);
	}
}

bool Event::error()
{
	return(_error);
}

int Event::handle_keypress(void *p, xcb_connection_t *conn, xcb_key_press_event_t *e)
{
	LOG_EVENT(_i << ":XCB_KEY_PRESS");
	return(0);
}

int Event::handle_keyrelease(void *p, xcb_connection_t *conn, xcb_key_release_event_t *e)
{
	LOG_EVENT(_i << ":XCB_KEY_RELEASE");
	LOG("KEY: " << (int)e->detail);
	// ESC exits
	if ((int)e->detail == 9) {
		SignalHandler(0);
	}
	return(0);
}

int Event::handle_buttonpress(void *p, xcb_connection_t *conn, xcb_button_press_event_t *e)
{
	LOG_EVENT(_i << ":XCB_BUTTON_PRESS");
	Window *window = Window::getWindowById(e->event);
	if (window) {
		_buttonPressed = window;
		window->mousePress(e);
	}
	return(0);
}

int Event::handle_buttonrelease(void *p, xcb_connection_t *conn, xcb_button_release_event_t *e)
{
	LOG_EVENT(_i << ":XCB_BUTTON_RELEASE");
	Window *window = Window::getWindowById(e->event);
	if (_buttonPressed) {
		if (window == _buttonPressed) {
			_buttonPressed->mouseRelease(e);
		} else {
			_buttonPressed->mouseCancel();
		}
		_buttonPressed = 0;
	}
	return(0);
}

int Event::handle_enter(void *p, xcb_connection_t *conn, xcb_enter_notify_event_t *e)
{
	//LOG_EVENT(_i << ":XCB_ENTER_NOTIFY");
	Window *window = Window::getWindowById(e->event);
	if (_buttonPressed && _buttonPressed == window) {
		_buttonPressed->mousePress(NULL);
	}
	return(0);
}

int Event::handle_leave(void *p, xcb_connection_t *conn, xcb_leave_notify_event_t *e)
{
	//LOG_EVENT(_i << ":XCB_LEAVE_NOTIFY");
	Window *window = Window::getWindowById(e->event);
	if (_buttonPressed && _buttonPressed == window) {
		_buttonPressed->mouseCancel();
	}
	return(0);
}

int Event::handle_expose(void *p, xcb_connection_t *conn, xcb_expose_event_t *e)
{
	LOG_EVENT(_i << ":XCB_EXPOSE");
	Window *window = Window::getWindowById(e->window);
	if (e->window == Screen::screen()->root) {
		Screen::instance()->setupBackground();
	}
	if (window) {
		window->draw();
		window->debug();
	}
	return(0);
}

int Event::handle_create(void *p, xcb_connection_t *conn, xcb_create_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_CREATE_NOTIFY");
	Client *c = Client::getByWindow(e->window);
	Window *win = Window::getWindowById(e->window);
	if (c) {
		LOG("EVENT: existing client!");
		c->debug();
	} else if (win) {
		LOG("EVENT: existing window!");
		win->debug();
	} else {
		LOG("EVENT: non-existing client!");
		LOG(" window : " << e->window);
		LOG(" parent : " << e->parent);
		LOG("      x : " << e->x);
		LOG("      y : " << e->y);
		LOG("  width : " << e->width);
		LOG(" height : " << e->height);
		LOG(" border : " << e->border_width);
		LOG("   over : " << (int)e->override_redirect);
		if ((int)e->override_redirect == 0) {
			new Client(e->window);
		}
	}
	return(0);
}

int Event::handle_destroy(void *p, xcb_connection_t *conn, xcb_destroy_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_DESTROY_NOTIFY");
	Client *c = Client::getByWindow(e->window);
	if (c) {
		LOG("EVENT: DESTROY client");
		Client::destroy(c);
	}
	return(0);
}

int Event::handle_unmap(void *p, xcb_connection_t *conn, xcb_unmap_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_UNMAP_NOTIFY");
	Client *c = Client::getByWindow(e->window);
	if (c) {
		LOG("EVENT: UNMAP client");
		c->unmap();
	}
	return(0);
}

int Event::handle_map(void *p, xcb_connection_t *conn, xcb_map_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_MAP_NOTIFY");
	Client *c = Client::getByWindow(e->window);
	if (c) {
		LOG("EVENT: MAP client");
		c->debug();
		c->map();
	}
	Window *win = Window::getWindowById(e->window);
	if (win) {
		win->debug();
	}
	return(0);
}

int Event::handle_maprequest(void *p, xcb_connection_t *conn, xcb_map_request_event_t *e)
{
	LOG_EVENT(_i << ":XCB_MAP_REQUEST");
	// this code is for subwindows
	Client *c = Client::getByWindow(e->window);
	if (c) {
		LOG("EVENT: MAP subwindow client");
		c->debug();
		xcb_map_window(Screen::conn(), e->window);
		c->map();
		c->reparent();
	} else {
		LOG("EVENT: unknown map request");
		xcb_map_window(Screen::conn(), e->window);
	}
	return(0);
}

int Event::handle_reparent(void *p, xcb_connection_t *conn, xcb_reparent_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_REPARENT_NOTIFY");
	Client *c = Client::getByWindow(e->window);
	if (c) {
		LOG("EVENT: REPARENT client");
		c->reparent();
	}
	return(0);
}

int Event::handle_configure(void *p, xcb_connection_t *conn, xcb_configure_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_CONFIGURE_NOTIFY");
	// only handle configure notify requests for root windows
	LOG("  event : " << e->event);
	LOG(" window : " << e->window);
	LOG("sibling : " << e->above_sibling);
	LOG("      x : " << e->x);
	LOG("      y : " << e->y);
	LOG("  width : " << e->width);
	LOG(" height : " << e->height);
	LOG(" border : " << e->border_width);
	LOG("   over : " << (int)e->override_redirect);
	Client *c = Client::getByWindow(e->window);
	if (c) {
		c->debug();
	}
	Window *win = Window::getWindowById(e->window);
	if (win) {
		win->debug();
	}
	if (e->window == Screen::screen()->root) {
		if (Client::getByWindow(e->window) == NULL) {
			new Client(e->window);
		}
	}
	return(0);
}

int Event::handle_configurerequest(void *p, xcb_connection_t *conn, xcb_configure_request_event_t *e)
{
	LOG_EVENT(_i << ":XCB_CONFIGURE_REQUEST");
	LOG(" window : " << e->window);
	// configure requests for clients are handled separately
	Client *c = Client::getByWindow(e->window);
	Window *win = Window::getWindowById(e->window);
	if (c) {
		c->debug();
		// reconfigure the window
	} else if (win) {
		win->debug();
	} else {
		LOG_DEBUG("configure_request with out client!");
	}
	// this is a request to configure a window we don't care about, pass it along to xcb
	uint16_t config_win_mask = 0;
	uint32_t config_win_vals[7];
	uint32_t client_vals[1];
	unsigned short i = 0;

	if (e->value_mask & XCB_CONFIG_WINDOW_X) {
		config_win_mask |= XCB_CONFIG_WINDOW_X;
		LOG("      x : " << e->x);
		config_win_vals[i++] = e->x;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
		config_win_mask |= XCB_CONFIG_WINDOW_Y;
		config_win_vals[i++] = e->y;
		LOG("      y : " << e->y);
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
		config_win_mask |= XCB_CONFIG_WINDOW_WIDTH;
		config_win_vals[i++] = e->width;
		LOG("  width : " << e->width);
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
		config_win_mask |= XCB_CONFIG_WINDOW_HEIGHT;
		config_win_vals[i++] = e->height;
		LOG(" height : " << e->height);
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
		config_win_mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
		config_win_vals[i++] = e->border_width;
		LOG(" border : " << e->border_width);
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
		config_win_mask |= XCB_CONFIG_WINDOW_SIBLING;
		config_win_vals[i++] = e->sibling;
		LOG("sibling : " << e->sibling);
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
		config_win_mask |= XCB_CONFIG_WINDOW_STACK_MODE;
		config_win_vals[i++] = e->stack_mode;
		client_vals[0] = e->stack_mode;
		LOG("  stack : " << (int)e->stack_mode);
	}
	if (c) {
		xcb_configure_window(Screen::conn(), c->id(), config_win_mask, config_win_vals);
		xcb_flush(Screen::conn());
	}
	if (!c) {
		xcb_configure_window(Screen::conn(), e->window, config_win_mask, config_win_vals);
		c = new Client(e->window);
		//c->debug();
		xcb_flush(Screen::conn());
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
		LOG_DEBUG("Changing stacking order for window!");
		xcb_configure_window(Screen::conn(), c->id(), XCB_CONFIG_WINDOW_STACK_MODE, client_vals);
	}
	return(0);
}

int Event::handle_property(void *p, xcb_connection_t *conn, xcb_property_notify_event_t *e)
{
	LOG_EVENT(_i << ":XCB_PROPERTY_NOTIFY");
	return(0);
}

int Event::handle_clientmessage(void *p, xcb_connection_t *conn, xcb_client_message_event_t *e)
{
	LOG_EVENT(_i << ":XCB_CLIENT_MESSAGE");
	return(0);
}

/* EVENT HANDLER WRAPPERS */

int Event::_handle_keypress(void *p, xcb_connection_t *conn, xcb_key_press_event_t *e)
{ return(_instance->handle_keypress(p, conn, e)); }

int Event::_handle_keyrelease(void *p, xcb_connection_t *conn, xcb_key_release_event_t *e)
{ return(_instance->handle_keyrelease(p, conn, e)); }

int Event::_handle_buttonpress(void *p, xcb_connection_t *conn, xcb_button_press_event_t *e)
{ return(_instance->handle_buttonpress(p, conn, e)); }

int Event::_handle_buttonrelease(void *p, xcb_connection_t *conn, xcb_button_release_event_t *e)
{ return(_instance->handle_buttonrelease(p, conn, e)); }

int Event::_handle_enter(void *p, xcb_connection_t *conn, xcb_enter_notify_event_t *e)
{ return(_instance->handle_enter(p, conn, e)); }

int Event::_handle_leave(void *p, xcb_connection_t *conn, xcb_leave_notify_event_t *e)
{ return(_instance->handle_leave(p, conn, e)); }

int Event::_handle_expose(void *p, xcb_connection_t *conn, xcb_expose_event_t *e)
{ return(_instance->handle_expose(p, conn, e)); }

int Event::_handle_create(void *p, xcb_connection_t *conn, xcb_create_notify_event_t *e)
{ return(_instance->handle_create(p, conn, e)); }

int Event::_handle_destroy(void *p, xcb_connection_t *conn, xcb_destroy_notify_event_t *e)
{ return(_instance->handle_destroy(p, conn, e)); }

int Event::_handle_unmap(void *p, xcb_connection_t *conn, xcb_unmap_notify_event_t *e)
{ return(_instance->handle_unmap(p, conn, e)); }

int Event::_handle_map(void *p, xcb_connection_t *conn, xcb_map_notify_event_t *e)
{ return(_instance->handle_map(p, conn, e)); }

int Event::_handle_maprequest(void *p, xcb_connection_t *conn, xcb_map_request_event_t *e)
{ return(_instance->handle_maprequest(p, conn, e)); }

int Event::_handle_reparent(void *p, xcb_connection_t *conn, xcb_reparent_notify_event_t *e)
{ return(_instance->handle_reparent(p, conn, e)); }

int Event::_handle_configure(void *p, xcb_connection_t *conn, xcb_configure_notify_event_t *e)
{ return(_instance->handle_configure(p, conn, e)); }

int Event::_handle_configurerequest(void *p, xcb_connection_t *conn, xcb_configure_request_event_t *e)
{ return(_instance->handle_configurerequest(p, conn, e)); }

int Event::_handle_property(void *p, xcb_connection_t *conn, xcb_property_notify_event_t *e)
{ return(_instance->handle_property(p, conn, e)); }

int Event::_handle_clientmessage(void *p, xcb_connection_t *conn, xcb_client_message_event_t *e)
{ return(_instance->handle_clientmessage(p, conn, e)); }

int Event::_handle_property_wmname(void *p, xcb_connection_t *conn, uint8_t state, xcb_window_t window, xcb_atom_t atom, xcb_get_property_reply_t *reply)
{
	LOG_EVENT("property WM_NAME");
	Client *c = Client::getByWindow(window);
	if (c) {
		char *title = xutil_get_text_property_from_reply(reply);
		c->updateTitle(xutil_get_text_property_from_reply(reply));
	}
	return(0);
}
/*
int Event::_handle_property_netwmname(void *p, xcb_connection_t *conn, uint8_t state, xcb_window_t window, xcb_atom_t atom, xcb_get_property_reply_t *reply)
{
	LOG_EVENT("_NET_WM_NAME property changed!");
	return(0);
}*/
