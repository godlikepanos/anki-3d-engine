// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>
#include <anki/Gr.h>
#include <anki/Math.h>
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
class SceneNode;

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is : public RenderingPass
{
	friend class WriteLightsTask;

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Is(Renderer* r);

	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate light buffers.
	ANKI_USE_RESULT Error populateBuffers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	TexturePtr getRt() const
	{
		return m_rt;
	}

	void generateMipmaps(CommandBufferPtr& cmdb)
	{
		cmdb->generateMipmaps(m_rt, 0, 0);
	}

private:
	static const U COMMON_VARS_LOCATION = 0;
	static const U P_LIGHTS_LOCATION = 1;
	static const U S_LIGHTS_LOCATION = 2;
	static const U PROBES_LOCATION = 3;
	static const U CLUSTERS_LOCATION = 0;
	static const U LIGHT_IDS_LOCATION = 1;

	/// The IS render target
	TexturePtr m_rt;

	/// The IS FBO
	FramebufferPtr m_fb;

	ResourceGroupPtr m_rcGroup;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	ShaderResourcePtr m_lightFrag;
	PipelinePtr m_lightPpline;

	/// Opt because many ask for it
	FrustumComponent* m_frc = nullptr;

	/// If enabled the ground emmits a light
	Bool8 m_groundLightEnabled;
	/// Keep the prev light dir to avoid uniform block updates
	Vec3 m_prevGroundLightDir = Vec3(0.0);

	/// @name Limits
	/// @{
	U32 m_maxLightIds;
	/// @}

	WeakPtr<Barrier> m_barrier;

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Prepare GL for rendering
	void setState(const RenderingContext& ctx, CommandBufferPtr& cmdBuff);

	void updateCommonBlock(const FrustumComponent& frc, RenderingContext& ctx);

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

	void writeAndBinProbe(const FrustumComponent& camFrc,
		const SceneNode& node,
		TaskCommonData& task,
		ClustererTestResult& testResult);
};

/// @}

} // end namespace anki
