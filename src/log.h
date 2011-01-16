#ifndef __LOG_H__
#define __LOG_H__

#include <ostream>
#include <iostream>

using namespace std;

#define LOG_ERROR(M)		do { if (Log::isLevelActive(Log::Error))	(Log::stream() << "ERR: " << M << endl); } while (false)
#define LOG_INFO(M)			do { if (Log::isLevelActive(Log::Info))		(Log::stream() << "INF: " << M << endl); } while (false)
#define LOG_DEBUG(M)		do { if (Log::isLevelActive(Log::Debug))	(Log::stream() << "DBG: " << M << endl); } while (false)
#define LOG_EVENT(M)		do { if (Log::isLevelActive(Log::Debug))	(Log::stream() << "EVNT :" << M << endl); } while (false)
#define LOG(M)					do { if (Log::isLevelActive(Log::Debug))	(Log::stream() << M << endl); } while (false)

class Log {
	public:

		enum Level {
			Error,
			Info,
			Debug
		};

		static ostream& stream();
		static bool isLevelActive(Level l);
};

#endif // __LOG_H__
