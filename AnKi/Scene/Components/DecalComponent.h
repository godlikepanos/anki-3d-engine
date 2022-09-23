// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/ImageAtlasResource.h>
#include <AnKi/Collision/Obb.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Decal component. Contains all the relevant info for a deferred decal.
class DecalComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(DecalComponent)

public:
	static constexpr U32 kAtlasSubImageMargin = 16;

	DecalComponent(SceneNode* node);

	~DecalComponent();

	Error setDiffuseDecal(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::kDiffuse);
	}

	Error setSpecularRoughnessDecal(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor)
	{
		return setLayer(texAtlasFname, texAtlasSubtexName, blendFactor, LayerType::kSpecularRoughness);
	}

	/// Update the internal structures.
	void setBoxVolumeSize(const Vec3& sizeXYZ)
	{
		m_boxSize = sizeXYZ;
		m_markedForUpdate = true;
	}

	const Vec3& getBoxVolumeSize() const
	{
		return m_boxSize;
	}

	const Obb& getBoundingVolumeWorldSpace() const
	{
		return m_obb;
	}

	void setWorldTransform(const Transform& trf)
	{
		ANKI_ASSERT(trf.getScale() == 1.0f);
		m_trf = trf;
		m_markedForUpdate = true;
	}

	/// Implements SceneComponent::update.
	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = m_markedForUpdate;
		m_markedForUpdate = false;
		if(updated)
		{
			updateInternal();
		}

		return Error::kNone;
	}

	const Mat4& getBiasProjectionViewMatrix() const
	{
		return m_biasProjViewMat;
	}

	void getDiffuseAtlasInfo(Vec4& uv, TexturePtr& tex, F32& blendFactor) const
	{
		uv = m_layers[LayerType::kDiffuse].m_uv;
		tex = m_layers[LayerType::kDiffuse].m_atlas->getTexture();
		blendFactor = m_layers[LayerType::kDiffuse].m_blendFactor;
	}

	void getSpecularRoughnessAtlasInfo(Vec4& uv, TexturePtr& tex, F32& blendFactor) const
	{
		uv = m_layers[LayerType::kSpecularRoughness].m_uv;
		if(m_layers[LayerType::kSpecularRoughness].m_atlas)
		{
			tex = m_layers[LayerType::kSpecularRoughness].m_atlas->getTexture();
		}
		else
		{
			tex.reset(nullptr);
		}
		blendFactor = m_layers[LayerType::kSpecularRoughness].m_blendFactor;
	}

	void setupDecalQueueElement(DecalQueueElement& el)
	{
		el.m_diffuseAtlas = (m_layers[LayerType::kDiffuse].m_atlas)
								? m_layers[LayerType::kDiffuse].m_atlas->getTextureView().get()
								: nullptr;
		el.m_specularRoughnessAtlas = (m_layers[LayerType::kSpecularRoughness].m_atlas)
										  ? m_layers[LayerType::kSpecularRoughness].m_atlas->getTextureView().get()
										  : nullptr;
		el.m_diffuseAtlasUv = m_layers[LayerType::kDiffuse].m_uv;
		el.m_specularRoughnessAtlasUv = m_layers[LayerType::kSpecularRoughness].m_uv;
		el.m_diffuseAtlasBlendFactor = m_layers[LayerType::kDiffuse].m_blendFactor;
		el.m_specularRoughnessAtlasBlendFactor = m_layers[LayerType::kSpecularRoughness].m_blendFactor;
		el.m_textureMatrix = m_biasProjViewMat;
		el.m_obbCenter = m_obb.getCenter().xyz();
		el.m_obbExtend = m_obb.getExtend().xyz();
		el.m_obbRotation = m_obb.getRotation().getRotationPart();
		el.m_debugDrawCallback = [](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			ANKI_ASSERT(userData.getSize() == 1);
			static_cast<const DecalComponent*>(userData[0])->draw(ctx);
		};
		el.m_debugDrawCallbackUserData = this;
	}

private:
	enum class LayerType : U8
	{
		kDiffuse,
		kSpecularRoughness,
		kCount
	};

	class Layer
	{
	public:
		ImageAtlasResourcePtr m_atlas;
		Vec4 m_uv = Vec4(0.0f);
		F32 m_blendFactor = 0.0f;
	};

	SceneNode* m_node = nullptr;
	Array<Layer, U(LayerType::kCount)> m_layers;
	Mat4 m_biasProjViewMat = Mat4::getIdentity();
	Vec3 m_boxSize = Vec3(1.0f);
	Transform m_trf = Transform::getIdentity();
	Obb m_obb = Obb(Vec4(0.0f), Mat3x4::getIdentity(), Vec4(0.5f, 0.5f, 0.5f, 0.0f));
	ImageResourcePtr m_debugImage;
	Bool m_markedForUpdate = true;

	Error setLayer(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor, LayerType type);

	void updateInternal();

	void draw(RenderQueueDrawContext& ctx) const;
};
/// @}

} // end namespace anki
