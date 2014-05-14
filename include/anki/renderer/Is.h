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
	const GlTextureHandle& _getRt() const
	{
		return m_rt;
	}

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

	enum
	{
		MS_RT0_TEX_UNIT = 0,
		MS_RT1_TEX_UNIT = 1,
		MS_DEPTH_RT_TEX_UNIT = 2,
		SM_ARRAY_TEX_UNIT = 3
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
	U8 m_maxSpotLights;
	U8 m_maxSpotTexLights;

	U8 m_maxPointLightsPerTile;
	U8 m_maxSpotLightsPerTile;
	U8 m_maxSpotTexLightsPerTile;
	/// @}

	U32 m_tileSize; ///< Cache the value here

	Is(Renderer* r);
	~Is();

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);

	/// Called by init
	void initInternal(const RendererInitializer& initializer);

	/// Do the actual pass
	void lightPass(GlJobChainHandle& jobs);

	/// Prepare GL for rendering
	void setState(GlJobChainHandle& jobs);

	/// Calculate the size of the lights UBO
	PtrSize calcLightsBufferSize() const;

	/// Calculate the size of the tile
	PtrSize calcTileSize() const;

	void updateCommonBlock(GlJobChainHandle& jobs);
};

/// @}

} // end namespace anki

#endif
