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
	const Texture& getShadowMap() const
	{
		return crntLevel->shadowMap;
	}

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

	int getResolution() const
	{
		return resolution;
	}
	/// @}

	void init(const RendererInitializer& initializer);

	/// Render the scene only with depth and store the result in the
	/// shadowMap
	/// @param[in] light The light
	/// @param[in] distance The distance between the viewer's camera and the
	///            light
	void run(Light& light, float distance); /// XXX order

private:
	/// The shadowmap levels of detail
	/// It starts from level 0 which is the detailed one and moves to lower
	/// resolution shadowmaps. When the distance between the viewer's
	/// camera and the light is getting bigger then a higher level will be
	/// used
	struct Level
	{
		Fbo fbo; ///< Illumination stage shadowmapping FBO
		Texture shadowMap; ///< The shadowmap for that level
		uint32_t resolution; ///< The shadowmap's resolution
		bool bilinear; ///< Enable bilinar filtering in shadowmap
		/// The maximum distance that this level applies. The distance is
		/// between the viewer's camera and the light
		float distance;
	};

	std::array<Level, 4> levels; ///< The levels of detail. The 0 is the
								   ///< detailed one
	Level* crntLevel; ///< Current level of detail. Assigned by run

	bool enabled; ///< If false then disable SM at all
	/// Enable Percentage Closer Filtering for all the levels
	bool pcfEnabled;
	/// Shadowmap bilinear filtering for the first level. Better quality
	bool bilinearEnabled;
	/// Shadowmap resolution of the first level. The higher the better but
	/// slower
	int resolution;
	/// The distance of the first level. @see Level::distance
	float level0Distance;

	/// Initialize a level
	static void initLevel(uint resolution, float maxDistance,
		bool bilinear, Level& level);
};


} // end namespace


#endif
