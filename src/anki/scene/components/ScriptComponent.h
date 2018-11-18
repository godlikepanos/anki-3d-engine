// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/resource/Forward.h>
#include <anki/script/ScriptEnvironment.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Component of scripts.
class ScriptComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::SCRIPT;

	ScriptComponent(SceneNode* node);

	~ScriptComponent();

	ANKI_USE_RESULT Error load(CString fname);

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

private:
	SceneNode* m_node;
	ScriptResourcePtr m_script;
	ScriptEnvironment m_env;
};
/// @}

} // end namespace anki