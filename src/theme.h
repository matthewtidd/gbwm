#ifndef __THEME_H__
#define __THEME_H__

#include <string>

using namespace std;

class Theme {
	public:
		Theme(const char *path);
		~Theme();

		static Theme* instance();

		const char *path(const char *file) const;

	private:
		static Theme* _instance;
		string *_path;
};

#endif // __THEME_H__
