#include "ScriptingCommon.h"
#include "Renderer/Bl.h"


WRAP(Bl)
{
	class_<R::Bl, noncopyable>("Bl", no_init)
		.def("isEnabled", (bool (R::Bl::*)() const)(&R::Bl::isEnabled))
		.def("setEnabled", &R::Bl::setEnabled)

		.def("getBlurringIterationsNum", (uint (R::Bl::*)() const)(
			&R::Bl::getBlurringIterationsNum))
		.def("setBlurringIterationsNum", &R::Bl::setBlurringIterationsNum)

		.def("getSideBlurFactor", (float (R::Bl::*)() const)(
			&R::Bl::getSideBlurFactor))
		.def("setSideBlurFactor", &R::Bl::setSideBlurFactor)
	;
}
