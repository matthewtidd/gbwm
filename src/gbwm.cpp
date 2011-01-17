#include <xcb/xcb.h>
#include <iostream>
//#include <xcb/xcb_keysym.h>
//#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <stdlib.h>
#include <list>
#include <exception>

#include "config.h"
#include "screen.h"
#include "client.h"
#include "event.h"
#include "theme.h"
#include "config_file.h"
#include "log.h"

using namespace std;

int setupscreen()
{
	xcb_connection_t *conn = Screen::conn();
	xcb_screen_t *screen = Screen::screen();
	
	xcb_query_tree_reply_t *reply;
	int len;
	xcb_window_t *children;
	xcb_get_window_attributes_reply_t *attr;

	reply = xcb_query_tree_reply(conn, xcb_query_tree(conn, screen->root), 0);

	if (reply == NULL) {
		return -1;
	}

	len = xcb_query_tree_children_length(reply);
	LOG_DEBUG("found " << len << " windows");
	children = xcb_query_tree_children(reply);

	for (int i = 0; i < len; i++) {
		attr = xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, children[i]), NULL);
		if (!attr) {
			LOG_ERROR("Couldn't get attributes for window " << children[i]);
			continue;
		}

		if (!attr->override_redirect && attr->map_state == XCB_MAP_STATE_VIEWABLE) {
			Client *c = new Client(children[i]);
			c->map();
		}
		free(attr);
	}
	free(reply);
	xcb_flush(conn);
	return(0);
}

void SignalHandler(int signal_number)
{
	LOG_DEBUG("Exiting with signal " << signal_number);
	Screen *screen = Screen::instance();
	if (screen) {
		bool connectionError = Screen::instance()->connectionError();
		if (!connectionError) {
			Screen::instance()->revertBackground();
		}
		list<Client *> clients = Client::clients();
		list<Client *>::iterator iter;
		for (iter = clients.begin(); iter != clients.end(); iter++) {
			Client *c = (Client *)*iter;
			c->revert();
			delete(c);
		}
		clients.clear();
		// save this so that we can clean up the Screen instance properly and then disconnect
		// otherwise the xcb connection isn't valid during ~Screen()
		xcb_connection_t *conn = Screen::conn();
		delete(Event::instance());
		delete(Screen::instance());
		if (!connectionError) {
			xcb_disconnect(conn);
		}
	}
	Log::closeLogFile();
	exit(signal_number);
}

int main(int argc, char** argv)
{
	Event *event;
	Screen *screen;

	// config file defaults
	string theme_folder;
	string log;
	string log_file;
	string config_file;
	string log_level;
	bool config_file_found = false;

	// SIGNALS
	signal(SIGALRM, SignalHandler);
	signal(SIGHUP, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGPIPE, SignalHandler);
	signal(SIGPOLL, SignalHandler);
	signal(SIGPROF, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGUSR1, SignalHandler);
	signal(SIGUSR2, SignalHandler);
	signal(SIGVTALRM, SignalHandler);
	signal(SIGSTKFLT, SignalHandler);
	signal(SIGQUIT, SignalHandler);

	Log::setLogFile(string("log.txt"));

	// load config file
	try {
		config_file = string(getenv("HOME"));
		config_file.append("/.gbwmrc");
		ConfigFile config(config_file.c_str());
		config_file_found = true;
		theme_folder = config.read<string>("theme", "../theme");
		log = config.read<string>("log", "screen");
		log_file = config.read<string>("log_file", "gbwm.log");
		log_level = config.read<string>("log_level", "debug");
	} catch (ConfigFile::file_not_found &e) {
		// set up defaults for when there isn't a config file
		LOG_DEBUG("Config file not found, loading sensible defaults");
		theme_folder = "../theme";
		log = "screen";
		log_level = "debug";
	}

	// log stuff
	Log::setLogLevel(log_level);
	if (log == "file") {
		Log::setLogFile(log_file);
	} else {
		Log::setLogFile("");
	}

	LOG_INFO("GBWM Version: " << GBWM_VERSION_MAJOR << "." << GBWM_VERSION_MINOR << "." << GBWM_VERSION_REV);

	if (config_file_found) {
		LOG_DEBUG("Config file = " << config_file.c_str());
	}

	if (log == "file") {
		LOG_DEBUG("Logging set to file");
		LOG_DEBUG("Log file = " << log_file.c_str());
	} else {
		LOG_DEBUG("Logging set to screen");
	}

	if (argc == 2) {
		screen = new Screen(argv[1]);
	} else {
		screen = new Screen(0);
	}
	if (screen->connectionError()) {
		SignalHandler(1);
	}

	new Theme(theme_folder.c_str());

	event = new Event();
	if (event->error()) {
		LOG_ERROR("Another window manager is running!");
		SignalHandler(1);
	}

	LOG_INFO("Screen size: " << Screen::screen()->width_in_pixels << "x" << Screen::screen()->height_in_pixels);
	setupscreen();
	xcb_flush(Screen::conn());

	// run the event loop
	event->loop();

	xcb_disconnect(Screen::conn());
	SignalHandler(0);

	// won't reach here
	return(0);
}

