#ifndef ANKI_RENDERER_SM_H
#define ANKI_RENDERER_SM_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include <array>

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

	bool getPcfEnabled() const
	{
		return pcfEnabled;
	}

	bool getBilinearEnabled() const
	{
		return bilinearEnabled;
	}
	/// @}

	void init(const RendererInitializer& initializer);

	/// Render the scene only with depth and store the result in the
	/// shadowMap
	/// @param[in] light The light
	/// @param[in] distance The distance between the viewer's camera and the
	///            light
	Texture* run(Light& light, float distance); /// XXX order

private:
	struct Shadowmap
	{
		Texture tex;
		Fbo fbo;
		Light* light = nullptr;
	};

	static const int MAX_SHADOWMAPS = 5;

	std::array<Shadowmap, MAX_SHADOWMAPS> sms;

	bool enabled; ///< If false then disable SM at all
	/// Enable Percentage Closer Filtering for all the levels
	bool pcfEnabled;
	/// Shadowmap bilinear filtering for the first level. Better quality
	bool bilinearEnabled;
	/// Shadowmap resolution
	int resolution;

	/// XXX
	Shadowmap& findBestCandidate(Light& light);
};


} // end namespace


#endif
