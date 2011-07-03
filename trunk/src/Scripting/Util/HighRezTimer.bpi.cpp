#include "ScriptingCommon.h"
#include "Util/HighRezTimer.h"


WRAP(HighRezTimer)
{
	class_<HighRezTimer>("HighRezTimer")
		.def("getCrntTime", &HighRezTimer::getCrntTime)
		.staticmethod("getCrntTime")
	;
}
