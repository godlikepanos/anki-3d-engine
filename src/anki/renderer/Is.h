// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>

namespace anki
{

// Forward
class FrustumComponent;
class LightBin;

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is : public RenderingPass
{
anki_internal:
	Is(Renderer* r);

	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error binLights(RenderingContext& ctx);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	TexturePtr getRt() const
	{
		return m_rt;
	}

	/// Get the number of mips for IS's render target.
	U getRtMipmapCount() const
	{
		return m_rtMipCount;
	}

private:
	static const U COMMON_VARS_LOCATION = 0;
	static const U P_LIGHTS_LOCATION = 1;
	static const U S_LIGHTS_LOCATION = 2;
	static const U PROBES_LOCATION = 3;
	static const U DECALS_LOCATION = 4;
	static const U CLUSTERS_LOCATION = 0;
	static const U LIGHT_IDS_LOCATION = 1;

	/// The IS render target
	TexturePtr m_rt;
	U8 m_rtMipCount = 0;

	/// The IS FBO
	FramebufferPtr m_fb;

	ResourceGroupPtr m_rcGroup;
	U64 m_rcGroup1Hash = 0;
	ResourceGroupPtr m_rcGroup1;

	TexturePtr m_dummyTex;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	ShaderResourcePtr m_lightFrag;
	PipelinePtr m_lightPpline;

	LightBin* m_lightBin = nullptr;

	/// @name Limits
	/// @{
	U32 m_maxLightIds;
	/// @}

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void updateCommonBlock(RenderingContext& ctx);
};
/// @}

} // end namespace anki
