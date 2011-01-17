#include "log.h"
#include <sys/stat.h>

ofstream* Log::_oss = 0;
filebuf* Log::_fb = 0;
Log::Level Log::_level = Log::Debug;

extern void SignalHandler(int);

void Log::setLogFile(string file)
{
	if (_oss) {
		_oss->close();
		delete(_oss);
		_oss = 0;
	}

	if (file.size() > 0) {
		_oss = new ofstream(file.c_str(), ios::app);
		if (!_oss->is_open()) {
			_oss = 0;
			LOG_ERROR("Error opening log file (" << file.c_str() << ")");
			SignalHandler(1);
		}
	}
}

void Log::setLogLevel(string level)
{
	if (level == "info") {
		_level = Log::Info;
	} else if (level == "error") {
		_level = Log::Error;
	} else {
		_level = Log::Debug;
	}
}

Log::Level Log::logLevel()
{
	return(_level);
}

ostream& Log::stream()
{
	if (_oss) {
		return(*_oss);
	}
	return(cout);
}

bool Log::isLevelActive(Level l)
{
	switch(_level) {
		case Error: {
			if (l == Error) {
				return(true);
			} else{
				return(false);
			}
			break;
		}
		case Info: {
			if (l == Info || l == Error) {
				return(true);
			} else {
				return(false);
			}
			break;
		}
		case Debug: {
			return(true);
			break;
		}
	}
	// if _level is not set, debug is the default
	return(true);
}

void Log::flush()
{
	if (_oss) {
		_oss->flush();
	}
}

void Log::closeLogFile()
{
	if (_oss) {
		_oss->close();
		delete(_oss);
	}
}
