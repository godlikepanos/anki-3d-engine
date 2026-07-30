// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Script.h>
#include <AnKi/Math.h>

ANKI_TEST(Script, LuaBinder)
{
	ScriptManager::allocateSingleton(allocAligned, nullptr);
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);

	{
		ScriptEnvironment env;

		Vec3 v3(1.1f, 2.2f, 3.3f);
		env.exposeVariable("v3", &v3);

		ANKI_TEST_EXPECT_NO_ERR(env.evalString(R"(
v3.x = v3.x + 10.0
)"));

		ANKI_TEST_EXPECT_EQ(v3, Vec3(11.1f, 2.2f, 3.3f));
	}

	{
		ScriptEnvironment env;

		Vec3 v3(1.1f, 2.2f, 3.3f);
		env.exposeVariable("v3", &v3);

		ANKI_TEST_EXPECT_NO_ERR(env.evalString(R"(
v3:copy(v3 + 10.0)
v3:copy(v3 + Vec3.new(20))
)"));

		ANKI_TEST_EXPECT_EQ(v3, Vec3(31.1f, 32.2f, 33.3f));
	}

	DefaultMemoryPool::freeSingleton();
	ScriptManager::freeSingleton();
}
