#include "theme.h"
#include <iostream>
#include "log.h"

Theme * Theme::_instance = 0;

Theme::Theme(const char* path)
{
	_path = new string(path);
	LOG_DEBUG("Theme path = " << _path->c_str());

	// make sure our path ends with a '/'
	if (_path->at(_path->size()-1) != '/') {
		_path->append("/");
	}
	if (_instance == 0) {
		_instance = this;
	}
}

Theme::~Theme()
{
	if (_path) {
		delete(_path);
	}
}

Theme* Theme::instance()
{
	return(_instance);
}

const char * Theme::path(const char *file) const
{
	string f = string(_path->c_str());
	if (file) {
		f.append(file);
	}
	return(f.c_str());
}
