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
	/// @}

private:
	static const U COMMON_UNIFORMS_BLOCK_BINDING = 0;
	static const U POINT_LIGHTS_BLOCK_BINDING = 1;
	static const U SPOT_LIGHTS_BLOCK_BINDING = 2;
	static const U SPOT_TEX_LIGHTS_BLOCK_BINDING = 3;
	static const U TILES_BLOCK_BINDING = 4;

	/// The IS FAI
	Texture fai;

	/// The IS FBO
	Fbo fbo;

	/// @name GPU buffers
	/// @{

	/// Contains common data for all shader programs
	Ubo commonUbo;

	/// Track the updates of commonUbo
	Timestamp commonUboUpdateTimestamp = getGlobTimestamp();

	/// Contains all the lights
	Ubo lightsUbo;
	PtrSize uboAlignment;

	/// Contains the indices of lights per tile
	BufferObject tilesBuffer;
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
	Vbo quadPositionsVbo; ///< The VBO for quad positions
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

	/// Calculate the size of the tiles UBO
	PtrSize calcTileSize() const;

	/// Calculate the size of the tiles UBO
	PtrSize calcTilesUboSize() const;
};

} // end namespace anki

#endif
