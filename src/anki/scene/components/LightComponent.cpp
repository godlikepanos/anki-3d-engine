// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/LightComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Octree.h>
#include <anki/collision/Frustum.h>
#include <shaders/glsl_cpp_common/ClusteredShading.h>

namespace anki
{

LightComponent::LightComponent(LightComponentType type, U64 uuid)
	: SceneComponent(CLASS_TYPE)
	, m_uuid(uuid)
	, m_type(type)
{
	ANKI_ASSERT(m_uuid > 0);

	switch(type)
	{
	case LightComponentType::POINT:
		m_point.m_radius = 1.0f;
		break;
	case LightComponentType::SPOT:
		setInnerAngle(toRad(45.0));
		setOuterAngle(toRad(30.0));
		m_spot.m_distance = 1.0f;
		m_spot.m_textureMat = Mat4::getIdentity();
		break;
	case LightComponentType::DIRECTIONAL:
		m_dir.m_sceneAabbMaxZLightSpace = MIN_F32;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

Error LightComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	updated = false;

	if(m_flags.get(DIRTY))
	{
		updated = true;
	}

	if(m_flags.get(TRF_DIRTY))
	{
		updated = true;

		if(m_type == LightComponentType::SPOT)
		{
			static const Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
			Mat4 proj = Mat4::calculatePerspectiveProjectionMatrix(
				m_spot.m_outerAngle, m_spot.m_outerAngle, LIGHT_FRUSTUM_NEAR_PLANE, m_spot.m_distance);
			m_spot.m_textureMat = biasMat4 * proj * Mat4(m_trf.getInverse());
		}
	}

	m_flags.unset(DIRTY | TRF_DIRTY);

	// Update the scene bounds always
	if(m_type == LightComponentType::DIRECTIONAL)
	{
		// Get some values fro the octree
		const Vec3& sceneWSpaceMin = node.getSceneGraph().getOctree().getActualSceneMinBound();
		const Vec3& sceneWSpaceMax = node.getSceneGraph().getOctree().getActualSceneMaxBound();

		// Gather the 8 scene points
		Array<Vec4, 8> sceneEdgesWSpace;
		sceneEdgesWSpace[0] = Vec4(sceneWSpaceMin.x(), sceneWSpaceMin.y(), sceneWSpaceMin.z(), 1.0f);
		sceneEdgesWSpace[1] = Vec4(sceneWSpaceMin.x(), sceneWSpaceMin.y(), sceneWSpaceMax.z(), 1.0f);
		sceneEdgesWSpace[2] = Vec4(sceneWSpaceMin.x(), sceneWSpaceMax.y(), sceneWSpaceMin.z(), 1.0f);
		sceneEdgesWSpace[3] = Vec4(sceneWSpaceMin.x(), sceneWSpaceMax.y(), sceneWSpaceMax.z(), 1.0f);
		sceneEdgesWSpace[4] = Vec4(sceneWSpaceMax.x(), sceneWSpaceMin.y(), sceneWSpaceMin.z(), 1.0f);
		sceneEdgesWSpace[5] = Vec4(sceneWSpaceMax.x(), sceneWSpaceMin.y(), sceneWSpaceMax.z(), 1.0f);
		sceneEdgesWSpace[6] = Vec4(sceneWSpaceMax.x(), sceneWSpaceMax.y(), sceneWSpaceMin.z(), 1.0f);
		sceneEdgesWSpace[7] = Vec4(sceneWSpaceMax.x(), sceneWSpaceMax.y(), sceneWSpaceMax.z(), 1.0f);

		// Transform the 8 scene points to light space
		m_dir.m_sceneAabbMaxZLightSpace = MIN_F32;
		const Mat4 viewMat = Mat4(m_trf).getInverse();
		for(const Vec4& point : sceneEdgesWSpace)
		{
			const Vec4 lightSpaceEdge = viewMat * point;

			m_dir.m_sceneAabbMaxZLightSpace = max(m_dir.m_sceneAabbMaxZLightSpace, lightSpaceEdge.z());
		}
	}

	return Error::NONE;
}

void LightComponent::setupDirectionalLightQueueElement(const Frustum& frustum,
	F32 overrideFrustumFar,
	DirectionalLightQueueElement& el,
	WeakArray<OrthographicFrustum> cascadeFrustums) const
{
	ANKI_ASSERT(m_type == LightComponentType::DIRECTIONAL);
	ANKI_ASSERT(cascadeFrustums.getSize() <= MAX_SHADOW_CASCADES);

	const U shadowCascadeCount = cascadeFrustums.getSize();

	el.m_userData = this;
	el.m_drawCallback = derectionalLightDebugDrawCallback;
	el.m_uuid = m_uuid;
	el.m_diffuseColor = m_diffColor.xyz();
	el.m_direction = m_trf.getRotation().getZAxis().xyz();
	el.m_shadowCascadeCount = shadowCascadeCount;

	// Compute the texture matrices
	if(shadowCascadeCount == 0)
	{
		return;
	}

	const Mat4 lightTrf(m_trf);
	const Mat4 lightViewMat(lightTrf.getInverse());
	if(frustum.getType() == FrustumType::PERSPECTIVE)
	{
		// Get some stuff
		const PerspectiveFrustum& pfrustum = static_cast<const PerspectiveFrustum&>(frustum);
		const F32 fovX = pfrustum.getFovX();
		const F32 fovY = pfrustum.getFovY();
		const F32 far = (overrideFrustumFar > 0.0f) ? overrideFrustumFar : pfrustum.getFar();

		// Gather the edges
		Array<Vec4, (MAX_SHADOW_CASCADES + 1) * 4> edgesLocalSpaceStorage;
		WeakArray<Vec4> edgesLocalSpace(&edgesLocalSpaceStorage[0], (shadowCascadeCount + 1) * 4);
		edgesLocalSpace[0] = edgesLocalSpace[1] = edgesLocalSpace[2] = edgesLocalSpace[3] =
			Vec4(0.0f, 0.0f, 0.0f, 1.0f); // Eye
		for(U i = 0; i < shadowCascadeCount; ++i)
		{
			const F32 cascadeFarNearDist = far / F32(shadowCascadeCount);
			const F32 cascadeFar = F32(i + 1) * cascadeFarNearDist;

			const F32 x = cascadeFar * tan(fovY / 2.0f) * fovX / fovY;
			const F32 y = tan(fovY / 2.0f) * cascadeFar;
			const F32 z = -cascadeFar;

			U idx = (i + 1) * 4;
			edgesLocalSpace[idx++] = Vec4(x, y, z, 1.0f); // top right
			edgesLocalSpace[idx++] = Vec4(-x, y, z, 1.0f); // top left
			edgesLocalSpace[idx++] = Vec4(-x, -y, z, 1.0f); // bot left
			edgesLocalSpace[idx++] = Vec4(x, -y, z, 1.0f); // bot right
		}

		// Transform those edges to light space
		Array<Vec4, edgesLocalSpaceStorage.getSize()> edgesLightSpaceStorage;
		WeakArray<Vec4> edgesLightSpace(&edgesLightSpaceStorage[0], edgesLocalSpace.getSize());
		const Mat4 lspaceMtx = lightViewMat * Mat4(frustum.getTransform());
		for(U i = 0; i < edgesLightSpace.getSize(); ++i)
		{
			edgesLightSpace[i] = lspaceMtx * edgesLocalSpace[i];
		}

		// Compute the min max per cascade
		Array2d<Vec3, MAX_SHADOW_CASCADES, 2> minMaxes;
		for(U i = 0; i < shadowCascadeCount; ++i)
		{
			Vec3 aabbMin(MAX_F32);
			Vec3 aabbMax(MIN_F32);
			for(U j = i * 4; j < i * 4 + 8; ++j)
			{
				aabbMin = aabbMin.min(edgesLightSpace[j].xyz());
				aabbMax = aabbMax.max(edgesLightSpace[j].xyz());
			}

			// Adjust the max to take into account the scene bounds
			aabbMax.z() = max(aabbMax.z(), m_dir.m_sceneAabbMaxZLightSpace);

			// Align it to avoid flickering
			const PtrSize alignmentMeters = 2;
			for(U j = 0; j < 3; ++j)
			{
				aabbMin[j] = getAlignedRoundDown(alignmentMeters, I32(aabbMin[j]));
				aabbMax[j] = getAlignedRoundUp(alignmentMeters, I32(aabbMax[j]));
			}

			ANKI_ASSERT(aabbMax > aabbMin);
			minMaxes[i][0] = aabbMin;
			minMaxes[i][1] = aabbMax;
		}

		// Compute the view and projection matrices per cascade
		for(U i = 0; i < shadowCascadeCount; ++i)
		{
			const Vec3& min = minMaxes[i][0];
			const Vec3& max = minMaxes[i][1];

			const Vec2 halfDistances = (max.xy() - min.xy()) / 2.0f;
			ANKI_ASSERT(halfDistances > Vec2(0.0f));

			const Mat4 cascadeProjMat = Mat4::calculateOrthographicProjectionMatrix(halfDistances.x(),
				-halfDistances.x(),
				halfDistances.y(),
				-halfDistances.y(),
				LIGHT_FRUSTUM_NEAR_PLANE,
				max.z() - min.z() - LIGHT_FRUSTUM_NEAR_PLANE);

			Vec4 eye;
			eye.x() = (max.x() + min.x()) / 2.0f;
			eye.y() = (max.y() + min.y()) / 2.0f;
			eye.z() = max.z();
			eye.w() = 1.0f;
			eye = lightTrf * eye;

			Transform cascadeTransform = m_trf;
			cascadeTransform.setOrigin(eye.xyz0());
			const Mat4 cascadeViewMat = Mat4(cascadeTransform.getInverse());

			static const Mat4 biasMat4(
				0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			el.m_textureMatrices[i] = biasMat4 * cascadeProjMat * cascadeViewMat;

			// Fill the frustum
			OrthographicFrustum& cascadeFrustum = cascadeFrustums[i];
			cascadeFrustum.setAll(-halfDistances.x(),
				halfDistances.x(),
				LIGHT_FRUSTUM_NEAR_PLANE,
				max.z() - min.z() - LIGHT_FRUSTUM_NEAR_PLANE,
				halfDistances.y(),
				-halfDistances.y());
			cascadeFrustum.transform(cascadeTransform);
		}
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

} // end namespace anki
