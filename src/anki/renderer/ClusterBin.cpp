// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ClusterBin.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Sphere.h>
#include <anki/collision/Functions.h>
#include <anki/collision/Tests.h>

namespace anki
{

const U TYPED_OBJECT_COUNT = 4; // Point, spot, decal & probe

static Vec4 unproject(const F32 zVspace, const Vec2& ndc, const Vec4& projParams)
{
	Vec4 view;
	Vec2 viewxy = ndc * projParams.xy() * zVspace;
	view.x() = viewxy.x();
	view.y() = viewxy.y();
	view.z() = zVspace;
	view.w() = 0.0;

	return view;
}

static Bool testConeVsSphere(
	const Vec3& coneOrigin, const Vec3& coneDir, F32 coneLength, F32 coneAngle, const Sphere& sphere)
{
	const Vec3 V = sphere.getCenter().xyz() - coneOrigin;
	const F32 VlenSq = V.dot(V);
	const F32 V1len = V.dot(coneDir);
	const F32 distanceClosestPoint = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

	const Bool angleCull = distanceClosestPoint > sphere.getRadius();
	const Bool frontCull = V1len > sphere.getRadius() + coneLength;
	const Bool backCull = V1len < -sphere.getRadius();
	return !(angleCull || frontCull || backCull);
}

/// Bin context.
class ClusterBin::BinCtx
{
public:
	ClusterBin* m_bin ANKI_DBG_NULLIFY;
	ClusterBinIn* m_in ANKI_DBG_NULLIFY;
	ClusterBinOut* m_out ANKI_DBG_NULLIFY;

	WeakArray<PointLight> m_pointLights;
	WeakArray<SpotLight> m_spotLights;
	WeakArray<ReflectionProbe> m_probes;
	WeakArray<Decal> m_decals;

	WeakArray<U32> m_lightIds;
	WeakArray<U32> m_clusters;

	Atomic<U32> m_clusterIdxToProcess = {0};
	Atomic<U32> m_allocatedIndexCount = {TYPED_OBJECT_COUNT};

	Vec4 m_unprojParams;
};

ClusterBin::ClusterBin(
	const GenericMemoryPoolAllocator<U8>& alloc, U32 clusterCountX, U32 clusterCountY, U32 clusterCountZ)
	: m_alloc(alloc)
	, m_clusterCounts{{clusterCountX, clusterCountY, clusterCountZ}}
{
	m_totalClusterCount = clusterCountX * clusterCountY * clusterCountZ;
}

ClusterBin::~ClusterBin()
{
}

void ClusterBin::processNextClusterCallback(
	void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore)
{
	ANKI_ASSERT(userData);
	BinCtx& ctx = *static_cast<BinCtx*>(userData);
	ctx.m_bin->processNextCluster(ctx);
}

void ClusterBin::bin(ClusterBinIn& in, ClusterBinOut& out)
{
	BinCtx ctx;
	ctx.m_bin = this;
	ctx.m_in = &in;
	ctx.m_out = &out;

	prepare(ctx);

	// Allocate indices
	U32* indices = static_cast<U32*>(ctx.m_in->m_stagingMem->allocateFrame(
		ctx.m_in->m_maxLightIndices * sizeof(U32), StagingGpuMemoryType::STORAGE, ctx.m_out->m_indicesToken));
	ctx.m_lightIds = WeakArray<U32>(indices, ctx.m_in->m_maxLightIndices);

	// Allocate clusters
	U32* clusters = static_cast<U32*>(ctx.m_in->m_stagingMem->allocateFrame(
		sizeof(U32) * m_totalClusterCount, StagingGpuMemoryType::STORAGE, ctx.m_out->m_clustersToken));
	ctx.m_clusters = WeakArray<U32>(clusters, m_totalClusterCount);
}

void ClusterBin::prepare(BinCtx& ctx)
{
	const F32 near = ctx.m_in->m_renderQueue->m_cameraNear;
	const F32 far = ctx.m_in->m_renderQueue->m_cameraFar;

	const F32 calcNearOpt = (far - near) / (m_clusterCounts[2] * m_clusterCounts[2]);

	// Compute magic val 0
	// It's been used to calculate the 'k' of a cluster given the world position
	{
		// Given a distance 'd' from the camera's near plane in world space the 'k' split is calculated like:
		// k = sqrt(d / (f - n) * Cz2)  (1)
		// where 'n' and 'f' are the near and far vals of the projection and Cz2 is the m_counts[2]^2
		// If the 'd' is not known and the world position instead is known then 'd' is the distance from that position
		// to the camera's near plane.
		// d = dot(Pn, W) - Po  (2)
		// where 'Pn' is the plane's normal, 'Po' is the plane's offset and 'W' is the world position.
		// Substituting d from (2) in (1) we have:
		// k = sqrt((dot(Pn, W) - Po) / (f - n) * Cz2) =>
		// k = sqrt((dot(Pn, W) * Cz2 - Po * Cz2) / (f - n))
		// k = sqrt(dot(Pn, W) * Cz2 / (f - n) - Po * Cz2 / (f - n))
		// k = sqrt(dot(Pn * Cz2 / (f - n), W) - Po * Cz2 / (f - n))
		// If we assume that:
		// A = Pn * Cz2 / (f - n) and B =  Po * Cz2 / (f - n)
		// Then:
		// k = sqrt(dot(A, W) - B)

		const Mat4& vp = ctx.m_in->m_renderQueue->m_viewProjectionMatrix;
		Plane nearPlane;
		Array<Plane*, U(FrustumPlaneType::COUNT)> planes = {};
		planes[FrustumPlaneType::NEAR] = &nearPlane;
		extractClipPlanes(vp, planes);

		Vec3 A = nearPlane.getNormal().xyz() * (m_clusterCounts[2] * m_clusterCounts[2]) / (far - near);
		F32 B = nearPlane.getOffset() * (m_clusterCounts[2] * m_clusterCounts[2]) / (far - near);

		ctx.m_out->m_shaderMagicValues.m_val0 = Vec4(A, B);
	}

	// Compute magic val 1
	{
		ctx.m_out->m_shaderMagicValues.m_val1.x() = calcNearOpt;
		ctx.m_out->m_shaderMagicValues.m_val1.y() = near;
	}

	// Unproj params
	ctx.m_unprojParams = ctx.m_in->m_renderQueue->m_projectionMatrix.extractPerspectiveUnprojectionParams();
}

void ClusterBin::processNextCluster(BinCtx& ctx) const
{
	const U clusterIdx = ctx.m_clusterIdxToProcess.fetchAdd(1);
	if(clusterIdx >= m_totalClusterCount)
	{
		// Done
		return;
	}

	// Get the cluster indices
	U clusterX, clusterY, clusterZ;
	unflatten3dArrayIndex(
		m_clusterCounts[2], m_clusterCounts[1], m_clusterCounts[0], clusterIdx, clusterZ, clusterY, clusterX);

	// Compute the cluster edges in vspace
	Array<Vec4, 8> clusterEdgesVSpace;

	const F32 zNear = -computeClusterNear(ctx.m_out->m_shaderMagicValues, clusterZ);
	const F32 zFar = -computeClusterFar(ctx.m_out->m_shaderMagicValues, clusterZ);
	ANKI_ASSERT(zNear > zFar);

	const Vec2 tileSize = 2.0f / Vec2(m_clusterCounts[0], m_clusterCounts[1]);
	const Vec2 startNdc = Vec2(F32(clusterX) / m_clusterCounts[0], F32(clusterY) / m_clusterCounts[1]) * 2.0f - 1.0f;

	const Vec4& unprojParams = ctx.m_unprojParams;
	clusterEdgesVSpace[0] = unproject(zNear, startNdc, unprojParams);
	clusterEdgesVSpace[1] = unproject(zNear, startNdc + Vec2(tileSize.x(), 0.0f), unprojParams);
	clusterEdgesVSpace[2] = unproject(zNear, startNdc + Vec2(0.0f, tileSize.y()), unprojParams);
	clusterEdgesVSpace[3] = unproject(zNear, startNdc + tileSize, unprojParams);
	clusterEdgesVSpace[4] = unproject(zFar, startNdc, unprojParams);
	clusterEdgesVSpace[5] = unproject(zFar, startNdc + Vec2(tileSize.x(), 0.0f), unprojParams);
	clusterEdgesVSpace[6] = unproject(zFar, startNdc + Vec2(0.0f, tileSize.y()), unprojParams);
	clusterEdgesVSpace[7] = unproject(zFar, startNdc + tileSize, unprojParams);

	// Move the cluster edges to wspace
	Array<Vec4, 8> clusterEdgesWSpace;
	for(U i = 0; i < 8; ++i)
	{
		clusterEdgesWSpace[i] = ctx.m_in->m_renderQueue->m_cameraTransform * clusterEdgesVSpace[i];
	}

	// Compute an AABB that contains the cluster
	Vec3 aabbMin(MAX_F32);
	Vec3 aabbMax(MIN_F32);
	for(U i = 0; i < 8; ++i)
	{
		aabbMin = aabbMin.min(clusterEdgesWSpace[i].xyz());
		aabbMax = aabbMax.max(clusterEdgesWSpace[i].xyz());
	}

	const Aabb clusterBox(aabbMin, aabbMax);

	const Vec3 sphereCenter = (aabbMin + aabbMax) / 2.0f;
	const Sphere clusterSphere(sphereCenter.xyz0(), (aabbMin - sphereCenter).getLength());

	// Bin decals
	Array<U32, 32> objectIndices;
	U32* pObjectIndex = &objectIndices[0];
	const U32* pObjectIndexEnd = &objectIndices[0] + objectIndices.getSize();
	(void)pObjectIndexEnd;

	U idx = 0;
	Obb decalBox;
	U32* pObjectCount = pObjectIndex;
	++pObjectIndex;
	for(const DecalQueueElement& decal : ctx.m_in->m_renderQueue->m_decals)
	{
		decalBox.setCenter(decal.m_obbCenter.xyz0());
		decalBox.setRotation(Mat3x4(decal.m_obbRotation));
		decalBox.setExtend(decal.m_obbExtend.xyz0());
		if(testCollisionShapes(decalBox, clusterBox))
		{
			ANKI_ASSERT(pObjectIndex < pObjectIndexEnd);
			*pObjectIndex = idx;
			++pObjectIndex;
		}

		++idx;
	}

	*pObjectCount = pObjectIndex - pObjectCount - 1;

	// Bin the point lights
	idx = 0;
	Sphere lightSphere;
	pObjectCount = pObjectIndex;
	++pObjectIndex;
	for(const PointLightQueueElement& plight : ctx.m_in->m_renderQueue->m_pointLights)
	{
		lightSphere.setCenter(plight.m_worldPosition.xyz0());
		lightSphere.setRadius(plight.m_radius);
		if(testCollisionShapes(lightSphere, clusterBox))
		{
			ANKI_ASSERT(pObjectIndex < pObjectIndexEnd);
			*pObjectIndex = idx;
			++pObjectIndex;
		}

		++idx;
	}

	*pObjectCount = pObjectIndex - pObjectCount - 1;

	// Bin the spot lights
	idx = 0;
	pObjectCount = pObjectIndex;
	++pObjectIndex;
	for(const SpotLightQueueElement& slight : ctx.m_in->m_renderQueue->m_spotLights)
	{
		if(testConeVsSphere(slight.m_worldTransform.getTranslationPart().xyz(),
			   slight.m_worldTransform.getZAxis().xyz(),
			   slight.m_distance,
			   slight.m_outerAngle,
			   clusterSphere))
		{
			ANKI_ASSERT(pObjectIndex < pObjectIndexEnd);
			*pObjectIndex = idx;
			++pObjectIndex;
		}

		++idx;
	}

	*pObjectCount = pObjectIndex - pObjectCount - 1;

	// Bin probes
	idx = 0;
	Aabb probeBox;
	pObjectCount = pObjectIndex;
	++pObjectIndex;
	for(const ReflectionProbeQueueElement& probe : ctx.m_in->m_renderQueue->m_reflectionProbes)
	{
		probeBox.setMin(probe.m_aabbMin);
		probeBox.setMax(probe.m_aabbMax);
		if(testCollisionShapes(probeBox, clusterBox))
		{
			ANKI_ASSERT(pObjectIndex < pObjectIndexEnd);
			*pObjectIndex = idx;
			++pObjectIndex;
		}

		++idx;
	}

	*pObjectCount = pObjectIndex - pObjectCount - 1;

	// Allocate and store indices for the cluster
	U indexCount = pObjectCount - &objectIndices[0];
	ANKI_ASSERT(indexCount >= TYPED_OBJECT_COUNT);

	U firstIndex;
	if(indexCount > TYPED_OBJECT_COUNT)
	{
		firstIndex = ctx.m_allocatedIndexCount.fetchAdd(indexCount);

		if(firstIndex + indexCount <= ctx.m_lightIds.getSize())
		{
			memcpy(&ctx.m_lightIds[firstIndex], &objectIndices[0], sizeof(objectIndices[0]) * indexCount);
		}
		else
		{
			ANKI_R_LOGW("XXX");
			firstIndex = 0;
			indexCount = TYPED_OBJECT_COUNT;
		}
	}
	else
	{
		// No typed objects, point to the preallocated cluster
		firstIndex = 0;
	}

	// Write the cluster
	ctx.m_clusters[clusterIdx] = firstIndex;
}

void ClusterBin::writeTypedObjectsToGpuBuffers(BinCtx& ctx) const
{
	const RenderQueue& rqueue = *ctx.m_in->m_renderQueue;

	// Write point lights
	const U visiblePointLightsCount = rqueue.m_pointLights.getSize();
	if(visiblePointLightsCount)
	{
		PointLight* data =
			static_cast<PointLight*>(ctx.m_in->m_stagingMem->allocateFrame(sizeof(PointLight) * visiblePointLightsCount,
				StagingGpuMemoryType::UNIFORM,
				ctx.m_out->m_pointLightsToken));

		WeakArray<PointLight> gpuLights(data, visiblePointLightsCount);

		// TODO
	}
	else
	{
		ctx.m_out->m_pointLightsToken.markUnused();
	}
}

} // end namespace anki