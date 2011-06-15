#include "EventMainRendererPpsHdr.h"
#include "MainRenderer.h"
#include "Globals.h"


namespace Event {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MainRendererPpsHdr::MainRendererPpsHdr(uint startTime, int duration,
                                       float exposure_, uint blurringIterationsNum_, float blurringDist_):
	Event(MAIN_RENDERER_PPS_HDR, startTime, duration)
{
	finalData.exposure = exposure_;
	finalData.blurringIterationsNum = blurringIterationsNum_;
	finalData.blurringDist = blurringDist_;

	originalData.exposure = MainRendererSingleton::getInstance().getPps().getHdr().getExposure();
	originalData.blurringIterationsNum =
		MainRendererSingleton::getInstance().getPps().getHdr().getBlurringIterationsNum();
	originalData.blurringDist = MainRendererSingleton::getInstance().getPps().getHdr().getBlurringDist();
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MainRendererPpsHdr::MainRendererPpsHdr(const MainRendererPpsHdr& b):
	Event(MAIN_RENDERER_PPS_HDR, 0.0, 0.0)
{
	*this = b;
}


//======================================================================================================================
// operator=                                                                                                           =
//======================================================================================================================
MainRendererPpsHdr& MainRendererPpsHdr::operator=(const MainRendererPpsHdr& b)
{
	Event::operator=(b);
	finalData = b.finalData;
	originalData = b.originalData;
	return *this;
}


//======================================================================================================================
// updateSp                                                                                                            =
//======================================================================================================================
void MainRendererPpsHdr::updateSp(uint /*prevUpdateTime*/, uint crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / getDuration(); // delta as persentage

	MainRendererSingleton::getInstance().getPps().getHdr().setExposure(
		interpolate(originalData.exposure, finalData.exposure, dp));

	MainRendererSingleton::getInstance().getPps().getHdr().setBlurringIterationsNum(
		interpolate(originalData.blurringIterationsNum, finalData.blurringIterationsNum, dp));

	MainRendererSingleton::getInstance().getPps().getHdr().setBlurringDist(
		interpolate(originalData.blurringDist, finalData.blurringDist, dp));
}


} // end namespace
