#ifndef __EVENT_H__
#define __EVENT_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>
#include "window.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "screen.h"

using namespace std;

class Event {
	public:
		Event();
		~Event();

		static Event *instance();
		void loop();
		bool error();

	private:
		bool _error;
		int _i;
		Window *_buttonPressed;
		xcb_event_handlers_t _eh;
		xcb_property_handlers_t _ph;

		static Event *_instance;

		static char * getPropertyTextFromReply(xcb_get_property_reply_t *reply);

		int handle_keypress(void *p, xcb_connection_t *conn, xcb_key_press_event_t *e);
		int handle_keyrelease(void *p, xcb_connection_t *conn, xcb_key_release_event_t *e);
		int handle_buttonpress(void *p, xcb_connection_t *conn, xcb_button_press_event_t *e);
		int handle_buttonrelease(void *p, xcb_connection_t *conn, xcb_button_release_event_t *e);
		int handle_enter(void *p, xcb_connection_t *conn, xcb_enter_notify_event_t *e);
		int handle_leave(void *p, xcb_connection_t *conn, xcb_leave_notify_event_t *e);
		int handle_expose(void *p, xcb_connection_t *conn, xcb_expose_event_t *e);
		int handle_create(void *p, xcb_connection_t *conn, xcb_create_notify_event_t *e);
		int handle_destroy(void *p, xcb_connection_t *conn, xcb_destroy_notify_event_t *e);
		int handle_unmap(void *p, xcb_connection_t *conn, xcb_unmap_notify_event_t *e);
		int handle_map(void *p, xcb_connection_t *conn, xcb_map_notify_event_t *e);
		int handle_maprequest(void *p, xcb_connection_t *conn, xcb_map_request_event_t *e);
		int handle_reparent(void *p, xcb_connection_t *conn, xcb_reparent_notify_event_t *e);
		int handle_configure(void *p, xcb_connection_t *conn, xcb_configure_notify_event_t *e);
		int handle_configurerequest(void *p, xcb_connection_t *conn, xcb_configure_request_event_t *e);
		int handle_property(void *p, xcb_connection_t *conn, xcb_property_notify_event_t *e);
		int handle_clientmessage(void *p, xcb_connection_t *conn, xcb_client_message_event_t *e);

		// xcb event handlers have to be static, but we need access to the event object itself
		// these functions are wrappers to call the real ones above
		static int _handle_keypress(void *p, xcb_connection_t *conn, xcb_key_press_event_t *e);
		static int _handle_keyrelease(void *p, xcb_connection_t *conn, xcb_key_release_event_t *e);
		static int _handle_buttonpress(void *p, xcb_connection_t *conn, xcb_button_press_event_t *e);
		static int _handle_buttonrelease(void *p, xcb_connection_t *conn, xcb_button_release_event_t *e);
		static int _handle_enter(void *p, xcb_connection_t *conn, xcb_enter_notify_event_t *e);
		static int _handle_leave(void *p, xcb_connection_t *conn, xcb_leave_notify_event_t *e);
		static int _handle_expose(void *p, xcb_connection_t *conn, xcb_expose_event_t *e);
		static int _handle_create(void *p, xcb_connection_t *conn, xcb_create_notify_event_t *e);
		static int _handle_destroy(void *p, xcb_connection_t *conn, xcb_destroy_notify_event_t *e);
		static int _handle_unmap(void *p, xcb_connection_t *conn, xcb_unmap_notify_event_t *e);
		static int _handle_map(void *p, xcb_connection_t *conn, xcb_map_notify_event_t *e);
		static int _handle_maprequest(void *p, xcb_connection_t *conn, xcb_map_request_event_t *e);
		static int _handle_reparent(void *p, xcb_connection_t *conn, xcb_reparent_notify_event_t *e);
		static int _handle_configure(void *p, xcb_connection_t *conn, xcb_configure_notify_event_t *e);
		static int _handle_configurerequest(void *p, xcb_connection_t *conn, xcb_configure_request_event_t *e);
		static int _handle_property(void *p, xcb_connection_t *conn, xcb_property_notify_event_t *e);
		static int _handle_clientmessage(void *p, xcb_connection_t *conn, xcb_client_message_event_t *e);

		static int _handle_property_wmname(void *p, xcb_connection_t *conn, uint8_t state, xcb_window_t window, xcb_atom_t atom, xcb_get_property_reply_t *reply);
		//static int _handle_property_netwmname(void *p, xcb_connection_t *conn, uint8_t state, xcb_window_t window, xcb_atom_t atom, xcb_get_property_reply_t *reply);

};

#endif // __EVENT_H__
