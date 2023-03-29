// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Script/LuaBinder.h>

namespace anki {

/// @addtogroup script
/// @{

/// The scripting manager.
class ScriptManager : public MakeSingleton<ScriptManager>
{
	template<typename>
	friend class MakeSingleton;

public:
	/// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		LockGuard<Mutex> lock(n_luaMtx);
		LuaBinder::exposeVariable<T>(m_lua.getLuaState(), name, y);
	}

	/// Evaluate a string
	Error evalString(const CString& str)
	{
		LockGuard<Mutex> lock(n_luaMtx);
		return LuaBinder::evalString(m_lua.getLuaState(), str);
	}

	ANKI_INTERNAL LuaBinder& getLuaBinder()
	{
		return m_lua;
	}

private:
	class PoolInit
	{
	public:
		PoolInit(AllocAlignedCallback allocCb, void* allocCbData);

		~PoolInit();
	};

	PoolInit m_poolInit;
	LuaBinder m_lua;
	Mutex n_luaMtx;

	ScriptManager(AllocAlignedCallback allocCb, void* allocCbData);

	~ScriptManager();
};
/// @}

} // end namespace anki
