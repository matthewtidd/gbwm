#include "event.h"
#include "screen.h"
#include "client.h"
#include "window.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

Event* Event::_instance = 0;

Event::Event()
{
	cout << "DEBUG: Event()" << endl;
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
	}

	if (_instance == 0) {
		_instance = this;
	}
}

Event* Event::instance()
{
	return(_instance);
}

void Event::loop()
{
	xcb_generic_event_t *_event;
	while (_event = xcb_wait_for_event(Screen::conn())) {
		process(_event);
		free(_event);
	}
}

void Event::process(xcb_generic_event_t *_event)
{
	xcb_window_t win_id;
	switch (_event->response_type & ~0x80) {
		case XCB_KEY_PRESS: { // 2
			cout << _i << ":EVENT: XCB_KEY_PRESS" << endl;
			xcb_key_press_event_t *kp = (xcb_key_press_event_t *)_event;
			win_id = kp->event;
			break;
		}
		case XCB_KEY_RELEASE: { // 3
			cout << _i << ":EVENT: XCB_KEY_RELEASE" << endl;
			xcb_key_release_event_t *kr = (xcb_key_release_event_t *)_event;
			win_id = kr->event;
			break;
		}
		case XCB_BUTTON_PRESS: { // 4
			cout << _i << ":EVENT: XCB_BUTTON_PRESS" << endl;
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
			cout << _i << ":EVENT: XCB_BUTTON_RELEASE" << endl;
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
			//cout << _i << ":EVENT: XCB_ENTER_NOTIFY" << endl;
			xcb_enter_notify_event_t *en = (xcb_enter_notify_event_t *)_event;
			win_id = en->event;
			Window *window = Window::getWindowById(win_id);
			if (_buttonPressed && _buttonPressed == window) {
				_buttonPressed->mousePress(NULL);
			}
			break;
		}
		case XCB_LEAVE_NOTIFY: { // 9
			//cout << _i << ":EVENT: XCB_LEAVE_NOTIFY" << endl;
			xcb_enter_notify_event_t *le = (xcb_enter_notify_event_t *)_event;
			win_id = le->event;
			Window *window = Window::getWindowById(win_id);
			if (_buttonPressed && _buttonPressed == window) {
				_buttonPressed->mouseCancel();
			}
			break;
		}
		case XCB_EXPOSE: { // 12
			cout << _i << ":EVENT: XCB_EXPOSE" << endl;
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
			cout << _i << ":EVENT: XCB_CREATE_NOTIFY" << endl;
			xcb_create_notify_event_t *cn = (xcb_create_notify_event_t *)_event;
			Client *c = Client::getByWindow(cn->window);
			Window *win = Window::getWindowById(cn->window);
			if (c) {
				cout << "EVENT: existing client!" << endl;
				c->debug();
			} else if (win) {
				cout << "EVENT: existing window!" << endl;
				win->debug();
			} else {
				cout << "EVENT: non-existing client!" << endl;
				cout << " window : " << cn->window << endl;
				cout << " parent : " << cn->parent << endl;
				cout << "      x : " << cn->x << endl;
				cout << "      y : " << cn->y << endl;
				cout << "  width : " << cn->width << endl;
				cout << " height : " << cn->height << endl;
				cout << " border : " << cn->border_width << endl;
				cout << "   over : " << (int)cn->override_redirect << endl;
				if ((int)cn->override_redirect == 0) {
					new Client(cn->window);
				}
			}
			break;
		}
		case XCB_DESTROY_NOTIFY: { // 17
			cout << _i << ":EVENT: XCB_DESTROY_NOTIFY" << endl;
			xcb_destroy_notify_event_t *dn = (xcb_destroy_notify_event_t *)_event;
			Client *c = Client::getByWindow(dn->window);
			if (c) {
				cout << "EVENT: DESTROY client" << endl;
				Client::destroy(c);
			}
		}
		case XCB_UNMAP_NOTIFY: { // 18
			cout << _i << ":EVENT: XCB_UNMAP_NOTIFY" << endl;
			xcb_map_request_event_t *mr = (xcb_map_request_event_t *)_event;
			Client *c = Client::getByWindow(mr->window);
			if (c) {
				cout << "EVENT: UNMAP client" << endl;
				c->unmap();
			}
			break;
		}
		case XCB_MAP_NOTIFY: { // 19
			cout << _i << ":EVENT: XCB_MAP_NOTIFY" << endl;
			xcb_map_request_event_t *mr = (xcb_map_request_event_t *)_event;
			Client *c = Client::getByWindow(mr->window);
			if (c) {
				cout << "EVENT: MAP client" << endl;
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
			cout << _i << ":EVENT: XCB_MAP_REQUEST" << endl;
			// this code is for subwindows
			xcb_map_notify_event_t *mn = (xcb_map_notify_event_t *)_event;
			Client *c = Client::getByWindow(mn->window);
			if (c) {
				cout << "EVENT: MAP subwindow client" << endl;
				c->debug();
				xcb_map_window(Screen::conn(), mn->window);
				c->map();
				c->reparent();
			} else {
				cout << "EVENT: unknown map request" << endl;
				xcb_map_window(Screen::conn(), mn->window);
			}
			break;
		}
		case XCB_REPARENT_NOTIFY: { // 21
			cout << _i << ":EVENT: XCB_REPARENT_NOTIFY" << endl;
			xcb_reparent_notify_event_t *rn = (xcb_reparent_notify_event_t *)_event;
			Client *c = Client::getByWindow(rn->window);
			if (c) {
				cout << "EVENT: REPARENT client" << endl;
				c->reparent();
			}
			break;
		}
		case XCB_CONFIGURE_NOTIFY: { // 22
			cout << _i << ":EVENT: XCB_CONFIGURE_NOTIFY" << endl;
			xcb_configure_notify_event_t *cn = (xcb_configure_notify_event_t *)_event;
			// only handle configure notify requests for root windows
			cout << "  event : " << cn->event << endl;
			cout << " window : " << cn->window << endl;
			cout << "sibling : " << cn->above_sibling << endl;
			cout << "      x : " << cn->x << endl;
			cout << "      y : " << cn->y << endl;
			cout << "  width : " << cn->width << endl;
			cout << " height : " << cn->height << endl;
			cout << " border : " << cn->border_width << endl;
			cout << "   over : " << (int)cn->override_redirect << endl;
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
			cout << _i << ":EVENT: XCB_CONFIGURE_REQUEST" << endl;
			xcb_configure_request_event_t *cr = (xcb_configure_request_event_t *)_event;
			cout << " window : " << cr->window << endl;
			// configure requests for clients are handled separately
			Client *c = Client::getByWindow(cr->window);
			Window *win = Window::getWindowById(cr->window);
			if (c) {
				c->debug();
				// reconfigure the window
			} else if (win) {
				win->debug();
			} else {
				cout << "DEBUG: configure_request with out client!" << endl;
			}
			// this is a request to configure a window we don't care about, pass it along to xcb
			uint16_t config_win_mask = 0;
			uint32_t config_win_vals[7];
			uint32_t client_vals[1];
			unsigned short i = 0;

			if (cr->value_mask & XCB_CONFIG_WINDOW_X) {
				config_win_mask |= XCB_CONFIG_WINDOW_X;
				cout << "      x : " << cr->x << endl;
				config_win_vals[i++] = cr->x;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_Y) {
				config_win_mask |= XCB_CONFIG_WINDOW_Y;
				config_win_vals[i++] = cr->y;
				cout << "      y : " << cr->y << endl;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
				config_win_mask |= XCB_CONFIG_WINDOW_WIDTH;
				config_win_vals[i++] = cr->width;
				cout << "  width : " << cr->width << endl;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
				config_win_mask |= XCB_CONFIG_WINDOW_HEIGHT;
				config_win_vals[i++] = cr->height;
				cout << " height : " << cr->height << endl;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
				config_win_mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
				config_win_vals[i++] = cr->border_width;
				cout << " border : " << cr->border_width << endl;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
				config_win_mask |= XCB_CONFIG_WINDOW_SIBLING;
				config_win_vals[i++] = cr->sibling;
				cout << "sibling : " << cr->sibling << endl;
			}
			if (cr->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
				config_win_mask |= XCB_CONFIG_WINDOW_STACK_MODE;
				config_win_vals[i++] = cr->stack_mode;
				client_vals[0] = cr->stack_mode;
				cout << "  stack : " << (int)cr->stack_mode << endl;
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
				cout << "DEBUG: Changing stacking order for window!" << endl;
				xcb_configure_window(Screen::conn(), c->id(), XCB_CONFIG_WINDOW_STACK_MODE, client_vals);
			}
			break;
		}
		case XCB_PROPERTY_NOTIFY: { // 28
			cout << _i << ":EVENT: XCB_PROPERTY_NOTIFY" << endl;
			break;
		}
		case XCB_CLIENT_MESSAGE: { // 33
			cout << _i << ":EVENT: XCB_CLIENT_MESSAGE" << endl;
			break;
		}
		default:
			cout << _i << ":EVENT: " << (int)(_event->response_type & ~0x80) << endl;
			break;
	}
	if (win_id == Screen::screen()->root) {
		//cout << "DEBUG: ROOT WINDOW!" << endl;
	}
	Window *window = Window::getWindowById(win_id);
	if (window) {
		//window->debug();
	}
	_i = _i++;
}

bool Event::error()
{
	return(_error);
}
