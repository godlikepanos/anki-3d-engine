// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/resource/TextureAtlasResource.h>
#include <anki/collision/Obb.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Decal component. Contains all the relevant info for a deferred decal.
class DecalComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::DECAL;

	static constexpr F32 FRUSTUM_NEAR_PLANE = 0.1f / 4.0f;
	static constexpr U ATLAS_SUB_TEXTURE_MARGIN = 16;

	DecalComponent(SceneNode* node);

	~DecalComponent();

	ANKI_USE_RESULT Error setDiffuseDecal(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::DIFFUSE);
	}

	ANKI_USE_RESULT Error setSpecularRoughnessDecal(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::SPECULAR_ROUGHNESS);
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

	const Obb& getBoundingVolume() const
	{
		return m_obb;
	}

	void updateTransform(const Transform& trf)
	{
		ANKI_ASSERT(trf.getScale() == 1.0f);
		m_trf = trf;
		m_markedForUpdate = true;
	}

	/// Implements SceneComponent::update.
	ANKI_USE_RESULT Error update(SceneNode&, Second, Second, Bool& updated) override
	{
		updated = m_markedForUpdate;

		if(m_markedForUpdate)
		{
			m_markedForUpdate = false;
			updateInternal();
		}

		return Error::NONE;
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

	void getSpecularRoughnessAtlasInfo(Vec4& uv, TexturePtr& tex, F32& blendFactor) const
	{
		uv = m_layers[LayerType::SPECULAR_ROUGHNESS].m_uv;
		if(m_layers[LayerType::SPECULAR_ROUGHNESS].m_atlas)
		{
			tex = m_layers[LayerType::SPECULAR_ROUGHNESS].m_atlas->getGrTexture();
		}
		else
		{
			tex.reset(nullptr);
		}
		blendFactor = m_layers[LayerType::SPECULAR_ROUGHNESS].m_blendFactor;
	}

	const Vec3& getVolumeSize() const
	{
		return m_sizes;
	}

	void setupDecalQueueElement(DecalQueueElement& el) const
	{
		el.m_diffuseAtlas = (m_layers[LayerType::DIFFUSE].m_atlas)
								? m_layers[LayerType::DIFFUSE].m_atlas->getGrTextureView().get()
								: nullptr;
		el.m_specularRoughnessAtlas = (m_layers[LayerType::SPECULAR_ROUGHNESS].m_atlas)
										  ? m_layers[LayerType::SPECULAR_ROUGHNESS].m_atlas->getGrTextureView().get()
										  : nullptr;
		el.m_diffuseAtlasUv = m_layers[LayerType::DIFFUSE].m_uv;
		el.m_specularRoughnessAtlasUv = m_layers[LayerType::SPECULAR_ROUGHNESS].m_uv;
		el.m_diffuseAtlasBlendFactor = m_layers[LayerType::DIFFUSE].m_blendFactor;
		el.m_specularRoughnessAtlasBlendFactor = m_layers[LayerType::SPECULAR_ROUGHNESS].m_blendFactor;
		el.m_textureMatrix = m_biasProjViewMat;
		el.m_obbCenter = m_obb.getCenter().xyz();
		el.m_obbExtend = m_obb.getExtend().xyz();
		el.m_obbRotation = m_obb.getRotation().getRotationPart();
		el.m_userData = this;
		el.m_drawCallback = debugDrawCallback;
	}

private:
	enum class LayerType
	{
		DIFFUSE,
		SPECULAR_ROUGHNESS,
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
	Vec3 m_sizes = Vec3(1.0f);
	Transform m_trf = Transform::getIdentity();
	Obb m_obb = Obb(Vec4(0.0f), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f));
	Bool8 m_markedForUpdate = true;

	ANKI_USE_RESULT Error setLayer(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor, LayerType type);

	void updateInternal();

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
