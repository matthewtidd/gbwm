#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <iostream>
#include "window.h"

using namespace std;

enum ButtonState {
	BUTTON_PRESSED,
	BUTTON_HELD,
	BUTTON_UNPRESSED
};

enum ButtonType {
	BUTTON_CLOSE,
	BUTTON_MINIMIZE,
	BUTTON_MAXIMIZE
};

class Button : public Window {
	public:
		Button(ButtonType type, Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values);
		~Button();

		virtual void draw();
		virtual void mousePress(xcb_button_press_event_t *event);
		virtual void mouseRelease(xcb_button_release_event_t *event);
		virtual void mouseCancel();

	private:
		ButtonType _buttonType;
		bool _active;
		cairo_surface_t *_button;
		cairo_surface_t *_pressed;
		cairo_surface_t *_surface;
		Window * _window;
		ButtonState _state;
};

#endif // __BUTTON_H__
