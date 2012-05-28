#include "anki/event/MainRendererPpsHdrEvent.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/core/Globals.h"

namespace anki {

//==============================================================================
MainRendererPpsHdrEvent::MainRendererPpsHdrEvent(float startTime,
	float duration,
	float exposure_,
	uint blurringIterationsNum_,
	float blurringDist_)
	: Event(ET_MAIN_RENDERER_PPS_HDR, startTime, duration)
{
	finalData.exposure = exposure_;
	finalData.blurringIterationsNum = blurringIterationsNum_;
	finalData.blurringDist = blurringDist_;

	const Hdr& hdr =
		MainRendererSingleton::get().getPps().getHdr();
	originalData.exposure = hdr.getExposure();
	originalData.blurringIterationsNum = hdr.getBlurringIterationsNum();
	originalData.blurringDist = hdr.getBlurringDistance();
}

//==============================================================================
MainRendererPpsHdrEvent::MainRendererPpsHdrEvent(
	const MainRendererPpsHdrEvent& b)
	: Event(ET_MAIN_RENDERER_PPS_HDR, 0.0, 0.0)
{
	*this = b;
}

//==============================================================================
MainRendererPpsHdrEvent& MainRendererPpsHdrEvent::operator=(
	const MainRendererPpsHdrEvent& b)
{
	Event::operator=(b);
	finalData = b.finalData;
	originalData = b.originalData;
	return *this;
}

//==============================================================================
void MainRendererPpsHdrEvent::updateSp(float /*prevUpdateTime*/, float crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / getDuration(); // delta as percentage

	Hdr& hdr = MainRendererSingleton::get().getPps().getHdr();

	hdr.setExposure(interpolate(originalData.exposure, finalData.exposure, dp));

	hdr.setBlurringIterationsNum(
		interpolate(originalData.blurringIterationsNum,
		finalData.blurringIterationsNum, dp));

	hdr.setBlurringDistance(interpolate(originalData.blurringDist,
		finalData.blurringDist, dp));
}

} // end namespace
