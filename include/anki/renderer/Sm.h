// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Array.h"

namespace anki {

// Forward
class SceneNode;

/// @addtogroup renderer
/// @{

/// Shadowmapping pass
class Sm: private RenderingPass
{
public:
	static const U32 MAX_SHADOW_CASTERS = 8;
#ifdef ANKI_BUILD
	static const PixelFormat DEPTH_RT_PIXEL_FORMAT;

	Sm(Renderer* r)
		: RenderingPass(r)
	{}

	~Sm()
	{
		m_spots.destroy(getAllocator());
		m_omnis.destroy(getAllocator());
	}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(
		SArray<SceneNode*> spotShadowCasters,
		SArray<SceneNode*> omniShadowCasters,
		CommandBufferPtr& cmdBuff);

	Bool getEnabled() const
	{
		return m_enabled;
	}

	Bool getPoissonEnabled() const
	{
		return m_poissonEnabled;
	}

	/// Get max shadow casters
	U32 getMaxLightsCount()
	{
		return m_spots.getSize();
	}

	TexturePtr& getSpotTextureArray()
	{
		return m_spotTexArray;
	}

	TexturePtr& getOmniTextureArray()
	{
		return m_omniTexArray;
	}
#endif

private:
	TexturePtr m_spotTexArray;
	TexturePtr m_omniTexArray;

	class ShadowmapBase
	{
	public:
		U32 m_layerId;
		SceneNode* m_light = nullptr;
		U32 m_timestamp = 0; ///< Timestamp of last render or light change
	};

	class ShadowmapSpot: public ShadowmapBase
	{
	public:
		FramebufferPtr m_fb;
	};

	class ShadowmapOmni: public ShadowmapBase
	{
	public:
		Array<FramebufferPtr, 6> m_fb;
	};

	DArray<ShadowmapSpot> m_spots;
	DArray<ShadowmapOmni> m_omnis;

	/// If false then disable SM at all
	Bool8 m_enabled;

	/// Enable Poisson for all the levels
	Bool8 m_poissonEnabled;

	/// Shadowmap bilinear filtering for the first level. Better quality
	Bool8 m_bilinearEnabled;

	/// Shadowmap resolution
	U32 m_resolution;

	void prepareDraw(CommandBufferPtr& cmdBuff);
	void finishDraw(CommandBufferPtr& cmdBuff);

	/// Find the best shadowmap for that light
	template<typename TShadowmap, typename TContainer>
	void bestCandidate(SceneNode& light, TContainer& arr, TShadowmap*& out);

	/// Check if a shadow pass can be skipped.
	Bool skip(SceneNode& light, ShadowmapBase& sm);

	ANKI_USE_RESULT Error doSpotLight(SceneNode& light,
		CommandBufferPtr& cmdBuff);

	ANKI_USE_RESULT Error doOmniLight(SceneNode& light,
		CommandBufferPtr& cmdBuff);
};

/// @}

} // end namespace anki

