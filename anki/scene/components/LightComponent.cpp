// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/LightComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Octree.h>
#include <anki/Collision.h>
#include <anki/shaders/include/ClusteredShadingTypes.h>

namespace anki
{

LightComponent::LightComponent(LightComponentType type, U64 uuid)
	: SceneComponent(CLASS_TYPE)
	, m_uuid(uuid)
	, m_type(type)
	, m_shadow(false)
	, m_componentDirty(true)
	, m_trfDirty(true)
{
	ANKI_ASSERT(m_uuid > 0);

	switch(type)
	{
	case LightComponentType::POINT:
		m_point.m_radius = 1.0f;
		break;
	case LightComponentType::SPOT:
		setInnerAngle(toRad(45.0f));
		setOuterAngle(toRad(30.0f));
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

	if(m_componentDirty)
	{
		updated = true;
	}

	if(m_trfDirty)
	{
		updated = true;

		if(m_type == LightComponentType::SPOT)
		{
			static const Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
			Mat4 proj = Mat4::calculatePerspectiveProjectionMatrix(m_spot.m_outerAngle, m_spot.m_outerAngle,
																   LIGHT_FRUSTUM_NEAR_PLANE, m_spot.m_distance);
			m_spot.m_textureMat = biasMat4 * proj * Mat4(m_trf.getInverse());
		}
	}

	m_trfDirty = false;
	m_componentDirty = false;

	// Update the scene bounds always
	if(m_type == LightComponentType::DIRECTIONAL)
	{
		node.getSceneGraph().getOctree().getActualSceneBounds(m_dir.m_sceneMin, m_dir.m_sceneMax);
	}

	return Error::NONE;
}

void LightComponent::setupDirectionalLightQueueElement(const FrustumComponent& frustumComp,
													   DirectionalLightQueueElement& el,
													   WeakArray<FrustumComponent> cascadeFrustumComponents) const
{
	ANKI_ASSERT(m_type == LightComponentType::DIRECTIONAL);
	ANKI_ASSERT(cascadeFrustumComponents.getSize() <= MAX_SHADOW_CASCADES);

	const U32 shadowCascadeCount = cascadeFrustumComponents.getSize();

	el.m_drawCallback = m_drawCallback;
	el.m_drawCallbackUserData = m_drawCallbackUserData;
	el.m_uuid = m_uuid;
	el.m_diffuseColor = m_diffColor.xyz();
	el.m_direction = -m_trf.getRotation().getZAxis().xyz();
	el.m_effectiveShadowDistance = frustumComp.getEffectiveShadowDistance();
	el.m_shadowCascadesDistancePower = frustumComp.getShadowCascadesDistancePower();
	el.m_shadowCascadeCount = U8(shadowCascadeCount);
	el.m_shadowLayer = MAX_U8;

	if(shadowCascadeCount == 0)
	{
		return;
	}

	// Compute the texture matrices
	const Mat4 lightTrf(m_trf);
	if(frustumComp.getFrustumType() == FrustumType::PERSPECTIVE)
	{
		// Get some stuff
		const F32 fovX = frustumComp.getFovX();
		const F32 fovY = frustumComp.getFovY();

		// Compute a sphere per cascade
		Array<Sphere, MAX_SHADOW_CASCADES> boundingSpheres;
		for(U32 i = 0; i < shadowCascadeCount; ++i)
		{
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
			const F32 f = frustumComp.computeShadowCascadeDistance(i); // Cascade far
			const F32 n =
				(i == 0) ? frustumComp.getNear() : frustumComp.computeShadowCascadeDistance(i - 1); // Cascade near
			const F32 a = f * tan(fovY / 2.0f) * fovX / fovY;
			const F32 b = n * tan(fovY / 2.0f) * fovX / fovY;
			const F32 z = (b * b + n * n - a * a - f * f) / (2.0f * (f - n));
			ANKI_ASSERT(absolute((Vec2(a, -f) - Vec2(0, z)).getLength() - (Vec2(b, -n) - Vec2(0, z)).getLength())
						<= EPSILON * 100.0f);

			Vec3 C(0.0f, 0.0f, z); // Sphere center

			// Compute the radius of the sphere
			const Vec3 A(a, tan(fovY / 2.0f) * f, -f);
			const F32 r = (A - C).getLength();

			// Set the sphere
			boundingSpheres[i].setRadius(r);
			boundingSpheres[i].setCenter(frustumComp.getTransform().transform(C));
		}

		// Compute the matrices
		for(U32 i = 0; i < shadowCascadeCount; ++i)
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
				const F32 t = testCollisionInside(sceneBox, Ray(sphereCenter, -lightDir));
				eye = sphereCenter + t * (-lightDir);
			}
			else
			{
				eye = sphereCenter + sphereRadius * (-lightDir);
			}

			// Projection
			const F32 far = (eye - sphereCenter).getLength() + sphereRadius;
			Mat4 cascadeProjMat = Mat4::calculateOrthographicProjectionMatrix(
				sphereRadius, -sphereRadius, sphereRadius, -sphereRadius, LIGHT_FRUSTUM_NEAR_PLANE, far);

			// View
			Transform cascadeTransform = m_trf;
			cascadeTransform.setOrigin(eye.xyz0());
			const Mat4 cascadeViewMat = Mat4(cascadeTransform.getInverse());

			// Now it's time to stabilize the shadows by aligning the projection matrix
			{
				// Project a random fixed point to the light matrix
				const Vec4 randomPointAlmostLightSpace = (cascadeProjMat * cascadeViewMat) * Vec3(0.0f).xyz1();

				// Chose a random low shadowmap size and align the random point
				const F32 shadowmapSize = 128.0f;
				const F32 shadowmapSize2 = shadowmapSize / 2.0f; // Div with 2 because the projected point is in NDC
				const F32 alignedX = std::round(randomPointAlmostLightSpace.x() * shadowmapSize2) / shadowmapSize2;
				const F32 alignedY = std::round(randomPointAlmostLightSpace.y() * shadowmapSize2) / shadowmapSize2;

				const F32 dx = alignedX - randomPointAlmostLightSpace.x();
				const F32 dy = alignedY - randomPointAlmostLightSpace.y();

				// Fix the projection matrix by applying an offset
				Mat4 correctionTranslationMat = Mat4::getIdentity();
				correctionTranslationMat.setTranslationPart(Vec4(dx, dy, 0, 1.0f));

				cascadeProjMat = correctionTranslationMat * cascadeProjMat;
			}

			// Light matrix
			static const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
									   0.0f, 0.0f, 1.0f);
			el.m_textureMatrices[i] = biasMat4 * cascadeProjMat * cascadeViewMat;

			// Fill the frustum with the fixed projection parameters from the fixed projection matrix
			Plane plane;
			extractClipPlane(cascadeProjMat, FrustumPlaneType::LEFT, plane);
			const F32 left = plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::RIGHT, plane);
			const F32 right = -plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::TOP, plane);
			const F32 top = -plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::BOTTOM, plane);
			const F32 bottom = plane.getOffset();

			FrustumComponent& cascadeFrustumComp = cascadeFrustumComponents[i];
			cascadeFrustumComp.setOrthographic(LIGHT_FRUSTUM_NEAR_PLANE, far, right, left, top, bottom);
			cascadeFrustumComp.setTransform(cascadeTransform);
		}
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

} // end namespace anki
