// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Script/LuaBinder.h>

namespace anki {

// A sandboxed LUA environment.
class ScriptEnvironment
{
	friend class ScriptManager;

public:
	ANKI_NON_COPYABLE(ScriptEnvironment)
	ANKI_FRIEND_CALL_CONSTRUCTOR_AND_DESTRUCTOR

	// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		LuaBinder::exposeVariable<T>(m_thread.getLuaState(), name, y);
	}

	// Evaluate a string
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

	U32 getUuid() const
	{
		return m_uuid;
	}

	I32 retain() const
	{
		return m_refcount.fetchAdd(1) + 1;
	}

	void release()
	{
		if(m_refcount.fetchSub(1) == 1)
		{
			deleteInstance(ScriptMemoryPool::getSingleton(), this);
		}
	}

private:
	LuaBinder m_thread;
	U32 m_uuid;
	mutable Atomic<I32> m_refcount = {0};

	ScriptEnvironment(U32 uuid)
		: m_uuid(uuid)
	{
		ANKI_ASSERT(m_uuid > 0);
	}

	~ScriptEnvironment() = default;
};

} // end namespace anki
