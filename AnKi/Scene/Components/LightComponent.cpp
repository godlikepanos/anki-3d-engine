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
	: SceneComponent(node, kClassType)
	, m_type(LightComponentType::kPoint)
{
	m_point.m_radius = 1.0f;

	setLightComponentType(LightComponentType::kPoint);
	m_worldTransform = node->getWorldTransform();
}

LightComponent::~LightComponent()
{
	if(m_type == LightComponentType::kDirectional)
	{
		SceneGraph::getSingleton().removeDirectionalLight(this);
	}
}

void LightComponent::setLightComponentType(LightComponentType newType)
{
	ANKI_ASSERT(newType >= LightComponentType::kFirst && newType < LightComponentType::kCount);
	const LightComponentType oldType = m_type;
	const Bool typeChanged = newType != oldType;

	if(typeChanged)
	{
		m_type = newType;
		m_shadowAtlasUvViewportCount = 0;
		m_dirty = true;
		m_uuid = 0;

		if(newType == LightComponentType::kDirectional)
		{
			// Now it's directional, inform the scene
			SceneGraph::getSingleton().addDirectionalLight(this);
		}
		else if(oldType == LightComponentType::kDirectional)
		{
			// It was directional, inform the scene
			SceneGraph::getSingleton().removeDirectionalLight(this);
		}
	}
}

Error LightComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool moveUpdated = info.m_node->movedThisFrame();
	updated = moveUpdated || m_dirty;
	m_dirty = false;

	if(moveUpdated)
	{
		m_worldTransform = info.m_node->getWorldTransform();
	}

	if(updated && m_type == LightComponentType::kPoint)
	{
		if(!m_shadow)
		{
			m_uuid = 0;
		}
		else if(m_uuid == 0)
		{
			m_uuid = SceneGraph::getSingleton().getNewUuid();
		}

		const Bool reallyShadow = m_shadow && m_shadowAtlasUvViewportCount == 6;

		// Upload to the GPU scene
		GpuSceneLight gpuLight = {};
		gpuLight.m_position = m_worldTransform.getOrigin().xyz();
		gpuLight.m_radius = m_point.m_radius;
		gpuLight.m_diffuseColor = m_diffColor.xyz();
		gpuLight.m_squareRadiusOverOne = 1.0f / (m_point.m_radius * m_point.m_radius);
		gpuLight.m_flags = GpuSceneLightFlag::kPointLight;
		gpuLight.m_flags |= (reallyShadow) ? GpuSceneLightFlag::kShadow : GpuSceneLightFlag::kNone;
		gpuLight.m_arrayIndex = getArrayIndex();
		gpuLight.m_uuid = m_uuid;
		for(U32 f = 0; f < m_shadowAtlasUvViewportCount; ++f)
		{
			gpuLight.m_spotLightMatrixOrPointLightUvViewports[f] = m_shadowAtlasUvViewports[f];
		}

		if(!m_gpuSceneLight.isValid())
		{
			m_gpuSceneLight.allocate();
		}
		m_gpuSceneLight.uploadToGpuScene(gpuLight);
	}
	else if(updated && m_type == LightComponentType::kSpot)
	{
		if(!m_shadow)
		{
			m_uuid = 0;
		}
		else if(m_uuid == 0)
		{
			m_uuid = SceneGraph::getSingleton().getNewUuid();
		}

		const Bool reallyShadow = m_shadow && m_shadowAtlasUvViewportCount == 1;

		// Upload to the GPU scene
		GpuSceneLight gpuLight = {};
		gpuLight.m_position = m_worldTransform.getOrigin().xyz();
		gpuLight.m_radius = m_spot.m_distance;
		gpuLight.m_diffuseColor = m_diffColor.xyz();
		gpuLight.m_squareRadiusOverOne = 1.0f / (m_spot.m_distance * m_spot.m_distance);
		gpuLight.m_flags = GpuSceneLightFlag::kSpotLight;
		gpuLight.m_flags |= (reallyShadow) ? GpuSceneLightFlag::kShadow : GpuSceneLightFlag::kNone;
		gpuLight.m_arrayIndex = getArrayIndex();
		gpuLight.m_uuid = m_uuid;
		gpuLight.m_innerCos = cos(m_spot.m_innerAngle / 2.0f);
		gpuLight.m_direction = -m_worldTransform.getRotation().getZAxis();
		gpuLight.m_outerCos = cos(m_spot.m_outerAngle / 2.0f);

		Array<Vec4, 4> points;
		computeEdgesOfFrustum(m_spot.m_distance, m_spot.m_outerAngle, m_spot.m_outerAngle, &points[0]);
		for(U32 i = 0; i < 4; ++i)
		{
			m_worldTransform.transform(points[i]);
			gpuLight.m_edgePoints[i] = points[i].xyz0();
		}

		if(reallyShadow)
		{
			const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			const Mat4 proj = Mat4::calculatePerspectiveProjectionMatrix(m_spot.m_outerAngle, m_spot.m_outerAngle, kClusterObjectFrustumNearPlane,
																		 m_spot.m_distance);
			const Mat4 uvToAtlas(m_shadowAtlasUvViewports[0].z(), 0.0f, 0.0f, m_shadowAtlasUvViewports[0].x(), 0.0f, m_shadowAtlasUvViewports[0].w(),
								 0.0f, m_shadowAtlasUvViewports[0].y(), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

			m_spot.m_viewMat = Mat3x4(m_worldTransform.getInverse());
			m_spot.m_viewProjMat = proj * Mat4(m_spot.m_viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
			const Mat4 texMat = uvToAtlas * biasMat4 * m_spot.m_viewProjMat;

			gpuLight.m_spotLightMatrixOrPointLightUvViewports[0] = texMat.getRow(0);
			gpuLight.m_spotLightMatrixOrPointLightUvViewports[1] = texMat.getRow(1);
			gpuLight.m_spotLightMatrixOrPointLightUvViewports[2] = texMat.getRow(2);
			gpuLight.m_spotLightMatrixOrPointLightUvViewports[3] = texMat.getRow(3);
		}

		if(!m_gpuSceneLight.isValid())
		{
			m_gpuSceneLight.allocate();
		}
		m_gpuSceneLight.uploadToGpuScene(gpuLight);
	}
	else if(m_type == LightComponentType::kDirectional)
	{
		// Update the scene bounds always
		SceneGraph::getSingleton().getOctree().getActualSceneBounds(m_dir.m_sceneMin, m_dir.m_sceneMax);

		m_gpuSceneLight.free();
	}

	return Error::kNone;
}

void LightComponent::computeCascadeFrustums(const Frustum& primaryFrustum, ConstWeakArray<F32> cascadeDistances, WeakArray<Mat4> cascadeViewProjMats,
											WeakArray<Mat3x4> cascadeViewMats) const
{
	ANKI_ASSERT(m_type == LightComponentType::kDirectional);
	ANKI_ASSERT(m_shadow);
	ANKI_ASSERT(cascadeViewProjMats.getSize() <= kMaxShadowCascades && cascadeViewProjMats.getSize() > 0);
	ANKI_ASSERT(cascadeDistances.getSize() == cascadeViewProjMats.getSize());

	const U32 shadowCascadeCount = cascadeViewProjMats.getSize();

	// Compute the texture matrices
	if(primaryFrustum.getFrustumType() == FrustumType::kPerspective)
	{
		// Get some stuff
		const F32 fovX = primaryFrustum.getFovX();
		const F32 fovY = primaryFrustum.getFovY();

		// Compute a sphere per cascade
		Array<Sphere, kMaxShadowCascades> boundingSpheres;
		for(U32 cascade = 0; cascade < shadowCascadeCount; ++cascade)
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
			const F32 f = cascadeDistances[cascade]; // Cascade far
			const F32 n = (cascade == 0) ? primaryFrustum.getNear() : cascadeDistances[cascade - 1]; // Cascade near
			const F32 a = f * tan(fovY / 2.0f) * fovX / fovY;
			const F32 b = n * tan(fovY / 2.0f) * fovX / fovY;
			const F32 z = (b * b + n * n - a * a - f * f) / (2.0f * (f - n));
			ANKI_ASSERT(absolute((Vec2(a, -f) - Vec2(0, z)).getLength() - (Vec2(b, -n) - Vec2(0, z)).getLength()) <= kEpsilonf * 100.0f);

			Vec3 C(0.0f, 0.0f, z); // Sphere center

			// Compute the radius of the sphere
			const Vec3 A(a, tan(fovY / 2.0f) * f, -f);
			const F32 r = (A - C).getLength();

			// Set the sphere
			boundingSpheres[cascade].setRadius(r);
			boundingSpheres[cascade].setCenter(primaryFrustum.getWorldTransform().transform(C));
		}

		// Compute the matrices
		for(U32 cascade = 0; cascade < shadowCascadeCount; ++cascade)
		{
			const Sphere& sphere = boundingSpheres[cascade];
			const Vec3 sphereCenter = sphere.getCenter().xyz();
			const F32 sphereRadius = sphere.getRadius();
			const Vec3& lightDir = getDirection();
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

			// View
			Transform cascadeTransform = m_worldTransform;
			cascadeTransform.setOrigin(eye.xyz0());
			const Mat4 cascadeViewMat = Mat4(cascadeTransform.getInverse());

			// Projection
			const F32 far = (eye - sphereCenter).getLength() + sphereRadius;
			Mat4 cascadeProjMat = Mat4::calculateOrthographicProjectionMatrix(sphereRadius, -sphereRadius, sphereRadius, -sphereRadius,
																			  kClusterObjectFrustumNearPlane, far);

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

			// Write the results
			cascadeViewProjMats[cascade] = cascadeProjMat * cascadeViewMat;

			if(cascade < cascadeViewMats.getSize())
			{
				cascadeViewMats[cascade] = Mat3x4(cascadeViewMat);
			}
		}
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

void LightComponent::setShadowAtlasUvViewports(ConstWeakArray<Vec4> viewports)
{
	ANKI_ASSERT(viewports.getSize() <= 6);
	if(m_type == LightComponentType::kPoint)
	{
		ANKI_ASSERT(viewports.getSize() == 0 || viewports.getSize() == 6);
	}
	else if(m_type == LightComponentType::kSpot)
	{
		ANKI_ASSERT(viewports.getSize() == 0 || viewports.getSize() == 1);
	}
	else
	{
		ANKI_ASSERT(viewports.getSize() == 0);
	}

	const Bool dirty = m_shadowAtlasUvViewportCount != viewports.getSize()
					   || memcmp(m_shadowAtlasUvViewports.getBegin(), viewports.getBegin(), viewports.getSizeInBytes()) != 0;

	if(dirty)
	{
		m_shadowAtlasUvViewportCount = viewports.getSize();
		for(U32 i = 0; i < viewports.getSize(); ++i)
		{
			m_shadowAtlasUvViewports[i] = viewports[i];
		}

		m_dirty = true;
	}
}

} // end namespace anki
