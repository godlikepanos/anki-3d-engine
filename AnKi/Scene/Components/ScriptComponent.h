// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

// Component of scripts. It can point to a resource with the script code or have the script code embedded to it.
class ScriptComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ScriptComponent)

public:
	ScriptComponent(const SceneComponentInitInfo& init);

	~ScriptComponent();

	void setScriptResourceFilename(CString fname);

	CString getScriptResourceFilename() const;

	void setScriptText(CString text);

	CString getScriptText() const;

	Bool hasScriptText() const
	{
		return !m_text.isEmpty();
	}

	Bool hasScriptResource() const
	{
		return m_resource.isCreated();
	}

	Bool isEnabled() const
	{
		return m_environments[0] || m_environments[1];
	}

private:
	ScriptResourcePtr m_resource;
	SceneString m_text;
	Array<ScriptEnvironment*, 2> m_environments = {};

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
