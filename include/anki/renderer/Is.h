// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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

// Forward
class Camera;
class PerspectiveCamera;
class PointLight;
class SpotLight;
class LightComponent;
class MoveComponent;
class SpatialComponent;
class FrustumComponent;
class TaskCommonData;

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is: private RenderingPass
{
	friend class Renderer;
	friend class Sslr;
	friend class WriteLightsTask;

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
		TILES_BLOCK_BINDING = 4,
		LIGHT_IDS_BLOCK_BINDING = 5
	};

	static const U MAX_FRAMES = 3;
	U32 m_currentFrame = 0; ///< Cache value.

	/// The IS render target
	GlTextureHandle m_rt;

	/// The IS FBO
	GlFramebufferHandle m_fb;

	/// @name GPU buffers
	/// @{

	/// Contains common data for all shader programs
	GlBufferHandle m_commonBuffer;

	/// Track the updates of commonUbo
	Timestamp m_commonBuffUpdateTimestamp = 0;

	/// Contains all the lights
	Array<GlBufferHandle, MAX_FRAMES> m_lightsBuffers;
	Array<void*, MAX_FRAMES> m_lightsBufferAddresses;

	/// Contains the number of lights per tile
	Array<GlBufferHandle, MAX_FRAMES> m_tilesBuffers;
	Array<void*, MAX_FRAMES> m_tilesBufferAddresses;

	/// Contains light indices.
	Array<GlBufferHandle, MAX_FRAMES> m_lightIdsBuffers;
	Array<void*, MAX_FRAMES> m_lightIdsBufferAddresses;
	/// @}

	// Light shaders
	ProgramResourcePointer m_lightVert;
	ProgramResourcePointer m_lightFrag;
	GlPipelineHandle m_lightPpline;

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

	U32 m_maxLightIds;
	/// @}

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

	// Binning
	void binLights(U32 threadId, PtrSize threadsCount, TaskCommonData& data);
	I writePointLight(const LightComponent& light, const MoveComponent& move,
		const FrustumComponent& camfrc, TaskCommonData& task);
	I writeSpotLight(const LightComponent& lightc, 
		const MoveComponent& lightMove, const FrustumComponent* lightFrc, 
		const MoveComponent& camMove, const FrustumComponent& camFrc, 
		TaskCommonData& task);
	void binLight(SpatialComponent& sp, U pos, U lightType, 
		TaskCommonData& task);
};

/// @}

} // end namespace anki

#endif
