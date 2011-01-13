#ifndef __TITLEBAR_H__
#define __TITLEBAR_H__

#include <string>
#include "window.h"

using namespace std;

class Titlebar : public Window {
	public:
		Titlebar(const char *text, Window *parent, int x, int y, int w, int h, int border, uint32_t mask, const uint32_t *values);
		~Titlebar();

		virtual void draw();
		void setText(const char *text);
	
	private:
		string _text;
};

#endif // __TITLEBAR_H__
