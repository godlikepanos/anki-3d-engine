// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	static constexpr U ATLAS_SUB_TEXTURE_MARGIN = 16;

	DecalComponent(SceneNode* node);

	~DecalComponent();

	ANKI_USE_RESULT Error setDiffuseDecal(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::DIFFUSE);
	}

	ANKI_USE_RESULT Error setNormalRoughnessDecal(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::NORMAL_ROUGHNESS);
	}

	/// Update the internal structures.
	void updateShape(F32 width, F32 height, F32 depth)
	{
		m_sizes = Vec3(width, height, depth);
		m_markedForUpdate = true;
	}

	F32 getWidth() const
	{
		return m_sizes.x();
	}

	F32 getHeight() const
	{
		return m_sizes.y();
	}

	F32 getDepth() const
	{
		return m_sizes.z();
	}

	void updateTransform(const Transform& trf)
	{
		ANKI_ASSERT(trf.getScale() == 1.0f);
		m_trf = trf;
		m_markedForUpdate = true;
	}

	/// Implements SceneComponent::update.
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override
	{
		updated = m_markedForUpdate;

		if(m_markedForUpdate)
		{
			m_markedForUpdate = false;
			updateInternal();
		}

		return ErrorCode::NONE;
	}

	const Mat4& getBiasProjectionViewMatrix() const
	{
		return m_biasProjViewMat;
	}

	void getDiffuseAtlasInfo(Vec4& uv, TexturePtr& tex, F32& blendFactor) const
	{
		uv = m_layers[LayerType::DIFFUSE].m_uv;
		tex = m_layers[LayerType::DIFFUSE].m_atlas->getGrTexture();
		blendFactor = m_layers[LayerType::DIFFUSE].m_blendFactor;
	}

	void getNormalRoughnessAtlasInfo(Vec4& uv, TexturePtr& tex, F32& blendFactor) const
	{
		uv = m_layers[LayerType::NORMAL_ROUGHNESS].m_uv;
		if(m_layers[LayerType::NORMAL_ROUGHNESS].m_atlas)
		{
			tex = m_layers[LayerType::NORMAL_ROUGHNESS].m_atlas->getGrTexture();
		}
		else
		{
			tex.reset(nullptr);
		}
		blendFactor = m_layers[LayerType::NORMAL_ROUGHNESS].m_blendFactor;
	}

	const Vec3& getVolumeSize() const
	{
		return m_sizes;
	}

private:
	enum class LayerType
	{
		DIFFUSE,
		NORMAL_ROUGHNESS,
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
	Vec3 m_sizes = Vec3(1.0);
	Transform m_trf = Transform::getIdentity();
	Bool8 m_markedForUpdate = true;

	ANKI_USE_RESULT Error setLayer(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor, LayerType type);

	void updateInternal();
};
/// @}

} // end namespace anki
