#include "window.h"
#include "screen.h"

list<Window *> Window::_windows;
list<Window *> Window::_rootWindows;

Window::Window(Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values)
{
	_conn = Screen::instance()->connection();
	_screen = Screen::instance()->screen();

	_x = x;
	_y = y;
	_width = w;
	_height = h;
	_parent = parent;

	_windows.push_back(this);

	xcb_window_t _parent_id;
	if (_parent) {
		_parent_id = parent->id();
	} else {
		_parent_id = _screen->root;
		_rootWindows.push_back(this);
	}
	
	_id = xcb_generate_id(_conn);
	xcb_create_window(_conn,
				XCB_COPY_FROM_PARENT,
				_id,
				_parent_id,
				_x, _y,
				_width, _height,
				border,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				_screen->root_visual,
				mask, values);
	xcb_map_window(_conn, _id);
	xcb_flush(_conn);
}

Window::~Window()
{
	xcb_destroy_window(_conn, _id);
}

xcb_window_t Window::id() const
{
	return(_id);
}
