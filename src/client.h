#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <iostream>
#include <list>
#include <string>
#include "draw_context.h"

using namespace std;

#define CLIENT_TITLEBAR_FONT "Sans 8"
#define CLIENT_TITLEBAR_HEIGHT 16
#define CLIENT_WINDOW_MASK XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK

class Client {
	public:
		Client(xcb_window_t win);
		~Client();

		void debug();
		static int count();
		static list<Client *> clients();
		static Client * getByWindow(xcb_window_t window);

		void revert();

		xcb_window_t window() const;

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
		xcb_window_t _frame;
		DrawContext *_draw;
		string _title;

		xcb_event_handlers_t _evenths;
		xcb_property_handlers_t _prophs;

		void setupTitlebar();
		void setupFrame();
		void setupEvents();
		void drawText(const char * str, xcb_window_t win, int x, int y, int w, int h);
		static list<Client*> _clients;
		static xcb_visualtype_t *_visual;
		static xcb_connection_t *_conn;
		static xcb_screen_t *_screen;

};

#endif // __CLIENT_H__