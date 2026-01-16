// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Resource/ImageAtlasResource.h>
#include <AnKi/Collision/Obb.h>

namespace anki {

// Decal component. Contains all the relevant info for a deferred decal.
class DecalComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(DecalComponent)

public:
	DecalComponent(const SceneComponentInitInfo& init);

	~DecalComponent();

	Bool isValid() const
	{
		return m_layers[LayerType::kDiffuse].m_bindlessTextureIndex != kMaxU32
			   || m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex != kMaxU32;
	}

	DecalComponent& setDiffuseImageFilename(CString fname)
	{
		setImage(LayerType::kDiffuse, fname);
		return *this;
	}

	CString getDiffuseImageFilename() const
	{
		return (m_layers[LayerType::kDiffuse].m_image) ? m_layers[LayerType::kDiffuse].m_image->getFilename() : "*Error*";
	}

	Bool hasDiffuseImageResource() const
	{
		return m_layers[LayerType::kDiffuse].m_bindlessTextureIndex != kMaxU32;
	}

	DecalComponent& setDiffuseBlendFactor(F32 f)
	{
		setBlendFactor(LayerType::kDiffuse, f);
		return *this;
	}

	F32 getDiffuseBlendFactor() const
	{
		return m_layers[LayerType::kDiffuse].m_blendFactor;
	}

	DecalComponent& setRoughnessMetalnessImageFilename(CString fname)
	{
		setImage(LayerType::kRoughnessMetalness, fname);
		return *this;
	}

	CString getRoughnessMetalnessImageFilename() const
	{
		return (m_layers[LayerType::kRoughnessMetalness].m_image) ? m_layers[LayerType::kRoughnessMetalness].m_image->getFilename() : "*Error*";
	}

	Bool hasRoughnessMetalnessImageResource() const
	{
		return m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex != kMaxU32;
	}

	DecalComponent& setRoughnessMetalnessBlendFactor(F32 f)
	{
		setBlendFactor(LayerType::kRoughnessMetalness, f);
		return *this;
	}

	F32 getRoughnessMetalnessBlendFactor() const
	{
		return m_layers[LayerType::kRoughnessMetalness].m_blendFactor;
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
		F32 m_blendFactor = 1.0f;
		U32 m_bindlessTextureIndex = kMaxU32;
	};

	ImageResourcePtr m_defaultDecalImage; // Keep that loaded to avoid loading it all the time when a new decal is constructed

	Array<Layer, U(LayerType::kCount)> m_layers;

	GpuSceneArrays::Decal::Allocation m_gpuSceneDecal;

	Bool m_dirty = true;

	void setImage(LayerType type, CString fname);

	void setBlendFactor(LayerType type, F32 blendFactor);

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
