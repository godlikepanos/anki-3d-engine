// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SM_H
#define ANKI_RENDERER_SM_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Vector.h"
#include "anki/util/Array.h"

namespace anki {

class Light;

/// @addtogroup renderer
/// @{

/// Shadowmapping pass
class Sm: private RenderingPass
{
	friend class Is;

public:
	static const U32 MAX_SHADOW_CASTERS = 8;

	/// @name Accessors
	/// @{
	Bool getEnabled() const
	{
		return m_enabled;
	}

	Bool getPoissonEnabled() const
	{
		return m_poissonEnabled;
	}
	/// @}

private:
	GlTextureHandle m_sm2DArrayTex;

	/// Shadowmap
	struct Shadowmap
	{
		U32 m_layerId;
		GlFramebufferHandle m_fb;
		Light* m_light = nullptr;
		U32 m_timestamp = 0; ///< Timestamp of last render or light change
	};

	Vector<Shadowmap> m_sms;

	/// If false then disable SM at all
	Bool8 m_enabled; 

	/// Enable Poisson for all the levels
	Bool8 m_poissonEnabled;

	/// Shadowmap bilinear filtering for the first level. Better quality
	Bool8 m_bilinearEnabled;

	/// Shadowmap resolution
	U32 m_resolution;

	Sm(Renderer* r)
		: RenderingPass(r)
	{}

	void init(const ConfigSet& initializer);
	void run(Light* shadowCasters[], U32 shadowCastersCount, 
		GlJobChainHandle& jobs);

	/// Get max shadow casters
	U32 getMaxLightsCount()
	{
		return m_sms.size();
	}

	void prepareDraw(GlJobChainHandle& jobs);
	void finishDraw(GlJobChainHandle& jobs);

	/// Find the best shadowmap for that light
	Shadowmap& bestCandidate(Light& light);

	Shadowmap* doLight(Light& light, GlJobChainHandle& jobs);
};

/// @}

} // end namespace anki

#endif
