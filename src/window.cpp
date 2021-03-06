#include "window.h"
#include "screen.h"
#include "log.h"

list<Window *> Window::_windows;
list<Window *> Window::_rootWindows;

Window::Window(Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values)
{
	xcb_connection_t *_conn = Screen::conn();
	xcb_screen_t *_screen = Screen::screen();

	_mapped = false;
	_x = x;
	_y = y;
	_width = w;
	_height = h;
	_parent = parent;
	_type = WINDOW_TYPE_NONE;

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

	const uint32_t win_vals[] = {
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
	};
	xcb_change_window_attributes(Screen::conn(), _id, XCB_CW_EVENT_MASK, win_vals);
	xcb_flush(_conn);
}

Window::~Window()
{
	xcb_destroy_window(Screen::conn(), _id);
}

Window *Window::getWindowById(xcb_window_t window)
{
	list<Window *>::iterator iter;
	for (iter = _windows.begin(); iter != _windows.end(); iter++) {
		Window *w = (Window *)*iter;
		if (w->_id == window) {
			return(w);
		}
	}
	return(NULL);
}

Window *Window::getRootWindowById(xcb_window_t window)
{
	list<Window *>::iterator iter;
	for (iter = _rootWindows.begin(); iter != _rootWindows.end(); iter++) {
		Window *w = (Window *)*iter;
		if (w->_id == window) {
			return(w);
		}
	}
	return(NULL);
}

xcb_window_t Window::id() const
{
	return(_id);
}

int Window::width() const
{
	return(_width);
}

int Window::height() const
{
	return(_height);
}

void Window::draw()
{

}

void Window::map()
{
	if (!_mapped) {
		xcb_map_window(Screen::conn(), _id);
		_mapped = true;
	}
}

void Window::unmap()
{
	if (_mapped) {
		xcb_unmap_window(Screen::conn(), _id);
		_mapped = false;
	}
}

void Window::mousePress(xcb_button_press_event_t * /*event*/)
{
}

void Window::mouseRelease(xcb_button_release_event_t * /*event*/)
{
}

void Window::mouseCancel()
{
}

void Window::debug() const
{
	LOG_DEBUG("window = " << _id);
	//LOG_DEBUG("   x = " << _x);
	//LOG_DEBUG("   y = " << _y);
	//LOG_DEBUG("   width = " << _width);
	//LOG_DEBUG("   height = " << _height);
	switch(_type) {
		case WINDOW_TYPE_NONE:
			LOG_DEBUG("    type = WINDOW");
			break;
		case WINDOW_TYPE_BUTTON:
			LOG_DEBUG("    type = BUTTON");
			break;
		case WINDOW_TYPE_TITLEBAR:
			LOG_DEBUG("    type = TITLEBAR");
			break;
		default:
			LOG_DEBUG("    type = UNKNOWN");
	}
}
