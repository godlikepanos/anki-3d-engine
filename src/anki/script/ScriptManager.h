// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

/// The scripting manager
class ScriptManager
{
public:
	/// Expose a variable to the scripting engine.
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		m_lua.exposeVariable<T>(name, y);
	}

	/// Evaluate a string
	ANKI_USE_RESULT Error evalString(const CString& str)
	{
		return m_lua.evalString(str);
	}

anki_internal:
	ScriptManager();
	~ScriptManager();

	/// Create the script manager.
	ANKI_USE_RESULT Error init(
		AllocAlignedCallback allocCb, void* allocCbData, SceneGraph* scene, MainRenderer* renderer);

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	MainRenderer& getMainRenderer()
	{
		return *m_r;
	}

private:
	SceneGraph* m_scene = nullptr;
	MainRenderer* m_r = nullptr;
	ChainAllocator<U8> m_alloc;
	LuaBinder m_lua;
};

/// @}

} // end namespace anki
