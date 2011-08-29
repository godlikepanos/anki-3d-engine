#include "Common.h"
#include "util/HighRezTimer.h"


WRAP(HighRezTimer)
{
	class_<HighRezTimer>("HighRezTimer")
		.def("getCrntTime", &HighRezTimer::getCrntTime)
		.staticmethod("getCrntTime")
	;
}
