// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Scene/ScriptUtils.h>

namespace anki {

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
		return m_env != nullptr;
	}

	template<typename TFunc>
	FunctorContinue iterateVariables(TFunc func)
	{
		return m_vars.iterateVariables(func);
	}

#if ANKI_WITH_EDITOR
	Bool getPlayOnEditor() const
	{
		return m_playOnEditor;
	}

	void setPlayOnEditor(Bool play)
	{
		m_playOnEditor = play;
	}
#endif

private:
	ScriptResourcePtr m_resource;
	SceneString m_text;

	ScriptEnvironment* m_env = nullptr;

	ScriptVariables m_vars;

#if ANKI_WITH_EDITOR
	Bool m_playOnEditor = false;
#endif

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
