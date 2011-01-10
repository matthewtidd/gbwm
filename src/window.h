#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>
#include <list>

using namespace std;

class Window {
	public:
		Window(Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values);
		~Window();

		xcb_window_t id() const;

	private:
		xcb_window_t _id;
		Window *_parent;
		int _x;
		int _y;
		unsigned int _width;
		unsigned int _height;

		xcb_connection_t *_conn;
		xcb_screen_t *_screen;

		static list<Window *> _windows;
		static list<Window *> _rootWindows;
};

#endif // __WINDOW_H__
