// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>
#include <anki/Gr.h>
#include <anki/Math.h>
#include <anki/renderer/Sm.h>
#include <anki/util/StdTypes.h>
#include <anki/util/Array.h>
#include <anki/core/Timestamp.h>
#include <anki/collision/Plane.h>

namespace anki
{

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
class ClustererTestResult;

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is : public RenderingPass
{
	friend class WriteLightsTask;

public:
	static const U MIPMAPS_COUNT = 7;

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Is(Renderer* r);

	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(CommandBufferPtr& cmdBuff);

	TexturePtr getRt() const
	{
		return m_rt;
	}

	void generateMipmaps(CommandBufferPtr& cmdb)
	{
		cmdb->generateMipmaps(m_rt);
	}

	void setAmbientColor(const Vec4& color)
	{
		m_ambientColor = color;
	}

	Sm& getSm()
	{
		return m_sm;
	}

	DynamicBufferToken getCommonVarsToken() const
	{
		return m_commonVarsToken;
	}

	DynamicBufferToken getPointLightsToken() const
	{
		return m_pLightsToken;
	}

	DynamicBufferToken getSpotLightsToken() const
	{
		return m_sLightsToken;
	}

	DynamicBufferToken getClustersToken() const
	{
		return m_clustersToken;
	}

	DynamicBufferToken getLightIndicesToken() const
	{
		return m_lightIdsToken;
	}

private:
	U32 m_currentFrame = 0; ///< Cache value.

	/// The IS render target
	TexturePtr m_rt;

	/// The IS FBO
	FramebufferPtr m_fb;

	DynamicBufferToken m_commonVarsToken;
	DynamicBufferToken m_pLightsToken;
	DynamicBufferToken m_sLightsToken;
	DynamicBufferToken m_clustersToken;
	DynamicBufferToken m_lightIdsToken;

	ResourceGroupPtr m_rcGroup;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	ShaderResourcePtr m_lightFrag;
	PipelinePtr m_lightPpline;

	/// Shadow mapping
	Sm m_sm;

	/// Opt because many ask for it
	FrustumComponent* m_frc = nullptr;

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

	WeakPtr<Barrier> m_barrier;

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Do the actual pass
	ANKI_USE_RESULT Error lightPass(CommandBufferPtr& cmdBuff);

	/// Prepare GL for rendering
	void setState(CommandBufferPtr& cmdBuff);

	void updateCommonBlock(
		CommandBufferPtr& cmdBuff, const FrustumComponent& frc);

	// Binning
	void binLights(U32 threadId, PtrSize threadsCount, TaskCommonData& data);
	I writePointLight(const LightComponent& light,
		const MoveComponent& move,
		const FrustumComponent& camfrc,
		TaskCommonData& task);
	I writeSpotLight(const LightComponent& lightc,
		const MoveComponent& lightMove,
		const FrustumComponent* lightFrc,
		const MoveComponent& camMove,
		const FrustumComponent& camFrc,
		TaskCommonData& task);
	void binLight(SpatialComponent& sp,
		U pos,
		U lightType,
		TaskCommonData& task,
		ClustererTestResult& testResult);
};

/// @}

} // end namespace anki
