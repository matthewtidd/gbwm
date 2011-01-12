#include "button.h"
#include "screen.h"

Button::Button(Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values) :
	Window(parent, x, y, w, h, border, mask, values)
{
	_type = WINDOW_TYPE_BUTTON;

	_button = cairo_image_surface_create_from_png("../theme/close.png");
	_pressed = cairo_image_surface_create_from_png("../theme/close_down.png");

	mouseCancel();
}

Button::~Button()
{
}

void Button::mousePress(xcb_button_press_event_t * /*event*/)
{
	cout << "button pressed!!" << endl;
	cairo_surface_t *_surface = cairo_xcb_surface_create(Screen::conn(), id(), Screen::visual(), width(), height());
	cairo_t *cr = cairo_create(_surface);
	cairo_set_source_surface(cr, _pressed, 0, 0);
	cairo_paint(cr);
	xcb_flush(Screen::conn());
}

void Button::mouseRelease(xcb_button_release_event_t * /*event*/)
{
	cout << "button release!!" << endl;
	mouseCancel();
}

void Button::mouseCancel()
{
	cout << "button cancelled!!" << endl;
	cairo_surface_t *_surface = cairo_xcb_surface_create(Screen::conn(), id(), Screen::visual(), width(), height());
	cairo_t *cr = cairo_create(_surface);
	cairo_set_source_surface(cr, _button, 0, 0);
	cairo_paint(cr);
	xcb_flush(Screen::conn());
}
