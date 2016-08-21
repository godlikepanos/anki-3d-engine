// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>
#include <anki/resource/TextureAtlas.h>
#include <anki/collision/Obb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Decal component. Contains all the relevant info for a deferred decal.
class DecalComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::DECAL;

	static constexpr F32 FRUSTUM_NEAR_PLANE = 0.1 / 4.0;

	DecalComponent(SceneNode* node)
		: SceneComponent(CLASS_TYPE, node)
	{
	}

	~DecalComponent();

	ANKI_USE_RESULT Error setDiffuseDecal(
		CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(
			texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::DIFFUSE);
	}

	/// Update the internal structures using a world space Obb.
	void updateShape(const Obb& box)
	{
		m_obb = box;
		m_markedForUpdate = true;
	}

	/// Implements SceneComponent::update.
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override
	{
		updated = m_markedForUpdate;

		if(m_markedForUpdate)
		{
			m_markedForUpdate = false;
			updateMatrices(m_obb);
		}

		return ErrorCode::NONE;
	}

private:
	enum class LayerType
	{
		DIFFUSE,
		COUNT
	};

	class Layer
	{
	public:
		TextureAtlasResourcePtr m_atlas;
		Vec4 m_uv = Vec4(0.0f);
		F32 m_blendFactor = 0.0f;
	};

	Array<Layer, U(LayerType::COUNT)> m_layers;
	Mat4 m_biasProjViewMat;
	Obb m_obb =
		Obb(Vec4(0.0f), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f));
	Bool8 m_markedForUpdate = true;

	ANKI_USE_RESULT Error setLayer(CString texAtlasFname,
		CString texAtlasSubtexName,
		F32 blendFactor,
		LayerType type);

	void updateMatrices(const Obb& box);
};
/// @}

} // end namespace anki
