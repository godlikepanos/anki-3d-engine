// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SM_H
#define ANKI_RENDERER_SM_H

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
		m_sms.destroy(getAllocator());
	}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(
		SceneNode* shadowCasters[],
		U32 shadowCastersCount,
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
		return m_sms.getSize();
	}

	TexturePtr& getTextureArray()
	{
		return m_sm2DArrayTex;
	}
#endif

private:
	TexturePtr m_sm2DArrayTex;

	/// Shadowmap
	struct Shadowmap
	{
		U32 m_layerId;
		FramebufferPtr m_fb;
		SceneNode* m_light = nullptr;
		U32 m_timestamp = 0; ///< Timestamp of last render or light change
	};

	DArray<Shadowmap> m_sms;

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
	Shadowmap& bestCandidate(SceneNode& light);

	ANKI_USE_RESULT Error doLight(
		SceneNode& light, CommandBufferPtr& cmdBuff, Shadowmap*& sm);
};

/// @}

} // end namespace anki

#endif
