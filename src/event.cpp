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
	_buttonPressed = 0;

	const uint32_t win_vals[] = {
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_PROPERTY_CHANGE
	};
	xcb_change_window_attributes(Screen::conn(), Screen::screen()->root, XCB_CW_EVENT_MASK, win_vals);

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
		case XCB_EXPOSE: {
			cout << "EVENT: XCB_EXPOSE" << endl;
			xcb_expose_event_t *ex = (xcb_expose_event_t *)_event;
			win_id = ex->window;
			break;
		}
		case XCB_KEY_PRESS: {
			cout << "EVENT: XCB_KEY_PRESS" << endl;
			xcb_key_press_event_t *kp = (xcb_key_press_event_t *)_event;
			win_id = kp->event;
			break;
		}
		case XCB_KEY_RELEASE: {
			cout << "EVENT: XCB_KEY_RELEASE" << endl;
			xcb_key_release_event_t *kr = (xcb_key_release_event_t *)_event;
			win_id = kr->event;
			break;
		}
		case XCB_BUTTON_PRESS: {
			cout << "EVENT: XCB_BUTTON_PRESS" << endl;
			xcb_button_press_event_t *bp = (xcb_button_press_event_t *)_event;
			win_id = bp->event;
			Window *window = Window::getWindowById(win_id);
			if (window) {
				_buttonPressed = window;
				window->mousePress(bp);
			}
			break;
		}
		case XCB_BUTTON_RELEASE: {
			cout << "EVENT: XCB_BUTTON_RELEASE" << endl;
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
		case XCB_ENTER_NOTIFY: {
			cout << "EVENT: XCB_ENTER_NOTIFY" << endl;
			xcb_enter_notify_event_t *en = (xcb_enter_notify_event_t *)_event;
			//win_id = en->event;
			break;
		}
		case XCB_LEAVE_NOTIFY: {
			cout << "EVENT: XCB_LEAVE_NOTIFY" << endl;
			xcb_enter_notify_event_t *le = (xcb_enter_notify_event_t *)_event;
			//win_id = le->event;
			break;
		}
		case XCB_PROPERTY_NOTIFY:
			cout << "EVENT: XCB_PROPERTY_NOTIFY" << endl;
			break;
		default:
			cout << "EVENT: " << (int)(_event->response_type & ~0x80) << endl;
			break;
	}
	if (win_id == Screen::screen()->root) {
		cout << "DEBUG: ROOT WINDOW!" << endl;
	}
	Window *window = Window::getWindowById(win_id);
	if (window) {
		window->debug();
	}
}
