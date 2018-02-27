// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/Script.h>
#include <anki/Math.h>

static const char* script = R"(
b = Vec4.new(0, 0, 0, 1.1)
if math.abs(b:getW() - 1.1) > 0.00001 then
	print(b:getW())
	error("oh no!!")
end

b:setX(3.0)
b:setY(4.0)

v4:copy(v4 * b)

v3:setZ(0.1)
)";

ANKI_TEST(Script, LuaBinder)
{
	ScriptManager sm;

	ANKI_TEST_EXPECT_NO_ERR(sm.init(allocAligned, nullptr));
	Vec4 v4(2.0, 3.0, 4.0, 5.0);
	Vec3 v3(1.1, 2.2, 3.3);

	sm.exposeVariable("v4", &v4);
	sm.exposeVariable("v3", &v3);

	ANKI_TEST_EXPECT_NO_ERR(sm.evalString(script));

	ANKI_TEST_EXPECT_EQ(v4, Vec4(6, 12, 0, 5.5));
	ANKI_TEST_EXPECT_EQ(v3, Vec3(1.1, 2.2, 0.1));
}

ANKI_TEST(Script, LuaBinderThreads)
{
	ScriptManager sm;
	ANKI_TEST_EXPECT_NO_ERR(sm.init(allocAligned, nullptr));

	ScriptEnvironmentPtr env;
	ANKI_TEST_EXPECT_NO_ERR(sm.newScriptEnvironment(env));

	static const char* script = R"(
vec = Vec4.new(0, 0, 0, 0)
	
function myFunc()
	vec:setX(vec:getX() + 1)
	logi(string.format("The number is %f", vec:getX()))
end
)";

	ANKI_TEST_EXPECT_NO_ERR(env->evalString(script));

	static const char* script1 = R"(
myFunc()
)";

	ANKI_TEST_EXPECT_NO_ERR(env->evalString(script1));
	ANKI_TEST_EXPECT_NO_ERR(env->evalString(script1));
}
