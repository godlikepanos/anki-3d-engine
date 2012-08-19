#include "tests/framework/Framework.h"
#include "anki/script/ScriptManager.h"
#include "anki/math/Math.h"

ANKI_TEST(Script, LuaBinder)
{
	ScriptManager sm;
	Vec2 v2(2.0, 3.0);
	Vec3 v3(1.1, 2.2, 3.3);

	sm.exposeVariable("v2", &v2);
	sm.exposeVariable("v3", &v3);

	const char* script = R"(
b = Vec2.new(3.0, 4.0)
v2:copy(v2 * b)

v3:setZ(0.1)
)";

	sm.evalString(script);

	ANKI_TEST_EXPECT_EQ(v2, Vec2(6, 12));
	ANKI_TEST_EXPECT_EQ(v3, Vec3(1.1, 2.2, 0.1));
}
