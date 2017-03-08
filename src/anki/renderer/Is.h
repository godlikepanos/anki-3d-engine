// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
enum class ShaderVariantBit : U8;

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

	const LightBin& getLightBin() const
	{
		return *m_lightBin;
	}

private:
	/// The IS render target
	TexturePtr m_rt;

	Array<U32, 3> m_clusterCounts = {{0, 0, 0}};
	U32 m_clusterCount = 0;

	/// The IS FBO
	FramebufferPtr m_fb;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	ShaderResourcePtr m_lightFrag;
	ShaderProgramPtr m_lightProg;

	class ShaderVariant
	{
	public:
		ShaderResourcePtr m_lightFrag;
		ShaderProgramPtr m_lightProg;
	};

	using Key = ShaderVariantBit;

	/// Hash the hash.
	class Hasher
	{
	public:
		U64 operator()(const Key& b) const
		{
			return U64(b);
		}
	};

	HashMap<Key, ShaderVariant, Hasher> m_shaderVariantMap;

	LightBin* m_lightBin = nullptr;

	/// @name Limits
	/// @{
	U32 m_maxLightIds;
	/// @}

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void updateCommonBlock(RenderingContext& ctx);

	ANKI_USE_RESULT Error getOrCreateProgram(
		ShaderVariantBit variantMask, RenderingContext& ctx, ShaderProgramPtr& prog);
};
/// @}

} // end namespace anki
