#include <xcb/xcb.h>
#include <iostream>
//#include <xcb/xcb_keysym.h>
//#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>

#include "config.h"
#include "screen.h"
#include "client.h"

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
	cout << "found " << len << " children" << endl;
	children = xcb_query_tree_children(reply);

	for (int i = 0; i < len; i++) {
		attr = xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, children[i]), NULL);
		if (!attr) {
			cout << "Couldn't get attributes for window " << children[i];
			continue;
		}

		if (!attr->override_redirect && attr->map_state == XCB_MAP_STATE_VIEWABLE) {
			new Client(children[i]);
		}
	}
	xcb_flush(conn);
	return(0);
}

int main(int /*argc*/, char** /*argv*/)
{
	xcb_connection_t *conn;
	xcb_drawable_t root;
	xcb_screen_t *screen;

	cout << "Version: " << GBWM_VERSION_MAJOR << "." << GBWM_VERSION_MINOR << endl;
	new Screen();

	conn = Screen::instance()->connection();
	screen = Screen::instance()->screen();
	root = screen->root;

	cout << "Screen size: " << screen->width_in_pixels << "x" << screen->height_in_pixels << endl;
	cout << "Root Window: " << root << endl;
	setupscreen();
	cout << "Clients: " << Client::count() << endl;
/*
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	xcb_window_t new_win = xcb_generate_id(conn);
	xcb_create_window(conn,
				XCB_COPY_FROM_PARENT,
				new_win,
				root,
				50, 400,
				100, 100,
				10,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				screen->root_visual,
				mask, values);
	xcb_map_window(conn, new_win);
	*/
	xcb_flush(conn);
	for(;;) {
		sleep(5);
		// do nothing
	}
	xcb_disconnect(conn);
	return(0);
}

