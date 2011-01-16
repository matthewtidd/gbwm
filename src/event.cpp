#include "event.h"
#include "screen.h"
#include "client.h"
#include "window.h"
#include "log.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

Event* Event::_instance = 0;

extern void SignalHandler(int);

Event::Event()
{
	LOG_DEBUG("Event()");
	_error = false;
	_buttonPressed = 0;
	_event = 0;
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
	}

	if (_instance == 0) {
		_instance = this;
	}
}

Event::~Event()
{
	if (_event) {
		free(_event);
	}
}

Event* Event::instance()
{
	return(_instance);
}

void Event::loop()
{
	_event = xcb_wait_for_event(Screen::conn());
	while (_event) {
		process(_event);
		free(_event);
		_event = 0;
		_event = xcb_wait_for_event(Screen::conn());
	}
}

void Event::process(xcb_generic_event_t *_event)
{
	xcb_window_t win_id = 0;
	switch (_event->response_type & ~0x80) {
		case XCB_KEY_PRESS: { // 2
			LOG_EVENT(_i << ":XCB_KEY_PRESS");
			xcb_key_press_event_t *kp = (xcb_key_press_event_t *)_event;
			win_id = kp->event;
			break;
		}
		case XCB_KEY_RELEASE: { // 3
			LOG_EVENT(_i << ":XCB_KEY_RELEASE");
			xcb_key_release_event_t *kr = (xcb_key_release_event_t *)_event;
			win_id = kr->event;
			LOG("KEY: " << (int)kr->detail);
			// ESC exits
			if ((int)kr->detail == 9) {
				SignalHandler(0);
			}
			break;
		}
		case XCB_BUTTON_PRESS: { // 4
			LOG_EVENT(_i << ":XCB_BUTTON_PRESS");
			xcb_button_press_event_t *bp = (xcb_button_press_event_t *)_event;
			win_id = bp->event;
			Window *window = Window::getWindowById(win_id);
			if (window) {
				_buttonPressed = window;
				window->mousePress(bp);
			}
			break;
		}
		case XCB_BUTTON_RELEASE: { // 5
			LOG_EVENT(_i << ":XCB_BUTTON_RELEASE");
			xcb_button_release_event_t *br = (xcb_button_release_event_t *)_event;
			win_id = br->event;
			Window *window = Window::getWindowById(win_id);
			if (_buttonPressed) {
				if (window == _buttonPressed) {
					_buttonPressed->mouseRelease(br);
				} else {
					_buttonPressed->mouseCancel();
				}
				_buttonPressed = 0;
			}
			break;
		}
		case XCB_ENTER_NOTIFY: { // 7
			//LOG_EVENT(_i << ":XCB_ENTER_NOTIFY");
			xcb_enter_notify_event_t *en = (xcb_enter_notify_event_t *)_event;
			win_id = en->event;
			Window *window = Window::getWindowById(win_id);
			if (_buttonPressed && _buttonPressed == window) {
				_buttonPressed->mousePress(NULL);
			}
			break;
		}
		case XCB_LEAVE_NOTIFY: { // 9
			//LOG_EVENT(_i << ":XCB_LEAVE_NOTIFY");
			xcb_enter_notify_event_t *le = (xcb_enter_notify_event_t *)_event;
			win_id = le->event;
			Window *window = Window::getWindowById(win_id);
			if (_buttonPressed && _buttonPressed == window) {
				_buttonPressed->mouseCancel();
			}
			break;
		}
		case XCB_EXPOSE: { // 12
			LOG_EVENT(_i << ":XCB_EXPOSE");
			xcb_expose_event_t *ex = (xcb_expose_event_t *)_event;
			win_id = ex->window;
			Window *window = Window::getWindowById(win_id);
			if (win_id == Screen::screen()->root) {
				Screen::instance()->setupBackground();
			}
			if (window) {
				window->draw();
				window->debug();
			}
			break;
		}
		case XCB_CREATE_NOTIFY: { // 16
			LOG_EVENT(_i << ":XCB_CREATE_NOTIFY");
			xcb_create_notify_event_t *cn = (xcb_create_notify_event_t *)_event;
			Client *c = Client::getByWindow(cn->window);
			Window *win = Window::getWindowById(cn->window);
			if (c) {
				LOG("EVENT: existing client!");
				c->debug();
			} else if (win) {
				LOG("EVENT: existing window!");
				win->debug();
			} else {
				LOG("EVENT: non-existing client!");
				LOG(" window : " << cn->window);
				LOG(" parent : " << cn->parent);
				LOG("      x : " << cn->x);
				LOG("      y : " << cn->y);
				LOG("  width : " << cn->width);
				LOG(" height : " << cn->height);
				LOG(" border : " << cn->border_width);
				LOG("   over : " << (int)cn->override_redirect);
				if ((int)cn->override_redirect == 0) {
					new Client(cn->window);
				}
			}
			break;
		}
		case XCB_DESTROY_NOTIFY: { // 17
			LOG_EVENT(_i << ":XCB_DESTROY_NOTIFY");
			xcb_destroy_notify_event_t *dn = (xcb_destroy_notify_event_t *)_event;
			Client *c = Client::getByWindow(dn->window);
			if (c) {
				LOG("EVENT: DESTROY client");
				Client::destroy(c);
			}
		}
		case XCB_UNMAP_NOTIFY: { // 18
			LOG_EVENT(_i << ":XCB_UNMAP_NOTIFY");
			xcb_map_request_event_t *mr = (xcb_map_request_event_t *)_event;
			Client *c = Client::getByWindow(mr->window);
			if (c) {
				LOG("EVENT: UNMAP client");
				c->unmap();
			}
			break;
		}
		case XCB_MAP_NOTIFY: { // 19
			LOG_EVENT(_i << ":XCB_MAP_NOTIFY");
			xcb_map_request_event_t *mr = (xcb_map_request_event_t *)_event;
			Client *c = Client::getByWindow(mr->window);
			if (c) {
				LOG("EVENT: MAP client");
				c->debug();
				c->map();
			}
			Window *win = Window::getWindowById(mr->window);
			if (win) {
				win->debug();
			}
			break;
		}
		case XCB_MAP_REQUEST: { // 20
			LOG_EVENT(_i << ":XCB_MAP_REQUEST");
			// this code is for subwindows
			xcb_map_notify_event_t *mn = (xcb_map_notify_event_t *)_event;
			Client *c = Client::getByWindow(mn->window);
			if (c) {
				LOG("EVENT: MAP subwindow client");
				c->debug();
				xcb_map_window(Screen::conn(), mn->window);
				c->map();
				c->reparent();
			} else {
				LOG("EVENT: unknown map request");
				xcb_map_window(Screen::conn(), mn->window);
			}
			break;
		}
		case XCB_REPARENT_NOTIFY: { // 21
			LOG_EVENT(_i << ":XCB_REPARENT_NOTIFY");
			xcb_reparent_notify_event_t *rn = (xcb_reparent_notify_event_t *)_event;
			Client *c = Client::getByWindow(rn->window);
			if (c) {
				LOG("EVENT: REPARENT client");
				c->reparent();
			}
			break;
		}
		case XCB_CONFIGURE_NOTIFY: { // 22
			LOG_EVENT(_i << ":XCB_CONFIGURE_NOTIFY");
			xcb_configure_notify_event_t *cn = (xcb_configure_notify_event_t *)_event;
			// only handle configure notify requests for root windows
			LOG("  event : " << cn->event);
			LOG(" window : " << cn->window);
			LOG("sibling : " << cn->above_sibling);
			LOG("      x : " << cn->x);
			LOG("      y : " << cn->y);
			LOG("  width : " << cn->width);
			LOG(" height : " << cn->height);
			LOG(" border : " << cn->border_width);
			LOG("   over : " << (int)cn->override_redirect);
			Client *c = Client::getByWindow(cn->window);
			if (c) {
				c->debug();
			}
			Window *win = Window::getWindowById(cn->window);
			if (win) {
				win->debug();
			}
			if (cn->window == Screen::screen()->root) {
				if (Client::getByWindow(cn->window) == NULL) {
					new Client(cn->window);
				}
			}
			break;
		}
		case XCB_CONFIGURE_REQUEST: { // 23
			LOG_EVENT(_i << ":XCB_CONFIGURE_REQUEST");
			xcb_configure_request_event_t *cr = (xcb_configure_request_event_t *)_event;
			LOG(" window : " << cr->window);
			// configure requests for clients are handled separately
			Client *c = Client::getByWindow(cr->window);
			Window *win = Window::getWindowById(cr->window);
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

			if (cr->value_mask & XCB_CONFIG_WINDOW_X) {
				config_win_mask |= XCB_CONFIG_WINDOW_X;
				LOG("      x : " << cr->x);
				config_win_vals[i++] = cr->x;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_Y) {
				config_win_mask |= XCB_CONFIG_WINDOW_Y;
				config_win_vals[i++] = cr->y;
				LOG("      y : " << cr->y);
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
				config_win_mask |= XCB_CONFIG_WINDOW_WIDTH;
				config_win_vals[i++] = cr->width;
				LOG("  width : " << cr->width);
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
				config_win_mask |= XCB_CONFIG_WINDOW_HEIGHT;
				config_win_vals[i++] = cr->height;
				LOG(" height : " << cr->height);
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
				config_win_mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
				config_win_vals[i++] = cr->border_width;
				LOG(" border : " << cr->border_width);
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
				config_win_mask |= XCB_CONFIG_WINDOW_SIBLING;
				config_win_vals[i++] = cr->sibling;
				LOG("sibling : " << cr->sibling);
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
				config_win_mask |= XCB_CONFIG_WINDOW_STACK_MODE;
				config_win_vals[i++] = cr->stack_mode;
				client_vals[0] = cr->stack_mode;
				LOG("  stack : " << (int)cr->stack_mode);
			}
			if (c) {
				xcb_configure_window(Screen::conn(), c->id(), config_win_mask, config_win_vals);
				xcb_flush(Screen::conn());
			}
			if (!c) {
				xcb_configure_window(Screen::conn(), cr->window, config_win_mask, config_win_vals);
				c = new Client(cr->window);
				//c->debug();
				xcb_flush(Screen::conn());
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
				LOG_DEBUG("Changing stacking order for window!");
				xcb_configure_window(Screen::conn(), c->id(), XCB_CONFIG_WINDOW_STACK_MODE, client_vals);
			}
			break;
		}
		case XCB_PROPERTY_NOTIFY: { // 28
			LOG_EVENT(_i << ":XCB_PROPERTY_NOTIFY");
			break;
		}
		case XCB_CLIENT_MESSAGE: { // 33
			LOG_EVENT(_i << ":XCB_CLIENT_MESSAGE");
			break;
		}
		default:
			LOG_EVENT(_i << (int)(_event->response_type & ~0x80));
			break;
	}
	if (win_id == Screen::screen()->root) {
		//LOG_DEBUG("ROOT WINDOW!");
	}
	if (win_id != 0) {
		Window *window = Window::getWindowById(win_id);
		if (window) {
			//window->debug();
		}
	}
	_i = _i++;
}

bool Event::error()
{
	return(_error);
}
