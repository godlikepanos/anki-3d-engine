// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Octree.h>
#include <AnKi/Collision.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(LightComponent)

LightComponent::LightComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_type(LightComponentType::kPoint)
	, m_shadow(false)
	, m_markedForUpdate(true)
{
	ANKI_ASSERT(m_uuid > 0);
	m_point.m_radius = 1.0f;

	if(node->getSceneGraph().getResourceManager().loadResource("EngineAssets/LightBulb.ankitex", m_pointDebugImage)
	   || node->getSceneGraph().getResourceManager().loadResource("EngineAssets/SpotLight.ankitex", m_spotDebugImage))
	{
		ANKI_SCENE_LOGF("Failed to load resources");
	}
}

Error LightComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_markedForUpdate;
	m_markedForUpdate = false;

	if(updated && m_type == LightComponentType::kSpot)
	{

		const Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
		const Mat4 proj = Mat4::calculatePerspectiveProjectionMatrix(m_spot.m_outerAngle, m_spot.m_outerAngle,
																	 kClusterObjectFrustumNearPlane, m_spot.m_distance);
		m_spot.m_textureMat = biasMat4 * proj * Mat4(m_worldtransform.getInverse());

		Array<Vec4, 4> points;
		computeEdgesOfFrustum(m_spot.m_distance, m_spot.m_outerAngle, m_spot.m_outerAngle, &points[0]);
		for(U32 i = 0; i < 4; ++i)
		{
			m_spot.m_edgePointsWspace[i] = m_worldtransform.transform(points[i].xyz());
		}
	}

	// Update the scene bounds always
	if(m_type == LightComponentType::kDirectional)
	{
		info.m_node->getSceneGraph().getOctree().getActualSceneBounds(m_dir.m_sceneMin, m_dir.m_sceneMax);
	}

	return Error::kNone;
}

void LightComponent::setupDirectionalLightQueueElement(const FrustumComponent& frustumComp,
													   DirectionalLightQueueElement& el,
													   WeakArray<FrustumComponent> cascadeFrustumComponents) const
{
	ANKI_ASSERT(m_type == LightComponentType::kDirectional);
	ANKI_ASSERT(cascadeFrustumComponents.getSize() <= kMaxShadowCascades);

	const U32 shadowCascadeCount = cascadeFrustumComponents.getSize();

	el.m_drawCallback = [](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
		ANKI_ASSERT(userData.getSize() == 1);
		static_cast<const LightComponent*>(userData[0])->draw(ctx);
	};
	el.m_drawCallbackUserData = this;
	el.m_uuid = m_uuid;
	el.m_diffuseColor = m_diffColor.xyz();
	el.m_direction = -m_worldtransform.getRotation().getZAxis().xyz();
	el.m_effectiveShadowDistance = frustumComp.getEffectiveShadowDistance();
	el.m_shadowCascadesDistancePower = frustumComp.getShadowCascadesDistancePower();
	el.m_shadowCascadeCount = U8(shadowCascadeCount);
	el.m_shadowLayer = kMaxU8;

	if(shadowCascadeCount == 0)
	{
		return;
	}

	// Compute the texture matrices
	const Mat4 lightTrf(m_worldtransform);
	if(frustumComp.getFrustumType() == FrustumType::kPerspective)
	{
		// Get some stuff
		const F32 fovX = frustumComp.getFovX();
		const F32 fovY = frustumComp.getFovY();

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
			const F32 f = frustumComp.computeShadowCascadeDistance(i); // Cascade far
			const F32 n =
				(i == 0) ? frustumComp.getNear() : frustumComp.computeShadowCascadeDistance(i - 1); // Cascade near
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
			boundingSpheres[i].setCenter(frustumComp.getWorldTransform().transform(C));
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
			Transform cascadeTransform = m_worldtransform;
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
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kLeft, plane);
			const F32 left = plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kRight, plane);
			const F32 right = -plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kTop, plane);
			const F32 top = -plane.getOffset();
			extractClipPlane(cascadeProjMat, FrustumPlaneType::kBottom, plane);
			const F32 bottom = plane.getOffset();

			FrustumComponent& cascadeFrustumComp = cascadeFrustumComponents[i];
			cascadeFrustumComp.setOrthographic(kClusterObjectFrustumNearPlane, far, right, left, top, bottom);
			cascadeFrustumComp.setWorldTransform(cascadeTransform);
		}
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

void LightComponent::draw(RenderQueueDrawContext& ctx) const
{
	const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDepthTestOn);
	if(enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kLess);
	}
	else
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kAlways);
	}

	Vec3 color = m_diffColor.xyz();
	color /= max(max(color.x(), color.y()), color.z());

	ImageResourcePtr imageResource = (m_type == LightComponentType::kPoint) ? m_pointDebugImage : m_spotDebugImage;
	m_node->getSceneGraph().getDebugDrawer().drawBillboardTexture(
		ctx.m_projectionMatrix, ctx.m_viewMatrix, m_worldtransform.getOrigin().xyz(), color.xyz1(),
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), imageResource->getTextureView(),
		ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

	// Restore state
	if(!enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kLess);
	}
}

} // end namespace anki
