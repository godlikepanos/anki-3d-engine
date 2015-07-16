// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderResource.h"
#include "anki/Gr.h"
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
class Is: public RenderingPass
{
	friend class WriteLightsTask;

public:
	static const U MIPMAPS_COUNT = 7;
	static const PixelFormat RT_PIXEL_FORMAT;

	/// @privatesection
	/// @{
	Is(Renderer* r);

	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(CommandBufferPtr& cmdBuff);

	TexturePtr& getRt()
	{
		return m_rt;
	}

	void generateMipmaps(CommandBufferPtr& cmdb)
	{
		ANKI_ASSERT(0 && "TODO");
		//m_rt.generateMipmaps(cmdb);
	}

	void setAmbientColor(const Vec4& color)
	{
		m_ambientColor = color;
	}

	FramebufferPtr& getFramebuffer()
	{
		return m_fb;
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

	U32 m_currentFrame = 0; ///< Cache value.

	/// The IS render target
	TexturePtr m_rt;

	/// The IS FBO
	FramebufferPtr m_fb;

	/// @name GPU buffers
	/// @{

	/// Contains common data for all shader programs
	BufferPtr m_commonBuffer;

	/// Track the updates of commonUbo
	Timestamp m_commonBuffUpdateTimestamp = 0;

	/// Contains all the point lights
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_pLightsBuffs;
	U32 m_pLightsBuffSize = 0;

	/// Contains all the spot lights
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_sLightsBuffs;
	U32 m_sLightsBuffSize = 0;

	/// Contains all the textured spot lights
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_stLightsBuffs;
	U32 m_stLightsBuffSize = 0;

	/// Contains the number of lights per tile
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_tilesBuffers;

	/// Contains light indices.
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_lightIdsBuffers;
	/// @}

	Array<ResourceGroupPtr, MAX_FRAMES_IN_FLIGHT> m_rcGroups;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	ShaderResourcePtr m_lightFrag;
	PipelinePtr m_lightPpline;

	/// Shadow mapping
	Sm m_sm;

	/// Opt because many ask for it
	SceneNode* m_cam;

	/// If enabled the ground emmits a light
	Bool8 m_groundLightEnabled;
	/// Keep the prev light dir to avoid uniform block updates
	Vec3 m_prevGroundLightDir = Vec3(0.0);

	/// @name Limits
	/// @{
	U16 m_maxPointLights;
	U32 m_maxSpotLights;
	U32 m_maxSpotTexLights;

	U32 m_maxLightIds;
	/// @}

	Vec4 m_ambientColor = Vec4(0.0);

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Do the actual pass
	ANKI_USE_RESULT Error lightPass(CommandBufferPtr& cmdBuff);

	/// Prepare GL for rendering
	void setState(CommandBufferPtr& cmdBuff);

	/// Calculate the size of the tile
	PtrSize calcTileSize() const;

	ANKI_USE_RESULT Error updateCommonBlock(CommandBufferPtr& cmdBuff);

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

