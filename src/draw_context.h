
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

static xcb_visualtype_t * draw_screen_default_visual(xcb_screen_t *s)
{
    if(!s)
        return NULL;

    xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(s);

    if(depth_iter.data)
        for(; depth_iter.rem; xcb_depth_next (&depth_iter))
            for(xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator (depth_iter.data);
                 visual_iter.rem; xcb_visualtype_next (&visual_iter))
                if(s->root_visual == visual_iter.data->visual_id)
                    return visual_iter.data;

    return NULL;
}

#endif // __DRAW_H__
