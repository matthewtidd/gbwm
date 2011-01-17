#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <ostream>
#include <fstream>

using namespace std;

#define LOG_ERROR(M)		do { if (Log::isLevelActive(Log::Error))	(Log::stream() << "ERROR: " << M << endl); } while (false)
#define LOG_INFO(M)			do { if (Log::isLevelActive(Log::Info))		(Log::stream() << "INF: " << M << endl); } while (false)
#define LOG_DEBUG(M)		do { if (Log::isLevelActive(Log::Debug))	(Log::stream() << "DBG: " << M << endl); } while (false)
#define LOG_EVENT(M)		do { if (Log::isLevelActive(Log::Debug))	(Log::stream() << "EVNT: " << M << endl); } while (false)
#define LOG(M)					do { if (Log::isLevelActive(Log::Debug))	(Log::stream() << M << endl); } while (false)
//#define LOG_ERROR(M)		if (Log::isLevelActive(Log::Error))	{ Log::stream() << "ERROR: " << M << endl; Log::flush(); }
//#define LOG_INFO(M)			if (Log::isLevelActive(Log::Info))	{ Log::stream() << "INF: " << M << endl; Log::flush(); }
//#define LOG_DEBUG(M)		if (Log::isLevelActive(Log::Debug))	{ Log::stream() << "DBG: " << M << endl; Log::flush(); }
//#define LOG_EVENT(M)		if (Log::isLevelActive(Log::Debug))	{ Log::stream() << "EVNT :" << M << endl; Log::flush(); }
//#define LOG(M)					if (Log::isLevelActive(Log::Debug))	{ Log::stream() << M << endl; Log::flush(); }

class Log {
	public:

		enum Level {
			Error,
			Info,
			Debug
		};

		static void setLogFile(string file);
		static void setLogLevel(string level);
		static Level logLevel();
		static ostream& stream();
		static bool isLevelActive(Level l);
		static void flush();
		static void closeLogFile();

	private:
		static ofstream *_oss;
		static filebuf *_fb;
		static Level _level;
};


#endif // __LOG_H__
