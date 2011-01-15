#include "screen.h"
#include "draw_context.h"

Screen * Screen::_instance = 0;

Screen::Screen(char * dsp)
{
	_conn = 0;
	_screen = 0;
	_connection_error = false;

	_conn = xcb_connect(dsp, NULL);
	if (xcb_connection_has_error(_conn)) {
		cout << "ERROR: xcb_connect error!" << endl;
		_connection_error = true;
	} else {
		_screen = xcb_setup_roots_iterator(xcb_get_setup(_conn)).data;
		_visual = draw_screen_default_visual(_screen);
		changeCursor();
		setupBackground();
	}

	if (_instance == 0) {
		_instance = this;
	}
}

Screen::~Screen()
{
}

Screen* Screen::instance()
{
	return(_instance);
}

xcb_connection_t* Screen::conn()
{
	if (!_instance) {
		return(NULL);
	}
	return(_instance->_conn);
}

xcb_screen_t* Screen::screen()
{
	if (!_instance) {
		return(NULL);
	}
	return(_instance->_screen);
}

xcb_visualtype_t* Screen::visual()
{
	if (!_instance) {
		return(NULL);
	}
	return(_instance->_visual);
}

bool Screen::connectionError()
{
	return(_connection_error);
}

void Screen::changeCursor()
{
	int cursor_index = XC_left_ptr;
	// change the default cursor
	xcb_font_t font = xcb_generate_id(_conn);
	xcb_void_cookie_t fontCookie = xcb_open_font_checked(_conn,
						font,
						6,
						"cursor" );

	xcb_cursor_t cursor = xcb_generate_id(_conn);
	xcb_create_glyph_cursor(_conn,
						 cursor,
						 font,
						 font,
						 cursor_index,
						 cursor_index + 1,
						 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF );

	xcb_gcontext_t gc = xcb_generate_id(_conn);

	uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
	uint32_t values[3];
	values[0] = _screen->black_pixel;
	values[1] = _screen->white_pixel;
	values[2] = font;

	xcb_void_cookie_t gcCookie = xcb_create_gc_checked(_conn, gc, _screen->root, mask, values);

	uint32_t value_list = cursor;
	xcb_change_window_attributes(_conn, _screen->root, XCB_CW_CURSOR, &value_list);

	xcb_free_cursor(_conn, cursor);

	fontCookie = xcb_close_font_checked(_conn, font);
}

void Screen::setupBackground()
{
	cairo_surface_t *_surface = cairo_xcb_surface_create(_conn, _screen->root, _visual, 1024, 768);
	cairo_t *cr = cairo_create(_surface);
	cairo_rectangle(cr, 0, 0, 1024, 768);
	cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
	cairo_fill(cr);
	cairo_paint(cr);
	xcb_flush(_conn);
}

void Screen::revertBackground()
{
	cairo_surface_t *_surface = cairo_xcb_surface_create(_conn, _screen->root, _visual, 1024, 768);
	cairo_t *cr = cairo_create(_surface);
	cairo_rectangle(cr, 0, 0, 1024, 768);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_fill(cr);
	cairo_paint(cr);
	xcb_flush(_conn);
}
