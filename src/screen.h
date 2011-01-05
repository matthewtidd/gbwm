#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <xcb/xcb.h>
#include <iostream>

using namespace std;

class Screen {
	public:
		Screen();
		~Screen();

		static Screen* instance();

		xcb_connection_t* connection();
		xcb_screen_t* screen();
		bool connectionError();

	private:
		xcb_connection_t* _conn;
		xcb_screen_t* _screen;
		bool _connection_error;

		static Screen* _instance;
};

#endif // __SCREEN_H__
