#include "MainRendererPpsHdrEvent.h"
#include "r/MainRenderer.h"
#include "core/Globals.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MainRendererPpsHdrEvent::MainRendererPpsHdrEvent(float startTime,
	float duration,
	float exposure_,
	uint blurringIterationsNum_,
	float blurringDist_)
:	Event(MAIN_RENDERER_PPS_HDR, startTime, duration)
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
// Copy constructor                                                            =
//==============================================================================
MainRendererPpsHdrEvent::MainRendererPpsHdrEvent(
	const MainRendererPpsHdrEvent& b)
:	Event(MAIN_RENDERER_PPS_HDR, 0.0, 0.0)
{
	*this = b;
}


//==============================================================================
// operator=                                                                   =
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
// updateSp                                                                    =
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
