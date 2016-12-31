// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/util/Array.h>

namespace anki
{

// Forward
class SceneNode;

/// @addtogroup renderer
/// @{

/// Shadowmapping pass
class Sm : public RenderingPass
{
anki_internal:
	static const PixelFormat DEPTH_RT_PIXEL_FORMAT;

	/// Enable Poisson for all the levels
	Bool8 m_poissonEnabled = false;

	TexturePtr m_spotTexArray;
	TexturePtr m_omniTexArray;

	Sm(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Sm();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void prepareBuildCommandBuffers(RenderingContext& ctx);

	ANKI_USE_RESULT Error buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const;

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	class ShadowmapBase
	{
	public:
		U32 m_layerId;
		SceneNode* m_light = nullptr;
		U32 m_timestamp = 0; ///< Timestamp of last render or light change
	};

	class ShadowmapSpot : public ShadowmapBase
	{
	public:
		FramebufferPtr m_fb;
	};

	class ShadowmapOmni : public ShadowmapBase
	{
	public:
		Array<FramebufferPtr, 6> m_fb;
	};

	DynamicArray<ShadowmapSpot> m_spots;
	DynamicArray<ShadowmapOmni> m_omnis;

	/// Shadowmap bilinear filtering for the first level. Better quality
	Bool8 m_bilinearEnabled;

	/// Shadowmap resolution
	U32 m_resolution;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Find the best shadowmap for that light
	template<typename TShadowmap, typename TContainer>
	void bestCandidate(SceneNode& light, TContainer& arr, TShadowmap*& out);

	/// Check if a shadow pass can be skipped.
	Bool skip(SceneNode& light, ShadowmapBase& sm);

	ANKI_USE_RESULT Error doSpotLight(
		SceneNode& light, CommandBufferPtr& cmdBuff, FramebufferPtr& fb, U threadId, U threadCount) const;

	ANKI_USE_RESULT Error doOmniLight(
		SceneNode& light, CommandBufferPtr cmdbs[], Array<FramebufferPtr, 6>& fbs, U threadId, U threadCount) const;
};

/// @}

} // end namespace anki
