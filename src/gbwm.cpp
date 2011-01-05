#include <xcb/xcb.h>
#include <iostream>
//#include <xcb/xcb_keysym.h>
//#include <xcb/xcb_atom.h>
//#include <xcb/xcb_icccm.h>

#include "config.h"

using namespace std;

int setupscreen(xcb_connection_t *conn, xcb_screen_t *screen)
{
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
	cout << "found " << len << " children";
	children = xcb_query_tree_children(reply);

	for (int i = 0; i < len; i++) {
		attr = xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, children[i]), NULL);
		if (!attr) {
			cout << "Couldn't get attributes for window " << children[i];
			continue;
		}
	}
	xcb_flush(conn);
	return(0);
}

int main(int /*argc*/, char** /*argv*/)
{
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	xcb_drawable_t root;

	cout << "Version: " << GBWM_VERSION_MAJOR << "." << GBWM_VERSION_MINOR << endl;

	conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(conn)) {
		cout << "xcb_connect error!";
		return(1);
	}

	screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	root = screen->root;

	cout << "Screen size: " << screen->width_in_pixels << "x" << screen->height_in_pixels << endl;
	cout << "Root Window: " << screen->root << endl;
	setupscreen(conn, screen);
	return(0);
}

