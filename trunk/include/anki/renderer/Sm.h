#ifndef ANKI_RENDERER_SM_H
#define ANKI_RENDERER_SM_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Vector.h"
#include "anki/util/Array.h"

namespace anki {

class Light;

/// Shadowmapping pass
class Sm: private RenderingPass
{
	friend class Is;
public:
	static const U32 MAX_SHADOW_CASTERS = 8;

	Sm(Renderer* r_)
		: RenderingPass(r_)
	{}

	/// @name Accessors
	/// @{
	Bool getEnabled() const
	{
		return enabled;
	}

	Bool getPoissonEnabled() const
	{
		return poissonEnabled;
	}
	/// @}

	void init(const RendererInitializer& initializer);
	void run(Light* shadowCasters[], U32 shadowCastersCount);

private:
	Texture sm2DArrayTex;

	/// Shadowmap
	struct Shadowmap
	{
		U32 layerId;
		Fbo fbo;
		Light* light = nullptr;
		U32 timestamp = 0; ///< Timestamp of last render or light change
	};

	Vector<Shadowmap> sms;

	/// If false then disable SM at all
	Bool enabled; 

	/// Enable Poisson for all the levels
	Bool poissonEnabled;

	/// Shadowmap bilinear filtering for the first level. Better quality
	Bool bilinearEnabled;

	/// Shadowmap resolution
	U32 resolution;

	/// Get max shadow casters
	U32 getMaxLightsCount()
	{
		return sms.size();
	}

	void prepareDraw();
	void afterDraw();

	/// Find the best shadowmap for that light
	Shadowmap& bestCandidate(Light& light);

	Shadowmap* doLight(Light& light);
};

} // end namespace anki

#endif
