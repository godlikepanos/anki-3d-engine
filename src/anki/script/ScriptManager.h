// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
		m_r = renderer;
	}

	void setSceneGraph(SceneGraph* scene)
	{
		m_scene = scene;
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

	ANKI_USE_RESULT Error newScriptEnvironment(ScriptEnvironmentPtr& out);

anki_internal:
	SceneGraph& getSceneGraph()
	{
		ANKI_ASSERT(m_scene);
		return *m_scene;
	}

	MainRenderer& getMainRenderer()
	{
		ANKI_ASSERT(m_r);
		return *m_r;
	}

	LuaBinder& getLuaBinder()
	{
		return m_lua;
	}

	ScriptAllocator getAllocator() const
	{
		return m_alloc;
	}

	LuaThread newLuaThread()
	{
		LockGuard<Mutex> lock(n_luaMtx);
		return m_lua.newLuaThread();
	}

	void destroyLuaThread(LuaThread& thread)
	{
		LockGuard<Mutex> lock(n_luaMtx);
		m_lua.destroyLuaThread(thread);
	}

private:
	SceneGraph* m_scene = nullptr;
	MainRenderer* m_r = nullptr;
	ScriptAllocator m_alloc;
	LuaBinder m_lua;
	Mutex n_luaMtx;
};
/// @}

} // end namespace anki
