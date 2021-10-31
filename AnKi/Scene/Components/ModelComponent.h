// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Holds geometry and material information.
class ModelComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ModelComponent)

public:
	ModelComponent(SceneNode* node);

	~ModelComponent();

	ANKI_USE_RESULT Error loadModelResource(CString filename);

	const ModelResourcePtr& getModelResource() const
	{
		return m_model;
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = m_dirty;
		m_dirty = false;
		return Error::NONE;
	}

	ConstWeakArray<U64> getRenderMergeKeys() const
	{
		return m_modelPatchMergeKeys;
	}

	Bool isEnabled() const
	{
		return m_model.isCreated();
	}

private:
	SceneNode* m_node = nullptr;
	ModelResourcePtr m_model;

	DynamicArray<U64> m_modelPatchMergeKeys;
	Bool m_dirty = true;
};
/// @}

} // end namespace anki
