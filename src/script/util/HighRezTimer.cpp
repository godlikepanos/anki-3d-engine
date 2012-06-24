#include "anki/script/Common.h"
#include "anki/util/HighRezTimer.h"

ANKI_WRAP(HighRezTimer)
{
	class_<HighRezTimer>("HighRezTimer")
		.def("getCurrentTime", &HighRezTimer::getCurrentTime)
		.staticmethod("getCurrentTime")
	;
}
