#ifndef ANKI_RENDERER_SM_H
#define ANKI_RENDERER_SM_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Vector.h"

namespace anki {

class Light;

/// Shadowmapping pass
class Sm: private RenderingPass
{
public:
	Sm(Renderer* r_)
		: RenderingPass(r_)
	{}

	/// @name Accessors
	/// @{
	bool getEnabled() const
	{
		return enabled;
	}
	/// @}

	void init(const RendererInitializer& initializer);
	void run();

private:
	/// Shadowmap
	struct Shadowmap
	{
		Texture tex;
		Fbo fbo;
		Light* light = nullptr;
		U32 timestamp = 0; ///< Timestamp of last render or light change
	};

	Vector<Shadowmap> sms;

	/// If false then disable SM at all
	Bool enabled; 

	/// Enable Percentage Closer Filtering for all the levels
	Bool pcfEnabled;

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

	void doLight(Light& light);
};

} // end namespace anki

#endif
