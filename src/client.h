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
#include "window.h"
#include "button.h"
#include "titlebar.h"

using namespace std;

#define CLIENT_TITLEBAR_FONT "Sans 8"
#define CLIENT_TITLEBAR_HEIGHT 16

class Client {
	public:
		Client(xcb_window_t win);
		~Client();

		void debug();
		static int count();
		static list<Client *> clients();
		static Client * getByWindow(xcb_window_t window);
		static void destroy(Client *client);

		void revert();
		void map();
		void unmap();
		void reparent();  // used to set the reparented flag from an X event after we called reparent

		xcb_window_t window() const;

	private:
		bool _mapped;
		bool _reparented;
		xcb_drawable_t _id;
		uint32_t _x;
		uint32_t _y;
		uint32_t _width;
		uint32_t _height;
		uint32_t _min_width;		// hints from application
		uint32_t _min_height;		// ...
		uint32_t _max_width;		// ...
		uint32_t _max_height;		// ...
		Titlebar *_titlebar;
		Window *_frame;
		Button *_closeButton;
		string _title;

		void setupTitlebar();
		void setupFrame();
		void drawText(const char * str, Window *win, int x, int y, int w, int h);
		static list<Client*> _clients;
		static xcb_visualtype_t *_visual;
		static xcb_connection_t *_conn;
		static xcb_screen_t *_screen;

};

#endif // __CLIENT_H__
