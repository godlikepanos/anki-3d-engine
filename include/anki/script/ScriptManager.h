// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCRIPT_SCRIPT_MANAGER_H
#define ANKI_SCRIPT_SCRIPT_MANAGER_H

#include "anki/script/LuaBinder.h"

namespace anki {

// Forward
class SceneGraph;

/// @addtogroup script
/// @{

/// The scripting manager
class ScriptManager
{
public:
	ScriptManager();
	~ScriptManager();

	/// Create the script manager.
	ANKI_USE_RESULT Error create(
		AllocAlignedCallback allocCb, 
		void* allocCbData,
		SceneGraph* scene);

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

	/// @privatesection
	/// @{
	SceneGraph& _getSceneGraph()
	{
		return *m_scene;
	}
	/// @}

public:
	SceneGraph* m_scene = nullptr;
	ChainAllocator<U8> m_alloc;
	LuaBinder m_lua;
};

/// @}

} // end namespace anki

#endif
