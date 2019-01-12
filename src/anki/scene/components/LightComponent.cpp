// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/LightComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Octree.h>
#include <anki/collision/Frustum.h>
#include <anki/collision/Sphere.h>
#include <anki/collision/Plane.h>
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
		m_dir.m_sceneMax = Vec3(MIN_F32);
		m_dir.m_sceneMin = Vec3(MAX_F32);
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
		node.getSceneGraph().getOctree().getActualSceneBounds(m_dir.m_sceneMin, m_dir.m_sceneMax);
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
	el.m_direction = -m_trf.getRotation().getZAxis().xyz();
	el.m_shadowCascadeCount = shadowCascadeCount;

	// Compute the texture matrices
	if(shadowCascadeCount == 0)
	{
		return;
	}

	const Mat4 lightTrf(m_trf);
	if(frustum.getType() == FrustumType::PERSPECTIVE)
	{
		// Get some stuff
		const PerspectiveFrustum& pfrustum = static_cast<const PerspectiveFrustum&>(frustum);
		const F32 fovX = pfrustum.getFovX();
		const F32 fovY = pfrustum.getFovY();
		const F32 far = (overrideFrustumFar > 0.0f) ? overrideFrustumFar : pfrustum.getFar();

		// Compute a sphere per cascade
		Array<Sphere, MAX_SHADOW_CASCADES> boundingSpheres;
		for(U i = 0; i < shadowCascadeCount; ++i)
		{
			const F32 cascadeFarNearDist = far / F32(shadowCascadeCount);

			// Compute the center of the sphere
			//           ^ z
			//           |
			// ----------|---------- A(a, -f)
			//  \        |        /
			//   \       |       /
			//    \    C(0,z)   /
			//     \     |     /
			//      \    |    /
			//       \---|---/ B(b, -n)
			//        \  |  /
			//         \ | /
			//           v
			// --------------------------> x
			//           |
			// The square distance of A-C is equal to B-C. Solve the equation to find the z.
			const F32 f = F32(i + 1) * cascadeFarNearDist; // Cascade far
			const F32 n = max(frustum.getNear(), F32(i) * cascadeFarNearDist); // Cascade near
			const F32 a = f * tan(fovY / 2.0f) * fovX / fovY;
			const F32 b = n * tan(fovY / 2.0f) * fovX / fovY;
			const F32 z = (b * b + n * n - a * a - f * f) / (2.0f * (f - n));
			ANKI_ASSERT(isZero((Vec2(a, -f) - Vec2(0, z)).getLength() - (Vec2(b, -n) - Vec2(0, z)).getLength()));

			Vec3 C(0.0f, 0.0f, z); // Sphere center

			// Compute the radius of the sphere
			const Vec3 A(a, tan(fovY / 2.0f) * f, -f);
			const F32 r = (A - C).getLength();

			// Set the sphere
			boundingSpheres[i].setRadius(r);
			boundingSpheres[i].setCenter(frustum.getTransform().transform(C));
		}

		// Compute the matrices
		for(U i = 0; i < shadowCascadeCount; ++i)
		{
			const Sphere& sphere = boundingSpheres[i];
			const Vec3 sphereCenter = sphere.getCenter().xyz();
			const F32 sphereRadius = sphere.getRadius();
			const Vec3& lightDir = el.m_direction;
			const Vec3 sceneMin = m_dir.m_sceneMin - Vec3(sphereRadius); // Push the bounds a bit
			const Vec3 sceneMax = m_dir.m_sceneMax + Vec3(sphereRadius);

			// Compute the intersections with the scene bounds
			Vec3 eye;
			if(sphereCenter > sceneMin && sphereCenter < sceneMax)
			{
				// Inside the scene bounds
				const Aabb sceneBox(sceneMin, sceneMax);
				const F32 t = sceneBox.intersectRayInside(sphereCenter, -lightDir);
				eye = sphereCenter + t * (-lightDir);
			}
			else
			{
				eye = sphereCenter + sphereRadius * (-lightDir);
			}

			// Projection
			const F32 far = (eye - sphereCenter).getLength() + sphereRadius;
			const Mat4 cascadeProjMat = Mat4::calculateOrthographicProjectionMatrix(
				sphereRadius, -sphereRadius, sphereRadius, -sphereRadius, LIGHT_FRUSTUM_NEAR_PLANE, far);

			// View
			Transform cascadeTransform = m_trf;
			cascadeTransform.setOrigin(eye.xyz0());
			const Mat4 cascadeViewMat = Mat4(cascadeTransform.getInverse());

			// Light matrix
			static const Mat4 biasMat4(
				0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			el.m_textureMatrices[i] = biasMat4 * cascadeProjMat * cascadeViewMat;

			// Fill the frustum
			OrthographicFrustum& cascadeFrustum = cascadeFrustums[i];
			cascadeFrustum.setAll(
				-sphereRadius, sphereRadius, LIGHT_FRUSTUM_NEAR_PLANE, far, sphereRadius, -sphereRadius);
			cascadeFrustum.transform(cascadeTransform);
		}
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

} // end namespace anki
