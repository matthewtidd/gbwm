#include "screen.h"

Screen * Screen::_instance = 0;

Screen::Screen(char * dsp)
{
	_conn = 0;
	_screen = 0;
	_connection_error = false;

	_conn = xcb_connect(dsp, NULL);
	if (xcb_connection_has_error(_conn)) {
		cout << "xcb_connect error!";
		_connection_error = true;
	} else {
		_screen = xcb_setup_roots_iterator(xcb_get_setup(_conn)).data;
	}

	if (_instance == 0) {
		_instance = this;
	}
}

Screen::~Screen()
{
}

Screen* Screen::instance()
{
	return(_instance);
}

xcb_connection_t* Screen::connection()
{
	return(_conn);
}

xcb_screen_t* Screen::screen()
{
	return(_screen);
}

bool Screen::connectionError()
{
	return(_connection_error);
}
