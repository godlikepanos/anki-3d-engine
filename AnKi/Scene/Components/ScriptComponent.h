// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Component of scripts.
class ScriptComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ScriptComponent)

public:
	ScriptComponent(SceneNode* node);

	~ScriptComponent();

	Error loadScriptResource(CString fname);

	Bool isEnabled() const
	{
		return m_script.isCreated();
	}

private:
	SceneNode* m_node;
	ScriptResourcePtr m_script;
	ScriptEnvironment* m_env = nullptr;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
