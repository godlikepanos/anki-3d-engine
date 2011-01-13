#include <gtest/gtest.h>
#include "App.h"
#include "MainRenderer.h"
#include "RendererInitializer.h"


int main(int argc, char** argv)
{
	// Init app
	AppSingleton::getInstance().init(argc, argv);

	// Init mainRenderer
	RendererInitializer initializer;
	initializer.ms.ez.enabled = false;
	initializer.dbg.enabled = true;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = true;
	initializer.is.sm.resolution = 512;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.25;
	initializer.pps.hdr.blurringDist = 1.0;
	initializer.pps.hdr.blurringIterationsNum = 2;
	initializer.pps.hdr.exposure = 4.0;
	initializer.pps.ssao.blurringIterationsNum = 2;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.renderingQuality = 0.5;
	initializer.mainRendererQuality = 1.0;
	MainRendererSingleton::getInstance().init(initializer);

	// Tests
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
