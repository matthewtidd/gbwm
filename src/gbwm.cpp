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
	xcb_connection_t *conn = Screen::conn();
	xcb_screen_t *screen = Screen::screen();
	
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
			Client *c = new Client(children[i]);
			c->map();
		}
	}
	xcb_flush(conn);
	return(0);
}

void SignalHandler(int signal_number)
{
	cout << "DEBUG: Exiting with signal " << signal_number << endl;
	if (!Screen::instance()->connectionError()) {
		Screen::instance()->revertBackground();
	}
	list<Client *> clients = Client::clients();
	list<Client *>::iterator iter;
	for (iter = clients.begin(); iter != clients.end(); iter++) {
		Client *c = (Client *)*iter;
		c->revert();
	}
	clients.clear();
	if (!Screen::instance()->connectionError()) {
		xcb_disconnect(Screen::conn());
	}
	exit(signal_number);
}

int main(int argc, char** argv)
{
	Event *event;
	Screen *screen;

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
	if (argc == 2) {
		screen = new Screen(argv[1]);
	} else {
		screen = new Screen(0);
	}
	if (screen->connectionError()) {
		SignalHandler(1);
	}
	event = new Event();
	if (event->error()) {
		cout << "ERROR: Another window manager is running!" << endl;
		SignalHandler(1);
	}

	cout << "INFO: Screen size: " << Screen::screen()->width_in_pixels << "x" << Screen::screen()->height_in_pixels << endl;
	setupscreen();
	xcb_flush(Screen::conn());

	// run the event loop
	event->loop();

	xcb_disconnect(Screen::conn());
	SignalHandler(0);

	// won't reach here
	return(0);
}

