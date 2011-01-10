#include "event.h"
#include "screen.h"
#include "client.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

Event* Event::_instance = 0;

Event::Event()
{
	cout << "DEBUG: Event()" << endl;
	xcb_connection_t *conn = Screen::instance()->connection();
	xcb_screen_t *screen = Screen::instance()->screen();

	const uint32_t win_vals[] = {
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_PROPERTY_CHANGE
	};
	xcb_change_window_attributes(conn, screen->root, XCB_CW_EVENT_MASK, win_vals);

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
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	conn = Screen::instance()->connection();
	screen = Screen::instance()->screen();

	xcb_generic_event_t *_event;
	while (_event = xcb_wait_for_event(conn)) {
		process(_event);
		free(_event);
	}
}

void Event::process(xcb_generic_event_t *_event)
{
	switch (_event->response_type & ~0x80) {
		case XCB_EXPOSE:
			cout << "EVENT: XCB_EXPOSE" << endl;
			break;
		case XCB_KEY_PRESS:
			cout << "EVENT: XCB_KEY_PRESS" << endl;
			break;
		case XCB_KEY_RELEASE:
			cout << "EVENT: XCB_KEY_RELEASE" << endl;
			break;
		case XCB_BUTTON_PRESS:
			cout << "EVENT: XCB_BUTTON_PRESS" << endl;
			break;
		case XCB_BUTTON_RELEASE:
			cout << "EVENT: XCB_BUTTON_RELEASE" << endl;
			break;
		case XCB_ENTER_NOTIFY:
			cout << "EVENT: XCB_ENTER_NOTIFY" << endl;
			break;
		case XCB_LEAVE_NOTIFY:
			cout << "EVENT: XCB_LEAVE_NOTIFY" << endl;
			break;
		case XCB_PROPERTY_NOTIFY:
			cout << "EVENT: XCB_PROPERTY_NOTIFY" << endl;
			break;
		default:
			cout << "EVENT: " << (int)(_event->response_type & ~0x80) << endl;
			break;
	}
}
