#include "Common.h"
#include "r/Bl.h"


WRAP(Bl)
{
	using namespace r;

	class_<Bl, noncopyable>("Bl", no_init)
		.def("isEnabled", (bool (Bl::*)() const)(&Bl::isEnabled))
		.def("setEnabled", &Bl::setEnabled)

		.def("getBlurringIterationsNum", (uint (Bl::*)() const)(
			&Bl::getBlurringIterationsNum))
		.def("setBlurringIterationsNum", &Bl::setBlurringIterationsNum)

		.def("getSideBlurFactor", (float (Bl::*)() const)(
			&Bl::getSideBlurFactor))
		.def("setSideBlurFactor", &Bl::setSideBlurFactor)
	;
}
