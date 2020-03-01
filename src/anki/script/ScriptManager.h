// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/script/LuaBinder.h>

namespace anki
{

// Forward
class SceneGraph;
class MainRenderer;

/// @addtogroup script
/// @{

/// The scripting manager.
class ScriptManager
{
public:
	ScriptManager();
	~ScriptManager();

	/// Create the script manager.
	ANKI_USE_RESULT Error init(AllocAlignedCallback allocCb, void* allocCbData);

	void setRenderer(MainRenderer* renderer)
	{
		m_otherSystems.m_renderer = renderer;
	}

	void setSceneGraph(SceneGraph* scene)
	{
		m_otherSystems.m_sceneGraph = scene;
	}

	LuaBinderOtherSystems& getOtherSystems()
	{
		return m_otherSystems;
	}

	/// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		LockGuard<Mutex> lock(n_luaMtx);
		LuaBinder::exposeVariable<T>(m_lua.getLuaState(), name, y);
	}

	/// Evaluate a string
	ANKI_USE_RESULT Error evalString(const CString& str)
	{
		LockGuard<Mutex> lock(n_luaMtx);
		return LuaBinder::evalString(m_lua.getLuaState(), str);
	}

	ANKI_INTERNAL LuaBinder& getLuaBinder()
	{
		return m_lua;
	}

	ANKI_INTERNAL ScriptAllocator getAllocator() const
	{
		return m_alloc;
	}

private:
	LuaBinderOtherSystems m_otherSystems;
	ScriptAllocator m_alloc;
	LuaBinder m_lua;
	Mutex n_luaMtx;
};
/// @}

} // end namespace anki
