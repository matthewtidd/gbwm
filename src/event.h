#ifndef __EVENT_H__
#define __EVENT_H__

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_atom.h>

class Event {
	public:
		Event();
		~Event();

		static Event *instance();

	private:
		static Event *_instance;

};

#endif // __EVENT_H__
