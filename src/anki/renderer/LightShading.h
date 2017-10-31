// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderProgramResource.h>

namespace anki
{

// Forward
class LightBin;

/// @addtogroup renderer
/// @{

/// Clustered deferred light pass.
class LightShading : public RendererObject
{
anki_internal:
	LightShading(Renderer* r);

	~LightShading();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void populateRenderGraph(RenderingContext& ctx);

	ANKI_USE_RESULT Error binLights(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	const LightBin& getLightBin() const
	{
		return *m_lightBin;
	}

private:
	Array<U32, 3> m_clusterCounts = {{0, 0, 0}};
	U32 m_clusterCount = 0;

	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;

	// Light shaders
	ShaderProgramResourcePtr m_prog;
	const ShaderProgramResourceVariant* m_progVariant = nullptr;

	LightBin* m_lightBin = nullptr;

	/// @name Limits
	/// @{
	U32 m_maxLightIds;
	/// @}

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx;
	} m_runCtx; ///< Run context.

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void updateCommonBlock(RenderingContext& ctx);

	void run(const RenderingContext& ctx, const RenderGraph& rgraph, CommandBufferPtr& cmdb);

	/// A RenderPassWorkCallback for the light pass.
	static void runCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		LightShading* self = static_cast<LightShading*>(userData);
		self->run(*self->m_runCtx.m_ctx, rgraph, cmdb);
	}
};
/// @}

} // end namespace anki
