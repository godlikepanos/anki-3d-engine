// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	Vec3 v3(1.1f, 2.2f, 3.3f);

	sm.exposeVariable("v4", &v4);
	sm.exposeVariable("v3", &v3);

	ANKI_TEST_EXPECT_NO_ERR(sm.evalString(script));

	ANKI_TEST_EXPECT_EQ(v4, Vec4(6, 12, 0, 5.5));
	ANKI_TEST_EXPECT_EQ(v3, Vec3(1.1f, 2.2f, 0.1f));
}

ANKI_TEST(Script, LuaBinderThreads)
{
	ScriptManager sm;
	ANKI_TEST_EXPECT_NO_ERR(sm.init(allocAligned, nullptr));

	ScriptEnvironment env;
	ANKI_TEST_EXPECT_NO_ERR(env.init(&sm));

	static const char* script = R"(
vec = Vec4.new(0, 0, 0, 0)

function myFunc()
	vec:setX(vec:getX() + 1)
	logi(string.format("The number is %f", vec:getX()))
end
)";

	ANKI_TEST_EXPECT_NO_ERR(env.evalString(script));

	static const char* script1 = R"(
myFunc()
)";

	ANKI_TEST_EXPECT_NO_ERR(env.evalString(script1));
	ANKI_TEST_EXPECT_NO_ERR(env.evalString(script1));
}

ANKI_TEST(Script, LuaBinderSerialize)
{
	ScriptManager sm;
	ANKI_TEST_EXPECT_NO_ERR(sm.init(allocAligned, nullptr));

	ScriptEnvironment env;
	ANKI_TEST_EXPECT_NO_ERR(env.init(&sm));

	static const char* script = R"(
num = 123.4
str = "lala"
vec = Vec3.new(1, 2, 3)
)";

	ANKI_TEST_EXPECT_NO_ERR(env.evalString(script));

	class Callback : public LuaBinderSerializeGlobalsCallback
	{
	public:
		Array<U8, 1024> m_buff;
		U32 m_offset = 0;

		void write(const void* data, PtrSize dataSize)
		{
			memcpy(&m_buff[m_offset], data, dataSize);
			m_offset += U32(dataSize);
		}
	} callback;

	env.serializeGlobals(callback);

	ScriptEnvironment env2;
	ANKI_TEST_EXPECT_NO_ERR(env2.init(&sm));

	env2.deserializeGlobals(&callback.m_buff[0], callback.m_offset);

	static const char* script2 = R"(
print(num)
print(str)
print(vec:getX(), vec:getY(), vec:getZ())
)";

	ANKI_TEST_EXPECT_NO_ERR(env2.evalString(script2));
}
