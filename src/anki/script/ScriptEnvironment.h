// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/script/LuaBinder.h>

namespace anki
{

/// @addtogroup script
/// @{

/// A sandboxed LUA environment.
class ScriptEnvironment
{
public:
	ScriptEnvironment()
	{
	}

	~ScriptEnvironment()
	{
	}

	Error init(ScriptManager* manager);

	/// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		ANKI_ASSERT(isCreated());
		LuaBinder::exposeVariable<T>(m_thread.getLuaState(), name, y);
	}

	/// Evaluate a string
	ANKI_USE_RESULT Error evalString(const CString& str)
	{
		ANKI_ASSERT(isCreated());
		return LuaBinder::evalString(m_thread.getLuaState(), str);
	}

	void serializeGlobals(LuaBinderSerializeGlobalsCallback& callback)
	{
		ANKI_ASSERT(isCreated());
		LuaBinder::serializeGlobals(m_thread.getLuaState(), callback);
	}

	void deserializeGlobals(const void* data, PtrSize dataSize)
	{
		ANKI_ASSERT(isCreated());
		LuaBinder::deserializeGlobals(m_thread.getLuaState(), data, dataSize);
	}

	lua_State& getLuaState()
	{
		ANKI_ASSERT(isCreated());
		return *m_thread.getLuaState();
	}

private:
	ScriptManager* m_manager = nullptr;
	LuaBinder m_thread;

	Bool isCreated() const
	{
		return m_manager != nullptr;
	}
};
/// @}

} // end namespace anki