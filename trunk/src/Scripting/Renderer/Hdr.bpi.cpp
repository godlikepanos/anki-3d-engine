#include "ScriptingCommon.h"
#include "Hdr.h"


WRAP(Hdr)
{
	class_<Hdr, noncopyable>("Hdr", no_init)
		.def("getBlurringIterationsNum", (uint (Hdr::*)() const)(&Hdr::getBlurringIterationsNum))
		.def("setBlurringIterationsNum", &Hdr::setBlurringIterationsNum)

		.def("getExposure", (float (Hdr::*)() const)(&Hdr::getExposure))
		.def("setExposure", &Hdr::setExposure)

		.def("getBlurringDist", (float (Hdr::*)() const)(&Hdr::getBlurringDist))
		.def("setBlurringDist", &Hdr::setBlurringDist)
	;
}
