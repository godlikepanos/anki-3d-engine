// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Script/LuaBinder.h>

namespace anki {

/// @addtogroup script
/// @{

/// A sandboxed LUA environment.
class ScriptEnvironment
{
public:
	/// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		LuaBinder::exposeVariable<T>(m_thread.getLuaState(), name, y);
	}

	/// Evaluate a string
	Error evalString(const CString& str)
	{
		return LuaBinder::evalString(m_thread.getLuaState(), str);
	}

	void serializeGlobals(LuaBinderSerializeGlobalsCallback& callback)
	{
		LuaBinder::serializeGlobals(m_thread.getLuaState(), callback);
	}

	void deserializeGlobals(const void* data, PtrSize dataSize)
	{
		LuaBinder::deserializeGlobals(m_thread.getLuaState(), data, dataSize);
	}

	lua_State& getLuaState()
	{
		return *m_thread.getLuaState();
	}

private:
	LuaBinder m_thread;
};
/// @}

} // end namespace anki
