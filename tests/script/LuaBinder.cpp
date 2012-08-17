#include "tests/framework/Framework.h"
#include "anki/script/ScriptManager.h"

ANKI_TEST(Script, LuaBinder)
{
	ScriptManager sm;

	const char* script = R"(
v = Vec2.new(1.1, 2.2)
print(v:getX())
)";

	sm.evalString(script);
}
