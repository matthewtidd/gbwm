
#ifndef __DRAW_CONTEXT_H__
#define __DRAW_CONTEXT_H__

#include <xcb/xcb.h>
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>

struct DrawContext {
	xcb_pixmap_t pixmap;
	uint16_t width;
	uint16_t height;
	cairo_t *cr;
	cairo_surface_t *surface;
	/*PangoLayout *layout;
	xcolor_t fg;
	xcolor_t bg;*/
};


#endif // __DRAW_CONTEXT_H__
