// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/script/ScriptObject.h>
#include <anki/script/LuaBinder.h>

namespace anki
{

/// @addtogroup script
/// @{

/// A sandboxed LUA environment.
class ScriptEnvironment : public ScriptObject
{
public:
	ScriptEnvironment(ScriptManager* manager)
		: ScriptObject(manager)
	{
	}

	~ScriptEnvironment();

	Error init();

	/// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		LuaBinder::exposeVariable<T>(m_thread.m_luaState, name, y);
	}

	/// Evaluate a string
	ANKI_USE_RESULT Error evalString(const CString& str)
	{
		return LuaBinder::evalString(m_thread.m_luaState, str);
	}

	lua_State& getLuaState()
	{
		ANKI_ASSERT(m_thread.m_luaState);
		return *m_thread.m_luaState;
	}

private:
	LuaThread m_thread;
};
/// @}

} // end namespace anki