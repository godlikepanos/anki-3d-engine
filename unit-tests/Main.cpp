#include <gtest/gtest.h>
#include "App.h"
#include "MainRenderer.h"
#include "RendererInitializer.h"


int main(int argc, char** argv)
{
	// Init app
	//AppSingleton::getInstance().init(0, NULL);

	// Tests
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
