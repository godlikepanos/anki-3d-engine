// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/ReflectionProbe.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// ReflectionProbeMoveFeedbackComponent                                        =
//==============================================================================

/// Feedback component
class ReflectionProbeMoveFeedbackComponent: public SceneComponent
{
public:
	ReflectionProbeMoveFeedbackComponent(SceneNode* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
	{}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProbe& dnode = static_cast<ReflectionProbe&>(node);
			dnode.onMoveUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ReflectionProbe                                                             =
//==============================================================================

//==============================================================================
Error ReflectionProbe::create(const CString& name, F32 radius)
{
	SceneComponent* comp;

	ANKI_CHECK(SceneNode::create(name));

	// Move component first
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().
		newInstance<ReflectionProbeMoveFeedbackComponent>(this);
	addComponent(comp, true);

	// The frustum components
	Array<Transform, 6>& trfs = m_localFrustumTrfs;
	const F32 PI_2 = toRad(90.0);
	const F32 PI = toRad(180.0);
	trfs[0] = Transform(Vec4(0.0), Mat3x4(Euler(0.0, -PI_2, 0.0)), 1.0);
	trfs[1] = Transform(Vec4(0.0), Mat3x4(Euler(PI_2, 0.0, 0.0)), 1.0);
	trfs[2] = Transform(Vec4(0.0), Mat3x4(Euler(0.0, PI, 0.0)), 1.0);
	trfs[3] = Transform(Vec4(0.0), Mat3x4(Euler(0.0, PI_2, 0.0)), 1.0);
	trfs[4] = Transform(Vec4(0.0), Mat3x4(Euler(-PI_2, 0.0, 0.0)), 1.0);
	trfs[5] = Transform(Vec4(0.0), Mat3x4::getIdentity(), 1.0);

	for(U i = 0; i < 6; ++i)
	{
		comp = getSceneAllocator().newInstance<FrustumComponent>(
			this, &m_frustums[i]);
		addComponent(comp, true);

		m_frustums[i].setAll(toRad(45.0), toRad(45.0), 0.5, radius);
		m_frustums[i].resetTransform(trfs[i]);
	}

	// Spatial component
	m_spatialSphere.setCenter(Vec4(0.0));
	m_spatialSphere.setRadius(0.1);
	comp = getSceneAllocator().newInstance<SpatialComponent>(
		this, &m_spatialSphere);

	// Reflection probe
	comp = getSceneAllocator().newInstance<ReflectionProbeComponent>(this);
	addComponent(comp, true);

	// Create graphics objects
	createGraphics();

	return ErrorCode::NONE;
}

//==============================================================================
void ReflectionProbe::createGraphics()
{
	// Create textures
	TexturePtr::Initializer init;
	init.m_type = TextureType::CUBE;
	init.m_width = init.m_height = m_fbSize;
	init.m_format =
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
	init.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;

	CommandBufferPtr cmdb;
	cmdb.create(&getSceneGraph().getGrManager());

	m_colorTex.create(cmdb, init);

	init.m_format = PixelFormat(ComponentFormat::D16, TransformFormat::FLOAT);
	m_depthTex.create(cmdb, init);

	cmdb.flush();

	// Create framebuffers
	for(U i = 0; i < 6; ++i)
	{
		FramebufferPtr::Initializer fbInit;
		fbInit.m_colorAttachmentsCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_colorTex;
		fbInit.m_colorAttachments[0].m_layer = i;
		fbInit.m_colorAttachments[0].m_loadOperation =
			AttachmentLoadOperation::DONT_CARE;

		fbInit.m_depthStencilAttachment.m_texture = m_depthTex;
		fbInit.m_depthStencilAttachment.m_loadOperation =
			AttachmentLoadOperation::CLEAR;

		m_fbs[i].create(&getSceneGraph().getGrManager(), fbInit);
	}
}

//==============================================================================
void ReflectionProbe::onMoveUpdate(MoveComponent& move)
{
	// Update frustum components
	U count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& frc) -> Error
	{
		Transform trf = m_localFrustumTrfs[count].combineTransformations(
			move.getWorldTransform());

		frc.getFrustum().resetTransform(trf);

		frc.markTransformForUpdate();
		++count;

		return ErrorCode::NONE;
	});

	ANKI_ASSERT(count == 6);
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
}

} // end namespace anki
