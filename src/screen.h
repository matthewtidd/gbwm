#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <xcb/xcb.h>
#include <iostream>

using namespace std;

class Screen {
	public:
		Screen(char * dsp);
		~Screen();

		static Screen* instance();
		static xcb_connection_t* conn();
		static xcb_screen_t* screen();
		static xcb_visualtype_t* visual();

		bool connectionError();

	private:
		xcb_connection_t* _conn;
		xcb_screen_t* _screen;
		xcb_visualtype_t *_visual;
		bool _connection_error;

		static Screen* _instance;
};

#endif // __SCREEN_H__
