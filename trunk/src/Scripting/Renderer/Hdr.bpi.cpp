#include "ScriptingCommon.h"
#include "Hdr.h"


WRAP(Hdr)
{
	//typedef ;

	class_<Hdr, noncopyable>("Hdr", no_init)
		.add_property("blurringIterationsNum", (uint (Hdr::*)() const)(&Hdr::getBlurringIterationsNum),
		              &Hdr::setBlurringIterationsNum)
		.add_property("exposure", (float (Hdr::*)() const)(&Hdr::getExposure), &Hdr::setExposure)
		.add_property("blurringDist", (float (Hdr::*)() const)(&Hdr::getBlurringDist), &Hdr::setBlurringDist)
	;
}
