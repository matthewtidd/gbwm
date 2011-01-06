#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <iostream>
#include <list>
#include "draw_context.h"

using namespace std;

class Client {
	public:
		Client(xcb_window_t win);
		~Client();

		void debug();
		static int count();

	private:
		xcb_drawable_t _id;
		uint32_t _x;
		uint32_t _y;
		uint32_t _width;
		uint32_t _height;
		uint32_t _min_width;		// hints from application
		uint32_t _min_height;		// ...
		uint32_t _max_width;		// ...
		uint32_t _max_height;		// ...
		xcb_window_t _titlebar;
		DrawContext *_draw;

		void setupTitlebar();
		static list<Client*> _clients;

};

#endif // __CLIENT_H__
