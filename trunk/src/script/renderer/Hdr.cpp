#include "ScriptCommon.h"
#include "renderer/Hdr.h"


WRAP(Hdr)
{
	class_<Hdr, noncopyable>("Hdr", no_init)
		.def("getBlurringIterationsNum", (uint (Hdr::*)() const)(
			&Hdr::getBlurringIterationsNum))
		.def("setBlurringIterationsNum", &Hdr::setBlurringIterationsNum)

		.def("getExposure", (float (Hdr::*)() const)(&Hdr::getExposure))
		.def("setExposure", &Hdr::setExposure)

		.def("getBlurringDistance", (float (Hdr::*)() const)(
			&Hdr::getBlurringDistance))
		.def("setBlurringDistance", &Hdr::setBlurringDistance)
	;
}
