#include "event.h"
#include "screen.h"
#include "client.h"
#include <iostream>

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
