#include "anki/script/ScriptManager.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/script/Common.h"

namespace anki {

//==============================================================================

/// Dummy class
class Anki
{};

/// A lua function to check if the user data are not null.
static int userDataValid(lua_State* l)
{
	detail::checkArgsCount(l, 1);
	detail::UserData* data = (detail::UserData*)lua_touserdata(l, 1);

	if(data == nullptr)
	{
		luaL_error(l, "Argument is not user data");
	}
	ANKI_ASSERT(data); // This will never happen
	Bool hasValue = data->m_ptr != nullptr;
	lua_pushunsigned(l, hasValue);
	return 1;
}

// Common anki functions
ANKI_SCRIPT_WRAP(Anki)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, Anki)
		detail::pushCFunctionStatic(lb._getLuaState(), "Anki",
			"userDataValid", &userDataValid);
	ANKI_LUA_CLASS_END()
}

//==============================================================================
void ScriptManager::init()
{
	ANKI_LOGI("Initializing scripting engine...");

	// Global functions
	ANKI_SCRIPT_CALL_WRAP(Anki);

	// Math
	ANKI_SCRIPT_CALL_WRAP(Vec2);
	ANKI_SCRIPT_CALL_WRAP(Vec3);
	ANKI_SCRIPT_CALL_WRAP(Vec4);
	ANKI_SCRIPT_CALL_WRAP(Mat3);

	// Renderer
	ANKI_SCRIPT_CALL_WRAP(Dbg);
	ANKI_SCRIPT_CALL_WRAP(MainRenderer);
	ANKI_SCRIPT_CALL_WRAP(MainRendererSingleton);

	// Scene
	ANKI_SCRIPT_CALL_WRAP(MoveComponent);
	ANKI_SCRIPT_CALL_WRAP(SceneNode);
	ANKI_SCRIPT_CALL_WRAP(ModelNode);
	ANKI_SCRIPT_CALL_WRAP(InstanceNode);
	ANKI_SCRIPT_CALL_WRAP(SceneGraph);
	ANKI_SCRIPT_CALL_WRAP(SceneGraphSingleton);

	ANKI_LOGI("Scripting engine initialized");
}

} // end namespace anki
