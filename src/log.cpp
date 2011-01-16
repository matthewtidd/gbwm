#include "log.h"

ostream& Log::stream()
{
	return(cout);
}

bool Log::isLevelActive(Level)
{
	return(true);
}
