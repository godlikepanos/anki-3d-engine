// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/MainRendererPpsHdrEvent.h>
#include <anki/renderer/MainRenderer.h>

namespace anki
{

#if 0


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
	originalData.blurringIterationsNum = hdr.getBlurringIterationsCount();
	//originalData.blurringDist = hdr.getBlurringDistance();
}


MainRendererPpsHdrEvent::MainRendererPpsHdrEvent(
	const MainRendererPpsHdrEvent& b)
	: Event(ET_MAIN_RENDERER_PPS_HDR, 0.0, 0.0)
{
	*this = b;
}


MainRendererPpsHdrEvent& MainRendererPpsHdrEvent::operator=(
	const MainRendererPpsHdrEvent& b)
{
	Event::operator=(b);
	finalData = b.finalData;
	originalData = b.originalData;
	return *this;
}


void MainRendererPpsHdrEvent::updateSp(float /*prevUpdateTime*/, float crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / getDuration(); // delta as percentage

	Hdr& hdr = MainRendererSingleton::get().getPps().getHdr();

	hdr.setExposure(interpolate(originalData.exposure, finalData.exposure, dp));

	hdr.setBlurringIterationsCount(
		interpolate(originalData.blurringIterationsNum,
		finalData.blurringIterationsNum, dp));

	/*hdr.setBlurringDistance(interpolate(originalData.blurringDist,
		finalData.blurringDist, dp));*/
}

#endif

} // end namespace
