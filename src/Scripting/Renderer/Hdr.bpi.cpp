#include "ScriptingCommon.h"
#include "Hdr.h"


WRAP(Hdr)
{
	class_<Hdr, noncopyable>("Hdr", no_init)
		BP_PROPERTY_BASIC_TYPE(uint, Hdr, "blurringIterationsNum", getBlurringIterationsNum, setBlurringIterationsNum)
		BP_PROPERTY_BASIC_TYPE(float, Hdr, "exposure", getExposure, setExposure)
		BP_PROPERTY_BASIC_TYPE(float, Hdr, "blurringDist", getBlurringDist, setBlurringDist)
	;
}
