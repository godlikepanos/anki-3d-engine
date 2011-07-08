#include "ScriptingCommon.h"
#include "Renderer/Hdr.h"


WRAP(Hdr)
{
	class_<R::Hdr, noncopyable>("Hdr", no_init)
		.def("getBlurringIterationsNum", (uint (R::Hdr::*)() const)(
			&R::Hdr::getBlurringIterationsNum))
		.def("setBlurringIterationsNum", &R::Hdr::setBlurringIterationsNum)

		.def("getExposure", (float (R::Hdr::*)() const)(&R::Hdr::getExposure))
		.def("setExposure", &R::Hdr::setExposure)

		.def("getBlurringDist", (float (R::Hdr::*)() const)(
			&R::Hdr::getBlurringDist))
		.def("setBlurringDist", &R::Hdr::setBlurringDist)
	;
}
