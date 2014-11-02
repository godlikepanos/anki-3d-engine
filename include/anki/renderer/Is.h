// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_IS_H
#define ANKI_RENDERER_IS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ProgramResource.h"
#include "anki/Gl.h"
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

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is: private RenderingPass
{
	friend class Renderer;
	friend class Sslr;
	friend class WriteLightsJob;

public:
	/// @privatesection
	/// @{
	GlTextureHandle& _getRt()
	{
		return m_rt;
	}
	/// @}

private:
	enum
	{
		COMMON_UNIFORMS_BLOCK_BINDING = 0,
		POINT_LIGHTS_BLOCK_BINDING = 1,
		SPOT_LIGHTS_BLOCK_BINDING = 2,
		SPOT_TEX_LIGHTS_BLOCK_BINDING = 3,
		TILES_BLOCK_BINDING = 4
	};

	/// The IS render target
	GlTextureHandle m_rt;

	/// The IS FBO
	GlFramebufferHandle m_fb;

	/// @name GPU buffers
	/// @{

	/// Contains common data for all shader programs
	GlBufferHandle m_commonBuff;

	/// Track the updates of commonUbo
	Timestamp m_commonBuffUpdateTimestamp = getGlobTimestamp();

	/// Contains all the lights
	GlBufferHandle m_lightsBuff;

	/// Contains the number of lights per tile
	GlBufferHandle m_tilesBuff;
	/// @}

	// Light shaders
	ProgramResourcePointer m_lightVert;
	ProgramResourcePointer m_lightFrag;
	GlProgramPipelineHandle m_lightPpline;

	/// Shadow mapping
	Sm m_sm;

	/// Opt because many ask for it
	Camera* m_cam;

	/// If enabled the ground emmits a light
	Bool8 m_groundLightEnabled;
	/// Keep the prev light dir to avoid uniform block updates
	Vec3 m_prevGroundLightDir = Vec3(0.0);

	/// @name For drawing a quad into the active framebuffer
	/// @{
	GlBufferHandle m_quadPositionsVertBuff;
	/// @}

	/// @name Limits
	/// @{
	U16 m_maxPointLights;
	U32 m_maxSpotLights;
	U32 m_maxSpotTexLights;

	U32 m_maxPointLightsPerTile;
	U32 m_maxSpotLightsPerTile;
	U32 m_maxSpotTexLightsPerTile;
	/// @}

	U32 m_tileSize; ///< Cache the value here

	Is(Renderer* r);
	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(GlCommandBufferHandle& cmdBuff);

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Do the actual pass
	ANKI_USE_RESULT Error lightPass(GlCommandBufferHandle& cmdBuff);

	/// Prepare GL for rendering
	void setState(GlCommandBufferHandle& cmdBuff);

	/// Calculate the size of the lights UBO
	PtrSize calcLightsBufferSize() const;

	/// Calculate the size of the tile
	PtrSize calcTileSize() const;

	ANKI_USE_RESULT Error updateCommonBlock(GlCommandBufferHandle& cmdBuff);
};

/// @}

} // end namespace anki

#endif
