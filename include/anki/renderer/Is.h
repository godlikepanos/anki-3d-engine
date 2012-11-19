#ifndef ANKI_RENDERER_IS_H
#define ANKI_RENDERER_IS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/gl/Gl.h"
#include "anki/math/Math.h"
#include "anki/renderer/Sm.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/core/Timestamp.h"
#include "anki/collision/Plane.h"

namespace anki {

class Camera;
class PerspectiveCamera;
class PointLight;
class SpotLight;
struct ShaderPointLights;
struct ShaderSpotLights;
struct ShaderTiles;

/// Illumination stage
class Is: private RenderingPass
{
	friend struct WritePointLightsUbo;
	friend struct WriteSpotLightsUbo;
	friend struct WriteTilesUboJob;
	friend struct UpdateTilesJob;

public:
	static const U MAX_LIGHTS_PER_TILE = 32;

	static const U MAX_POINT_LIGHTS = 512;
	static const U MAX_SPOT_LIGHTS = 8;

	Is(Renderer* r);
	~Is();

	void init(const RendererInitializer& initializer);
	void run();

	/// @name Accessors
	/// @{
	const Texture& getFai() const
	{
		return fai;
	}
	/// @}

private:
	static const U COMMON_UNIFORMS_BLOCK_BINDING = 0;
	static const U POINT_LIGHTS_BLOCK_BINDING = 1;
	static const U SPOT_LIGHTS_BLOCK_BINDING = 2;
	static const U TILES_BLOCK_BINDING = 3;

	/// The IS FAI
	Texture fai;

	/// The IS FBO
	Fbo fbo;

	/// Contains common data for all shader programs
	Ubo commonUbo;

	/// Track the updates of commonUbo
	U32 commonUboUpdateTimestamp = Timestamp::getTimestamp();

	/// Contains info of for lights
	Ubo pointLightsUbo;
	Ubo spotLightsUbo;

	/// Contains the indices of lights per tile
	Ubo tilesUbo;

	/// Light shaders
	ShaderProgramResourcePointer lightPassProg;

	/// Shadow mapping
	Sm sm;

	/// Opt because many ask for it
	Camera* cam;

	Bool drawToDefaultFbo;
	U32 width, height;

	/// Called by init
	void initInternal(const RendererInitializer& initializer);

	// Do the actual pass
	void lightPass();
};

} // end namespace anki

#endif
