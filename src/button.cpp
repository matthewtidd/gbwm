#include "button.h"
#include "screen.h"
#include "client.h"
#include "theme.h"
#include "log.h"
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>

Button::Button(Client *client, ButtonType type, Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values) :
	Window(parent, x, y, w, h, border, mask, values)
{
	_type = WINDOW_TYPE_BUTTON;
	_buttonType = type;
	_client = client;
	Theme *theme = Theme::instance();

	_button = cairo_image_surface_create_from_png(theme->path("close.png"));
	_pressed = cairo_image_surface_create_from_png(theme->path("close_down.png"));
	_surface = cairo_xcb_surface_create(Screen::conn(), id(), Screen::visual(), width(), height());

	mouseCancel();
	_active = false;
}

Button::~Button()
{
	cairo_surface_destroy(_surface);
	cairo_surface_destroy(_button);
	cairo_surface_destroy(_pressed);
}

void Button::draw()
{
	cairo_t *cr = cairo_create(_surface);
	if (_active) {
		cairo_set_source_surface(cr, _pressed, 0, 0);
	} else {
		cairo_set_source_surface(cr, _button, 0, 0);
	}
	cairo_paint(cr);
	cairo_destroy(cr);;
	xcb_flush(Screen::conn());
}

void Button::mousePress(xcb_button_press_event_t * /*event*/)
{
	_active = true;
	draw();
}

void Button::mouseRelease(xcb_button_release_event_t * /*event*/)
{
	LOG_DEBUG("button release!!");
	if (_active) {
		switch (_buttonType) {
			case BUTTON_CLOSE:
				LOG_DEBUG("Close window");
				closeAction();
				break;
			default:
				LOG_DEBUG("Unknown button");
		}
	}
	mouseCancel();
}

void Button::mouseCancel()
{
	_active = false;
	draw();
}

void Button::closeAction()
{
	xcb_get_property_cookie_t cookie;
	xcb_get_wm_protocols_reply_t protocols;
	bool use_delete = false;

	xcb_atom_t wm_protocols = xcb_atom_get(Screen::conn(), "WM_PROTOCOLS");
	xcb_atom_t wm_delete_window = xcb_atom_get(Screen::conn(), "WM_DELETE_WINDOW");

	// check if WM_DELETE is supported
	cookie = xcb_get_wm_protocols_unchecked(Screen::conn(), _client->id(), wm_protocols);
	if (xcb_get_wm_protocols_reply(Screen::conn(), cookie, &protocols, NULL) == 1) {
		for (unsigned int i = 0; i < protocols.atoms_len; i++) {
			if (protocols.atoms[i] == wm_delete_window) {
				use_delete = true;
			}
		}
	}

	if (use_delete) {
		xcb_client_message_event_t ev;
		ev.response_type = XCB_CLIENT_MESSAGE;
		ev.format = 32;
		ev.sequence = 0;
		ev.window = _client->id();
		ev.type = wm_protocols;
		ev.data.data32[0] = wm_delete_window;
		ev.data.data32[1] = XCB_CURRENT_TIME;
		xcb_send_event(Screen::conn(), false, _client->id(), XCB_EVENT_MASK_NO_EVENT, (char *)&ev);
	} else {
		xcb_kill_client(Screen::conn(), _client->id());
	}
}
