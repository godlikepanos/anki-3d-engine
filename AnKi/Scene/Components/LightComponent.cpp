// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Octree.h>
#include <AnKi/Collision.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

LightComponent::LightComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_spatial(this)
	, m_type(LightComponentType::kPoint)
{
	ANKI_ASSERT(m_uuid > 0);
	m_point.m_radius = 1.0f;

	setLightComponentType(LightComponentType::kPoint);
	m_worldTransform = node->getWorldTransform();
}

LightComponent::~LightComponent()
{
}

void LightComponent::setLightComponentType(LightComponentType type)
{
	ANKI_ASSERT(type >= LightComponentType::kFirst && type < LightComponentType::kCount);
	m_type = type;
	m_markedForUpdate = true;
	m_forceFullUpdate = true;

	if(type == LightComponentType::kDirectional)
	{
		m_spatial.setAlwaysVisible(true);
		m_spatial.setUpdatesOctreeBounds(false);
	}
	else
	{
		m_spatial.setAlwaysVisible(false);
		m_spatial.setUpdatesOctreeBounds(true);
	}
}

Error LightComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool moveUpdated = info.m_node->movedThisFrame() || m_forceFullUpdate;
	const Bool shapeUpdated = m_markedForUpdate || m_forceFullUpdate;
	updated = moveUpdated || shapeUpdated;
	m_markedForUpdate = false;
	m_forceFullUpdate = false;

	if(moveUpdated)
	{
		m_worldTransform = info.m_node->getWorldTransform();
	}

	if(updated && m_type == LightComponentType::kPoint)
	{
		const Sphere sphere(m_worldTransform.getOrigin(), m_point.m_radius);
		m_spatial.setBoundingShape(sphere);

		if(m_shadow)
		{
			if(m_frustums == nullptr || m_frustumCount != 6) [[unlikely]]
			{
				// Allocate, initialize and update the frustums, just do everything to avoid bugs
				deleteArray(info.m_node->getMemoryPool(), m_frustums, m_frustumCount);
				m_frustums = newArray<Frustum>(info.m_node->getMemoryPool(), 6);
				m_frustumCount = 6;

				for(U32 i = 0; i < 6; i++)
				{
					m_frustums[i].init(FrustumType::kPerspective, &info.m_node->getMemoryPool());
					m_frustums[i].setPerspective(kClusterObjectFrustumNearPlane, m_point.m_radius, kPi / 2.0f,
												 kPi / 2.0f);
					m_frustums[i].setWorldTransform(Transform(m_worldTransform.getOrigin(),
															  Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));
				}
			}

			// Update the frustums
			for(U32 i = 0; i < 6; i++)
			{
				if(shapeUpdated)
				{
					m_frustums[i].setFar(m_point.m_radius);
				}

				if(moveUpdated || shapeUpdated)
				{
					m_frustums[i].setWorldTransform(Transform(m_worldTransform.getOrigin(),
															  Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));
				}
			}
		}
	}
	else if(updated && m_type == LightComponentType::kSpot)
	{
		// Update texture matrix
		const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
							1.0f);
		const Mat4 proj = Mat4::calculatePerspectiveProjectionMatrix(m_spot.m_outerAngle, m_spot.m_outerAngle,
																	 kClusterObjectFrustumNearPlane, m_spot.m_distance);
		m_spot.m_textureMat = biasMat4 * proj * Mat4(m_worldTransform.getInverse());

		// Update the spatial
		Array<Vec4, 4> points;
		computeEdgesOfFrustum(m_spot.m_distance, m_spot.m_outerAngle, m_spot.m_outerAngle, &points[0]);
		Array<Vec3, 5> worldPoints;
		for(U32 i = 0; i < 4; ++i)
		{
			m_spot.m_edgePointsWspace[i] = m_worldTransform.transform(points[i].xyz());
			worldPoints[i] = m_spot.m_edgePointsWspace[i].xyz();
		}
		worldPoints[4] = m_worldTransform.getOrigin().xyz();
		m_spatial.setBoundingShape(ConstWeakArray<Vec3>(worldPoints));

		if(m_shadow)
		{
			if(m_frustums == nullptr || m_frustumCount != 1) [[unlikely]]
			{
				// Allocate, initialize and update the frustums, just do everything to avoid bugs
				deleteArray(info.m_node->getMemoryPool(), m_frustums, m_frustumCount);
				m_frustums = newArray<Frustum>(info.m_node->getMemoryPool(), 1);
				m_frustumCount = 1;

				m_frustums[0].init(FrustumType::kPerspective, &info.m_node->getMemoryPool());
				m_frustums[0].setPerspective(kClusterObjectFrustumNearPlane, m_spot.m_distance, m_spot.m_outerAngle,
											 m_spot.m_outerAngle);
				m_frustums[0].setWorldTransform(m_worldTransform);
			}

			// Update the frustum
			if(shapeUpdated)
			{
				m_frustums[0].setFar(m_spot.m_distance);
				m_frustums[0].setFovX(m_spot.m_outerAngle);
				m_frustums[0].setFovY(m_spot.m_outerAngle);
			}

			if(moveUpdated)
			{
				m_frustums[0].setWorldTransform(m_worldTransform);
			}
		}
	}
	else if(m_type == LightComponentType::kDirectional)
	{
		// Update the scene bounds always
		info.m_node->getSceneGraph().getOctree().getActualSceneBounds(m_dir.m_sceneMin, m_dir.m_sceneMax);
	}

	const Bool spatialUpdated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
	updated = updated || spatialUpdated;

	if(m_shadow)
	{
		for(U32 i = 0; i < m_frustumCount; ++i)
		{
			const Bool frustumUpdated = m_frustums[i].update();
			updated = updated || frustumUpdated;
		}
	}

	return Error::kNone;
}

void LightComponent::setupDirectionalLightQueueElement(const Frustum& primaryFrustum, DirectionalLightQueueElement& el,
													   WeakArray<Frustum> cascadeFrustums) const
{
	ANKI_ASSERT(m_type == LightComponentType::kDirectional);
	ANKI_ASSERT(cascadeFrustums.getSize() <= kMaxShadowCascades);

	const U32 shadowCascadeCount = cascadeFrustums.getSize();

	el.m_uuid = m_uuid;
	el.m_diffuseColor = m_diffColor.xyz();
	el.m_direction = -m_worldTransform.getRotation().getZAxis().xyz();
	for(U32 i = 0; i < shadowCascadeCount; ++i)
	{
		el.m_shadowCascadesDistances[i] = primaryFrustum.getShadowCascadeDistance(i);
	}
	el.m_shadowCascadeCount = U8(shadowCascadeCount);
	el.m_shadowLayer = kMaxU8;

	if(shadowCascadeCount == 0)
	{
		return;
	}

	// Compute the texture matrices
	const Mat4 lightTrf(m_worldTransform);
	if(primaryFrustum.getFrustumType() == FrustumType::kPerspective)
	{
		// Get some stuff
		const F32 fovX = primaryFrustum.getFovX();
		const F32 fovY = primaryFrustum.getFovY();

		// Compute a sphere per cascade
		Array<Sphere, kMaxShadowCascades> boundingSpheres;
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
			const F32 f = primaryFrustum.getShadowCascadeDistance(i); // Cascade far
			const F32 n =
				(i == 0) ? primaryFrustum.getNear() : primaryFrustum.getShadowCascadeDistance(i - 1); // Cascade near
			const F32 a = f * tan(fovY / 2.0f) * fovX / fovY;
			const F32 b = n * tan(fovY / 2.0f) * fovX / fovY;
			const F32 z = (b * b + n * n - a * a - f * f) / (2.0f * (f - n));
			ANKI_ASSERT(absolute((Vec2(a, -f) - Vec2(0, z)).getLength() - (Vec2(b, -n) - Vec2(0, z)).getLength())
						<= kEpsilonf * 100.0f);

			Vec3 C(0.0f, 0.0f, z); // Sphere center

			// Compute the radius of the sphere
			const Vec3 A(a, tan(fovY / 2.0f) * f, -f);
			const F32 r = (A - C).getLength();

			// Set the sphere
			boundingSpheres[i].setRadius(r);
			boundingSpheres[i].setCenter(primaryFrustum.getWorldTransform().transform(C));
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
				sphereRadius, -sphereRadius, sphereRadius, -sphereRadius, kClusterObjectFrustumNearPlane, far);

			// View
			Transform cascadeTransform = m_worldTransform;
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
			const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f);
			el.m_textureMatrices[i] = biasMat4 * cascadeProjMat * cascadeViewMat;

			// Fill the frustum with the fixed projection parameters from the fixed projection matrix
			Plane plane;
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kLeft, plane);
			const F32 left = plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kRight, plane);
			const F32 right = -plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kTop, plane);
			const F32 top = -plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kBottom, plane);
			const F32 bottom = plane.getOffset();

			Frustum& cascadeFrustum = cascadeFrustums[i];
			cascadeFrustum.init(FrustumType::kOrthographic, nullptr);
			cascadeFrustum.setOrthographic(kClusterObjectFrustumNearPlane, far, right, left, top, bottom);
			cascadeFrustum.setWorldTransform(cascadeTransform);
			[[maybe_unused]] const Bool updated = cascadeFrustum.update();
			ANKI_ASSERT(updated);
		}
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

void LightComponent::onDestroy(SceneNode& node)
{
	deleteArray(node.getMemoryPool(), m_frustums, m_frustumCount);
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());
}

} // end namespace anki
