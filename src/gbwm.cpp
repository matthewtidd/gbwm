#include <xcb/xcb.h>
#include <iostream>
//#include <xcb/xcb_keysym.h>
//#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <stdlib.h>
#include <list>

#include "config.h"
#include "screen.h"
#include "client.h"
#include "event.h"

using namespace std;

int setupscreen()
{
	xcb_connection_t *conn = Screen::instance()->connection();
	xcb_screen_t *screen = Screen::instance()->screen();
	
	xcb_query_tree_reply_t *reply;
	xcb_query_pointer_reply_t *pointer;
	int len;
	xcb_window_t *children;
	xcb_get_window_attributes_reply_t *attr;
	uint32_t ws;

	reply = xcb_query_tree_reply(conn, xcb_query_tree(conn, screen->root), 0);

	if (reply == NULL) {
		return -1;
	}

	len = xcb_query_tree_children_length(reply);
	cout << "DEBUG: found " << len << " windows" << endl;
	children = xcb_query_tree_children(reply);

	for (int i = 0; i < len; i++) {
		attr = xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, children[i]), NULL);
		if (!attr) {
			cout << "ERROR: Couldn't get attributes for window " << children[i];
			continue;
		}

		if (!attr->override_redirect && attr->map_state == XCB_MAP_STATE_VIEWABLE) {
			new Client(children[i]);
		}
	}
	xcb_flush(conn);
	return(0);
}

void SignalHandler(int signal_number)
{
	cout << "DEBUG: signal SIGTERM" << endl;
	list<Client *> clients = Client::clients();
	list<Client *>::iterator iter;
	for (iter = clients.begin(); iter != clients.end(); iter++) {
		Client *c = (Client *)*iter;
		c->revert();
	}
	clients.clear();
	exit(1);
}

int main(int /*argc*/, char** /*argv*/)
{
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	Event *event;

	// SIGNALS
	signal(SIGALRM, SignalHandler);
	signal(SIGHUP, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGKILL, SignalHandler);
	signal(SIGPIPE, SignalHandler);
	signal(SIGPOLL, SignalHandler);
	signal(SIGPROF, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGUSR1, SignalHandler);
	signal(SIGUSR2, SignalHandler);
	signal(SIGVTALRM, SignalHandler);
	signal(SIGSTKFLT, SignalHandler);
	signal(SIGQUIT, SignalHandler);

	cout << "INFO: Version: " << GBWM_VERSION_MAJOR << "." << GBWM_VERSION_MINOR << endl;
	new Screen();
	event = new Event();

	conn = Screen::instance()->connection();
	screen = Screen::instance()->screen();

	cout << "INFO: Screen size: " << screen->width_in_pixels << "x" << screen->height_in_pixels << endl;
	setupscreen();
	xcb_flush(conn);
	xcb_generic_event_t *_event;
	while (_event = xcb_wait_for_event(conn)) {
		switch (_event->response_type & ~0x80) {
			case XCB_EXPOSE:
				cout << "EVENT: XCB_EXPOSE" << endl;
				break;
			case XCB_KEY_PRESS:
				cout << "EVENT: XCB_KEY_PRESS" << endl;
				break;
			case XCB_KEY_RELEASE:
				cout << "EVENT: XCB_KEY_RELEASE" << endl;
				break;
			case XCB_BUTTON_PRESS:
				cout << "EVENT: XCB_BUTTON_PRESS" << endl;
				break;
			case XCB_BUTTON_RELEASE:
				cout << "EVENT: XCB_BUTTON_RELEASE" << endl;
				break;
			case XCB_ENTER_NOTIFY:
				cout << "EVENT: XCB_ENTER_NOTIFY" << endl;
				break;
			case XCB_LEAVE_NOTIFY:
				cout << "EVENT: XCB_LEAVE_NOTIFY" << endl;
				break;
			case XCB_PROPERTY_NOTIFY:
				cout << "EVENT: XCB_PROPERTY_NOTIFY" << endl;
				break;
			default:
				cout << "EVENT: " << (int)(_event->response_type & ~0x80) << endl;
				break;
		}
		free(_event);
	}
	xcb_disconnect(conn);
	return(0);
}

