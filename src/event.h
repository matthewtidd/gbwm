#ifndef __EVENT_H__
#define __EVENT_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>
#include "window.h"

class Event {
	public:
		Event();
		~Event();

		static Event *instance();
		void loop();
		bool error();

	private:
		bool _error;
		Window *_buttonPressed;

		void process(xcb_generic_event_t *_event);

		static Event *_instance;

};

#endif // __EVENT_H__
