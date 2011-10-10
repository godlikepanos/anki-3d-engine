#include "anki/script/ScriptCommon.h"
#include "anki/util/HighRezTimer.h"


WRAP(HighRezTimer)
{
	class_<HighRezTimer>("HighRezTimer")
		.def("getCrntTime", &HighRezTimer::getCrntTime)
		.staticmethod("getCrntTime")
	;
}
