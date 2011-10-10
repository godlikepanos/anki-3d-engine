#include <gtest/gtest.h>
#include "ScriptingEngine.h"
#include "Math.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"ls.h"


TEST(ScriptingTests, ScriptingEngine)
{
	Vec4 v;
	ScriptingEngineSingleton::get().exposeVar<Vec4>("v", &v);
	
	const char* src =
		"from Anki import *\n" 
		"v.x = 1.0\n"
		"v.y = 2.0\n"
		"v.z = 3.0\n"
		"v.w = 4.0\n"
		"v += Vec4(1.0)\n";
		
	EXPECT_NO_THROW(ScriptingEngineSingleton::get().execScript(src));
	EXPECT_EQ(v, Vec4(2.0, 3.0, 4.0, 5.0));
}
