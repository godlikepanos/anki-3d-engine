// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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

/// XXX
static int getSceneGraph(lua_State* l)
{
	LuaBinder* binder = reinterpret_cast<LuaBinder*>(lua_getuserdata(l));
	ANKI_ASSERT(binder != nullptr);

	ScriptManager* scriptManager = 
		reinterpret_cast<ScriptManager*>(binder->_getParent());

	ANKI_ASSERT(scriptManager != nullptr);

	detail::UserData* d = reinterpret_cast<detail::UserData*>(
		lua_newuserdata(l, sizeof(detail::UserData)));
	luaL_setmetatable(l, "SceneGraph");

	d->m_ptr = &scriptManager->_getSceneGraph();
	d->m_gc = false;

	return 1;
}

// Common anki functions
ANKI_SCRIPT_WRAP(Anki)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, Anki)
		detail::pushCFunctionStatic(lb._getLuaState(), "Anki",
			"userDataValid", &userDataValid);
		detail::pushCFunctionStatic(lb._getLuaState(), "Anki",
			"getSceneGraph", &getSceneGraph);
	ANKI_LUA_CLASS_END()
}

//==============================================================================
ScriptManager::ScriptManager(HeapAllocator<U8>& alloc, SceneGraph* scene)
:	LuaBinder(alloc, this),
	m_scene(scene)
{
	ANKI_LOGI("Initializing scripting engine...");

	// Global functions
	ANKI_SCRIPT_CALL_WRAP(Anki);

	// Util
	ANKI_SCRIPT_CALL_WRAP(CString);

	// Math
	ANKI_SCRIPT_CALL_WRAP(Vec2);
	ANKI_SCRIPT_CALL_WRAP(Vec3);
	ANKI_SCRIPT_CALL_WRAP(Vec4);
	ANKI_SCRIPT_CALL_WRAP(Mat3);
	ANKI_SCRIPT_CALL_WRAP(Mat3x4);

	// Renderer
	ANKI_SCRIPT_CALL_WRAP(Dbg);
	ANKI_SCRIPT_CALL_WRAP(MainRenderer);

	// Scene
	ANKI_SCRIPT_CALL_WRAP(MoveComponent);
	ANKI_SCRIPT_CALL_WRAP(SceneNode);
	ANKI_SCRIPT_CALL_WRAP(ModelNode);
	ANKI_SCRIPT_CALL_WRAP(InstanceNode);
	ANKI_SCRIPT_CALL_WRAP(SceneGraph);

	ANKI_LOGI("Scripting engine initialized");
}

//==============================================================================
ScriptManager::~ScriptManager()
{
	ANKI_LOGI("Destroying scripting engine...");
}

} // end namespace anki