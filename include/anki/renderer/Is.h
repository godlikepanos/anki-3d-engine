#ifndef ANKI_RENDERER_IS_H
#define ANKI_RENDERER_IS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/gl/Gl.h"
#include "anki/Math.h"
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

/// Illumination stage
class Is: private RenderingPass
{
	friend struct WriteLightsJob;

public:
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

	Texture& getFai()
	{
		return fai;
	}
	/// @}

private:
	static const U COMMON_UNIFORMS_BLOCK_BINDING = 0;
	static const U POINT_LIGHTS_BLOCK_BINDING = 1;
	static const U SPOT_LIGHTS_BLOCK_BINDING = 2;
	static const U SPOT_TEX_LIGHTS_BLOCK_BINDING = 3;
	static const U TILES_BLOCK_BINDING = 4;
	static const U TILES_POINT_LIGHT_INDICES_BLOCK_BINDING = 5;
	static const U TILES_SPOT_LIGHT_INDICES_BLOCK_BINDING = 6;

	/// The IS FAI
	Texture fai;

	/// The IS FBO
	Fbo fbo;

	/// @name GPU buffers
	/// @{

	PtrSize uboAlignment = MAX_PTR_SIZE; ///< Cache the value here

	/// Contains common data for all shader programs
	BufferObject commonUbo;

	/// Track the updates of commonUbo
	Timestamp commonUboUpdateTimestamp = getGlobTimestamp();

	/// Contains all the lights
	BufferObject lightsUbo;

	/// Contains the number of lights per tile
	BufferObject tilesBuffer;

	BufferObject pointLightIndicesBuffer;
	BufferObject spotLightIndicesBuffer;
	/// @}

	// Light shaders
	ShaderProgramResourcePointer lightPassProg;
	ShaderProgramResourcePointer rejectProg;

	/// Shadow mapping
	Sm sm;

	/// Opt because many ask for it
	Camera* cam;

	/// If enabled the ground emmits a light
	Bool groundLightEnabled;
	/// Keep the prev light dir to avoid uniform block updates
	Vec3 prevGroundLightDir = Vec3(0.0);

	/// @name For drawing a quad into the active framebuffer
	/// @{
	BufferObject quadPositionsVbo; ///< The VBO for quad positions
	Vao quadVao; ///< This VAO is used everywhere except material stage
	/// @}

	/// @name Limits
	/// @{
	U16 maxPointLights;
	U8 maxSpotLights;
	U8 maxSpotTexLights;

	U8 maxPointLightsPerTile;
	U8 maxSpotLightsPerTile;
	U8 maxSpotTexLightsPerTile;
	/// @}

	/// Called by init
	void initInternal(const RendererInitializer& initializer);

	/// Do the actual pass
	void lightPass();

	/// Prepare GL for rendering
	void setState();

	/// Calculate the size of the lights UBO
	PtrSize calcLightsUboSize() const;

	/// Calculate the size of the indices of point lights
	PtrSize calcPointLightIndicesBufferSize() const;

	/// Calculate the size of the indices of spot lights
	PtrSize calcSpotLightIndicesBufferSize() const;

	/// Setup the binding of the block and do some sanity checks on the size
	void blockSetupAndSanityCheck(const char* name, U binding, PtrSize size);
};

} // end namespace anki

#endif
