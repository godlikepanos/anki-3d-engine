// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Spatial.h>
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

	Bool isEnabled() const
	{
		return m_layers[LayerType::kDiffuse].m_bindlessTextureIndex != kMaxU32
			   || m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex != kMaxU32;
	}

	void loadDiffuseImageResource(CString fname, F32 blendFactor)
	{
		setLayer(fname, blendFactor, LayerType::kDiffuse);
	}

	void loadRoughnessMetalnessImageResource(CString fname, F32 blendFactor)
	{
		setLayer(fname, blendFactor, LayerType::kRoughnessMetalness);
	}

	/// Update the internal structures.
	void setBoxVolumeSize(const Vec3& sizeXYZ)
	{
		m_boxSize = sizeXYZ;
		m_dirty = true;
	}

	const Vec3& getBoxVolumeSize() const
	{
		return m_boxSize;
	}

	void setupDecalQueueElement(DecalQueueElement& el) const
	{
		ANKI_ASSERT(isEnabled());
		el.m_diffuseBindlessTextureIndex = m_layers[LayerType::kDiffuse].m_bindlessTextureIndex;
		el.m_roughnessMetalnessBindlessTextureIndex = m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex;
		el.m_diffuseBlendFactor = m_layers[LayerType::kDiffuse].m_blendFactor;
		el.m_roughnessMetalnessBlendFactor = m_layers[LayerType::kRoughnessMetalness].m_blendFactor;
		el.m_textureMatrix = m_biasProjViewMat;
		el.m_obbCenter = m_obb.getCenter().xyz();
		el.m_obbExtend = m_obb.getExtend().xyz();
		el.m_obbRotation = m_obb.getRotation().getRotationPart();
	}

private:
	enum class LayerType : U8
	{
		kDiffuse,
		kRoughnessMetalness,
		kCount
	};

	class Layer
	{
	public:
		ImageResourcePtr m_image;
		F32 m_blendFactor = 0.0f;
		U32 m_bindlessTextureIndex = kMaxU32;
	};

	SceneNode* m_node;
	Spatial m_spatial;

	Array<Layer, U(LayerType::kCount)> m_layers;
	Mat4 m_biasProjViewMat = Mat4::getIdentity();
	Vec3 m_boxSize = Vec3(1.0f);
	Obb m_obb = Obb(Vec4(0.0f), Mat3x4::getIdentity(), Vec4(0.5f, 0.5f, 0.5f, 0.0f));

	Bool m_dirty = true;

	void setLayer(CString fname, F32 blendFactor, LayerType type);

	Error update(SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
