// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Scene/ScriptUtils.h>

namespace anki {

// The Lua callbacks that a script defines
enum class ScriptComponentFunctionMask : U8
{
	kNone = 0,
	kUpdate = 1 << 0,
	kOnAnimationStarted = 1 << 1,
	kOnAnimationEnded = 1 << 2,
	kOnTriggerEnter = 1 << 3,
	kOnTriggerExit = 1 << 4,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ScriptComponentFunctionMask)

// Component of scripts. It can point to a resource with the script code or have the script code embedded to it.
class ScriptComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ScriptComponent)

public:
	ScriptComponent(const SceneComponentInitInfo& init);

	~ScriptComponent();

	// Initialize using a script resource. Calling this will remove what was set with setScriptText()
	ScriptComponent& setScriptResourceFilename(CString fname);

	CString getScriptResourceFilename() const;

	// Initialize using plain LUA text. Calling this will remove what was set with setScriptResourceFilename()
	ScriptComponent& setScriptText(CString text);

	CString getScriptText() const;

	Bool hasScriptText() const
	{
		return !m_text.isEmpty();
	}

	Bool hasScriptResource() const
	{
		return m_resource.isCreated();
	}

	Bool isValid() const
	{
		return m_env.isCreated();
	}

	template<typename TFunc>
	FunctorContinue iterateVariables(TFunc func)
	{
		return m_vars.iterateVariables(func);
	}

#if ANKI_WITH_EDITOR
	ANKI_INTERNAL Bool getPlayOnEditor() const
	{
		return m_playOnEditor;
	}

	ANKI_INTERNAL void setPlayOnEditor(Bool play)
	{
		m_playOnEditor = play;
	}

	ANKI_INTERNAL SceneNode& getSceneNode() const
	{
		return *m_node;
	}
#endif

	// Call the onTriggerEnter or onTriggerExit with some parameters
	ANKI_INTERNAL void callOnTrigger(Bool onEnter, SceneNode& selfNode, SceneNode& otherNode);

private:
	ScriptResourcePtr m_resource;
	SceneString m_text;

	ScriptEnvironmentPtr m_env;

	ScriptVariables m_vars;

#if ANKI_WITH_EDITOR
	Bool m_playOnEditor = false;
	SceneNode* m_node = nullptr;
#endif

	ScriptComponentFunctionMask m_definedFuncsMask = ScriptComponentFunctionMask::kNone; // A callback is disabled after its 1st error until a reload

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;

	void rebuildDefinedFunctionsMask();
};

} // end namespace anki
