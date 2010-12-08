#include "ScriptingCommon.h"
#include "Hdr.h"


WRAP(Hdr)
{
	class_<Hdr, noncopyable>("Hdr", no_init)
		BP_PROPERTY_RW("blurringIterationsNum", Hdr::getBlurringIterationsNum, Hdr::setBlurringIterationsNum)
		BP_PROPERTY_RW("exposure", Hdr::getExposure, Hdr::setExposure)
		BP_PROPERTY_RW("blurringDist", Hdr::getBlurringDist, Hdr::setBlurringDist)
	;
}
