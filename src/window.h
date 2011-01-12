#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>
#include <list>

using namespace std;

enum WindowType {
	WINDOW_TYPE_NONE,
	WINDOW_TYPE_BUTTON
};

class Window {
	public:
		Window(Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values);
		~Window();

		static Window * getWindowById(xcb_window_t window);
		static Window * getRootWindowById(xcb_window_t window);

		xcb_window_t id() const;
		int width() const;
		int height() const;

		virtual void mousePress(xcb_button_press_event_t *event);
		virtual void mouseRelease(xcb_button_release_event_t *event);
		virtual void mouseCancel();

		void debug() const;

	protected:
		WindowType _type;

	private:
		xcb_window_t _id;
		Window *_parent;
		int _x;
		int _y;
		unsigned int _width;
		unsigned int _height;

		static list<Window *> _windows;
		static list<Window *> _rootWindows;
};

#endif // __WINDOW_H__
