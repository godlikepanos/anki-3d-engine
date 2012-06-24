#include "anki/script/Common.h"
#include "anki/renderer/Bl.h"

ANKI_WRAP(Bl)
{
	class_<Bl, noncopyable>("Bl", no_init)
		.def("getEnabled", (bool (Bl::*)() const)(&Bl::getEnabled))
		.def("setEnabled", &Bl::setEnabled)

		.def("getBlurringIterationsNum", (uint (Bl::*)() const)(
			&Bl::getBlurringIterationsNum))
		.def("setBlurringIterationsNum", &Bl::setBlurringIterationsNum)

		.def("getSideBlurFactor", (float (Bl::*)() const)(
			&Bl::getSideBlurFactor))
		.def("setSideBlurFactor", &Bl::setSideBlurFactor)
	;
}
