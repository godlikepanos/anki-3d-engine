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
#include <anki/collision/Plane.h>
#include <anki/util/ThreadHive.h>
#include <anki/core/Config.h>
#include <anki/core/Trace.h>

namespace anki
{

static const U32 TYPED_OBJECT_COUNT = 4; // Point, spot, decal & probe
static const F32 INVALID_TEXTURE_INDEX = -1.0;

/// Get a view space point.
static Vec4 unproject(const F32 zVspace, const Vec2& ndc, const Vec4& unprojParams)
{
	Vec4 view;
	view.x() = ndc.x() * unprojParams.x();
	view.y() = ndc.y() * unprojParams.y();
	view.z() = 1.0f;
	view.w() = 0.0f;

	return view * zVspace;
}

template<typename TShape>
static Bool insideClusterFrustum(const Array<Plane, 4>& planeArr, const TShape& shape)
{
	for(const Plane& plane : planeArr)
	{
		if(shape.testPlane(plane) < 0.0f)
		{
			return false;
		}
	}

	return true;
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

	Atomic<U32> m_tileIdxToProcess = {0};
	Atomic<U32> m_allocatedIndexCount = {TYPED_OBJECT_COUNT};

	Vec4 m_unprojParams;

	Bool m_clusterEdgesDirty;
};

class ClusterBin::TileCtx
{
public:
	TileCtx(StackAllocator<U8>& alloc)
		: m_clusterEdgesWSpace(alloc)
		, m_clusterBoxes(alloc)
		, m_clusterSpheres(alloc)
		, m_indices(alloc)
		, m_pIndices(alloc)
		, m_pCounts(alloc)
	{
	}

	DynamicArrayAuto<Vec4> m_clusterEdgesWSpace;
	DynamicArrayAuto<Aabb> m_clusterBoxes;
	DynamicArrayAuto<Sphere> m_clusterSpheres;
	DynamicArrayAuto<U32> m_indices;
	DynamicArrayAuto<U32*> m_pIndices;
	DynamicArrayAuto<U32*> m_pCounts;
};

ClusterBin::~ClusterBin()
{
	m_clusterEdges.destroy(m_alloc);
}

void ClusterBin::init(
	HeapAllocator<U8> alloc, U32 clusterCountX, U32 clusterCountY, U32 clusterCountZ, const ConfigSet& cfg)
{
	m_alloc = alloc;

	m_clusterCounts[0] = clusterCountX;
	m_clusterCounts[1] = clusterCountY;
	m_clusterCounts[2] = clusterCountZ;

	m_totalClusterCount = clusterCountX * clusterCountY * clusterCountZ;

	m_indexCount = m_totalClusterCount * cfg.getNumber("r.avgObjectsPerCluster");

	m_clusterEdges.create(m_alloc, m_clusterCounts[0] * m_clusterCounts[1] * (m_clusterCounts[2] + 1) * 4);
}

void ClusterBin::binToClustersCallback(
	void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore)
{
	ANKI_ASSERT(userData);

	ANKI_TRACE_SCOPED_EVENT(R_BIN_TO_CLUSTERS);
	BinCtx& ctx = *static_cast<BinCtx*>(userData);

	TileCtx tileCtx(ctx.m_in->m_tempAlloc);
	const U clusterCountZ = ctx.m_bin->m_clusterCounts[2];
	const U32 avgIndicesPerCluster = ctx.m_bin->m_indexCount / ctx.m_bin->m_totalClusterCount;
	tileCtx.m_clusterEdgesWSpace.create((clusterCountZ + 1) * 4);
	tileCtx.m_clusterBoxes.create(clusterCountZ);
	tileCtx.m_clusterSpheres.create(clusterCountZ);
	tileCtx.m_indices.create(clusterCountZ * avgIndicesPerCluster);
	tileCtx.m_pIndices.create(clusterCountZ);
	tileCtx.m_pCounts.create(clusterCountZ);

	const U tileCount = ctx.m_bin->m_clusterCounts[0] * ctx.m_bin->m_clusterCounts[1];
	U tileIdx;
	while((tileIdx = ctx.m_tileIdxToProcess.fetchAdd(1)) < tileCount)
	{
		ctx.m_bin->binTile(tileIdx, ctx, tileCtx);
	}
}

void ClusterBin::writeTypedObjectsToGpuBuffersCallback(
	void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore)
{
	ANKI_ASSERT(userData);

	ANKI_TRACE_SCOPED_EVENT(R_WRITE_LIGHT_BUFFERS);
	BinCtx& ctx = *static_cast<BinCtx*>(userData);
	ctx.m_bin->writeTypedObjectsToGpuBuffers(ctx);
}

void ClusterBin::bin(ClusterBinIn& in, ClusterBinOut& out)
{
	BinCtx ctx;
	ctx.m_bin = this;
	ctx.m_in = &in;
	ctx.m_out = &out;

	prepare(ctx);

	if(ctx.m_unprojParams != m_prevUnprojParams)
	{
		ctx.m_clusterEdgesDirty = true;
		m_prevUnprojParams = ctx.m_unprojParams;
	}
	else
	{
		ctx.m_clusterEdgesDirty = false;
	}

	// Allocate indices
	U32* indices = static_cast<U32*>(ctx.m_in->m_stagingMem->allocateFrame(
		m_indexCount * sizeof(U32), StagingGpuMemoryType::STORAGE, ctx.m_out->m_indicesToken));
	ctx.m_lightIds = WeakArray<U32>(indices, m_indexCount);

	// Reserve some indices for empty clusters
	for(U i = 0; i < TYPED_OBJECT_COUNT; ++i)
	{
		indices[i] = 0;
	}

	// Allocate clusters
	U32* clusters = static_cast<U32*>(ctx.m_in->m_stagingMem->allocateFrame(
		sizeof(U32) * m_totalClusterCount, StagingGpuMemoryType::STORAGE, ctx.m_out->m_clustersToken));
	ctx.m_clusters = WeakArray<U32>(clusters, m_totalClusterCount);

	// Create task for writing GPU buffers
	Array<ThreadHiveTask, ThreadHive::MAX_THREADS + 1> tasks;
	tasks[0].m_callback = writeTypedObjectsToGpuBuffersCallback;
	tasks[0].m_argument = &ctx;

	// Create tasks for binning
	for(U threadIdx = 0; threadIdx < in.m_threadHive->getThreadCount(); ++threadIdx)
	{
		tasks[threadIdx + 1].m_callback = binToClustersCallback;
		tasks[threadIdx + 1].m_argument = &ctx;
	}

	// Submit and wait
	in.m_threadHive->submitTasks(&tasks[0], in.m_threadHive->getThreadCount() + 1);
	in.m_threadHive->waitAllTasks();
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

void ClusterBin::binTile(U32 tileIdx, BinCtx& ctx, TileCtx& tileCtx)
{
	ANKI_ASSERT(tileIdx < m_clusterCounts[0] * m_clusterCounts[1]);
	const U tileX = tileIdx % m_clusterCounts[0];
	const U tileY = tileIdx / m_clusterCounts[0];
	const U32 avgIndicesPerCluster = m_indexCount / m_totalClusterCount;

	// Compute the tile's cluster edges in view space
	WeakArray<Vec4> clusterEdgesVSpace(
		&m_clusterEdges[tileIdx * (m_clusterCounts[2] + 1) * 4], (m_clusterCounts[2] + 1) * 4);
	if(ctx.m_clusterEdgesDirty)
	{
		const Vec2 tileSize = 2.0f / Vec2(m_clusterCounts[0], m_clusterCounts[1]);
		const Vec2 startNdc = Vec2(F32(tileX) / m_clusterCounts[0], F32(tileY) / m_clusterCounts[1]) * 2.0f - 1.0f;
		const Vec4& unprojParams = ctx.m_unprojParams;

		for(U clusterZ = 0; clusterZ < m_clusterCounts[2] + 1; ++clusterZ)
		{
			const F32 zNear = -computeClusterNear(ctx.m_out->m_shaderMagicValues, clusterZ);
			const U idx = clusterZ * 4;

			clusterEdgesVSpace[idx + 0] = unproject(zNear, startNdc, unprojParams).xyz1();
			clusterEdgesVSpace[idx + 1] = unproject(zNear, startNdc + Vec2(tileSize.x(), 0.0f), unprojParams).xyz1();
			clusterEdgesVSpace[idx + 2] = unproject(zNear, startNdc + tileSize, unprojParams).xyz1();
			clusterEdgesVSpace[idx + 3] = unproject(zNear, startNdc + Vec2(0.0f, tileSize.y()), unprojParams).xyz1();
		}
	}

	// Transform the tile's cluster edges to world space
	DynamicArrayAuto<Vec4>& clusterEdgesWSpace = tileCtx.m_clusterEdgesWSpace;
	for(U clusterZ = 0; clusterZ < m_clusterCounts[2] + 1; ++clusterZ)
	{
		const U idx = clusterZ * 4;
		clusterEdgesWSpace[idx + 0] = (ctx.m_in->m_renderQueue->m_cameraTransform * clusterEdgesVSpace[idx + 0]).xyz0();
		clusterEdgesWSpace[idx + 1] = (ctx.m_in->m_renderQueue->m_cameraTransform * clusterEdgesVSpace[idx + 1]).xyz0();
		clusterEdgesWSpace[idx + 2] = (ctx.m_in->m_renderQueue->m_cameraTransform * clusterEdgesVSpace[idx + 2]).xyz0();
		clusterEdgesWSpace[idx + 3] = (ctx.m_in->m_renderQueue->m_cameraTransform * clusterEdgesVSpace[idx + 3]).xyz0();
	}

	// Compute the tile frustum
	Array<Plane, 4> frustumPlanes;
	const U lastQuartet = clusterEdgesWSpace.getSize() - 1 - 4;
	const U beforeLastQuartet = lastQuartet - 4;
	frustumPlanes[0].setFrom3Points(clusterEdgesWSpace[beforeLastQuartet + 0],
		clusterEdgesWSpace[beforeLastQuartet + 1],
		clusterEdgesWSpace[lastQuartet + 0]);
	frustumPlanes[1].setFrom3Points(clusterEdgesWSpace[beforeLastQuartet + 1],
		clusterEdgesWSpace[beforeLastQuartet + 2],
		clusterEdgesWSpace[lastQuartet + 2]);
	frustumPlanes[2].setFrom3Points(clusterEdgesWSpace[beforeLastQuartet + 2],
		clusterEdgesWSpace[beforeLastQuartet + 3],
		clusterEdgesWSpace[lastQuartet + 2]);
	frustumPlanes[3].setFrom3Points(clusterEdgesWSpace[beforeLastQuartet + 3],
		clusterEdgesWSpace[beforeLastQuartet + 0],
		clusterEdgesWSpace[lastQuartet + 0]);

	// Compute the cluster AABBs and spheres
	DynamicArrayAuto<Aabb>& clusterBoxes = tileCtx.m_clusterBoxes;
	DynamicArrayAuto<Sphere>& clusterSpheres = tileCtx.m_clusterSpheres;
	for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
	{
		// Compute an AABB and a sphere that contains the cluster
		Vec4 aabbMin(MAX_F32, MAX_F32, MAX_F32, 0.0f);
		Vec4 aabbMax(MIN_F32, MIN_F32, MIN_F32, 0.0f);
		for(U i = 0; i < 8; ++i)
		{
			aabbMin = aabbMin.min(clusterEdgesWSpace[clusterZ * 4 + i]);
			aabbMax = aabbMax.max(clusterEdgesWSpace[clusterZ * 4 + i]);
		}

		clusterBoxes[clusterZ] = Aabb(aabbMin, aabbMax);

		const Vec4 sphereCenter = (aabbMin + aabbMax) / 2.0f;
		clusterSpheres[clusterZ] = Sphere(sphereCenter, (aabbMin - sphereCenter).getLength());
	}

	// Set temp indices for each cluster
	DynamicArrayAuto<U32>& indices = tileCtx.m_indices;
	DynamicArrayAuto<U32*>& pIndices = tileCtx.m_pIndices;
	DynamicArrayAuto<U32*>& pCounts = tileCtx.m_pCounts;

	for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
	{
		pIndices[clusterZ] = &indices[clusterZ * avgIndicesPerCluster];
	}

	// Decals
	{
		for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
		{
			pCounts[clusterZ] = pIndices[clusterZ];
			*pCounts[clusterZ] = 0;
			++pIndices[clusterZ];
		}

		Obb decalBox;
		for(U i = 0; i < ctx.m_in->m_renderQueue->m_decals.getSize(); ++i)
		{
			const DecalQueueElement& decal = ctx.m_in->m_renderQueue->m_decals[i];
			decalBox.setCenter(decal.m_obbCenter.xyz0());
			decalBox.setRotation(Mat3x4(decal.m_obbRotation));
			decalBox.setExtend(decal.m_obbExtend.xyz0());

			if(!insideClusterFrustum(frustumPlanes, decalBox))
			{
				continue;
			}

			for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
			{
				if(!testCollisionShapes(decalBox, clusterBoxes[clusterZ]))
				{
					continue;
				}

				const U32 count = pIndices[clusterZ] - &indices[clusterZ * avgIndicesPerCluster];
				if(ANKI_UNLIKELY(count + 3 >= avgIndicesPerCluster))
				{
					ANKI_R_LOGW("Out of cluster indices. Increase r.avgObjectsPerCluster");
					continue;
				}

				*pIndices[clusterZ] = i;
				*pCounts[clusterZ] += 1;
				++pIndices[clusterZ];
			}
		}
	}

	// Point lights
	{
		for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
		{
			pCounts[clusterZ] = pIndices[clusterZ];
			*pCounts[clusterZ] = 0;
			++pIndices[clusterZ];
		}

		Sphere lightSphere;
		for(U i = 0; i < ctx.m_in->m_renderQueue->m_pointLights.getSize(); ++i)
		{
			const PointLightQueueElement& plight = ctx.m_in->m_renderQueue->m_pointLights[i];
			lightSphere.setCenter(plight.m_worldPosition.xyz0());
			lightSphere.setRadius(plight.m_radius);

			if(!insideClusterFrustum(frustumPlanes, lightSphere))
			{
				continue;
			}

			for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
			{
				if(!testCollisionShapes(lightSphere, clusterBoxes[clusterZ]))
				{
					continue;
				}

				const U32 count = pIndices[clusterZ] - &indices[clusterZ * avgIndicesPerCluster];
				if(ANKI_UNLIKELY(count + 2 >= avgIndicesPerCluster))
				{
					ANKI_R_LOGW("Out of cluster indices. Increase r.avgObjectsPerCluster");
					continue;
				}

				*pIndices[clusterZ] = i;
				*pCounts[clusterZ] += 1;
				++pIndices[clusterZ];
			}
		}
	}

	// Spot lights
	{
		for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
		{
			pCounts[clusterZ] = pIndices[clusterZ];
			*pCounts[clusterZ] = 0;
			++pIndices[clusterZ];
		}

		PerspectiveFrustum slightFrustum;
		for(U i = 0; i < ctx.m_in->m_renderQueue->m_spotLights.getSize(); ++i)
		{
			const SpotLightQueueElement& slight = ctx.m_in->m_renderQueue->m_spotLights[i];
			slightFrustum.setAll(slight.m_outerAngle, slight.m_outerAngle, 0.01f, slight.m_distance);
			slightFrustum.resetTransform(Transform(slight.m_worldTransform));

			if(!insideClusterFrustum(frustumPlanes, slightFrustum))
			{
				continue;
			}

			for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
			{
				if(!testConeSphere(slight.m_worldTransform.getTranslationPart().xyz0(),
					   -slight.m_worldTransform.getZAxis(),
					   slight.m_distance,
					   slight.m_outerAngle,
					   clusterSpheres[clusterZ].getCenter(),
					   clusterSpheres[clusterZ].getRadius()))
				{
					continue;
				}

				const U32 count = pIndices[clusterZ] - &indices[clusterZ * avgIndicesPerCluster];
				if(ANKI_UNLIKELY(count + 1 >= avgIndicesPerCluster))
				{
					ANKI_R_LOGW("Out of cluster indices. Increase r.avgObjectsPerCluster");
					continue;
				}

				*pIndices[clusterZ] = i;
				*pCounts[clusterZ] += 1;
				++pIndices[clusterZ];
			}
		}
	}

	// Probes
	{
		for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
		{
			pCounts[clusterZ] = pIndices[clusterZ];
			*pCounts[clusterZ] = 0;
			++pIndices[clusterZ];
		}

		Aabb probeBox;
		for(U i = 0; i < ctx.m_in->m_renderQueue->m_reflectionProbes.getSize(); ++i)
		{
			const ReflectionProbeQueueElement& probe = ctx.m_in->m_renderQueue->m_reflectionProbes[i];
			probeBox.setMin(probe.m_aabbMin);
			probeBox.setMax(probe.m_aabbMax);

			if(!insideClusterFrustum(frustumPlanes, probeBox))
			{
				continue;
			}

			for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
			{
				if(!testCollisionShapes(probeBox, clusterBoxes[clusterZ]))
				{
					continue;
				}

				const U32 count = pIndices[clusterZ] - &indices[clusterZ * avgIndicesPerCluster];
				if(ANKI_UNLIKELY(count >= avgIndicesPerCluster))
				{
					ANKI_R_LOGW("Out of cluster indices. Increase r.avgObjectsPerCluster");
					continue;
				}

				*pIndices[clusterZ] = i;
				*pCounts[clusterZ] += 1;
				++pIndices[clusterZ];
			}
		}
	}

	// Upload the indices for all clusters of the tile
	for(U clusterZ = 0; clusterZ < m_clusterCounts[2]; ++clusterZ)
	{
		const U indexCount = pIndices[clusterZ] - &indices[clusterZ * avgIndicesPerCluster];
		ANKI_ASSERT(indexCount <= avgIndicesPerCluster);
		ANKI_ASSERT(indexCount >= TYPED_OBJECT_COUNT);

		U firstIndex;
		if(indexCount > TYPED_OBJECT_COUNT)
		{
			// Have some objects to bin

			firstIndex = ctx.m_allocatedIndexCount.fetchAdd(indexCount);
			ANKI_ASSERT(firstIndex + indexCount <= ctx.m_lightIds.getSize());

			memcpy(&ctx.m_lightIds[firstIndex],
				&indices[clusterZ * avgIndicesPerCluster],
				sizeof(ctx.m_lightIds[firstIndex]) * indexCount);
		}
		else
		{
			// No typed objects, point to the preallocated cluster
			firstIndex = 0;
		}

		// Write the cluster
		const U clusterIndex =
			clusterZ * (m_clusterCounts[0] * m_clusterCounts[1]) + tileY * m_clusterCounts[0] + tileX;
		ctx.m_clusters[clusterIndex] = firstIndex;
	}
}

void ClusterBin::writeTypedObjectsToGpuBuffers(BinCtx& ctx) const
{
	const RenderQueue& rqueue = *ctx.m_in->m_renderQueue;

	// Write the point lights
	const U visiblePointLightCount = rqueue.m_pointLights.getSize();
	if(visiblePointLightCount)
	{
		PointLight* data = static_cast<PointLight*>(ctx.m_in->m_stagingMem->allocateFrame(
			sizeof(PointLight) * visiblePointLightCount, StagingGpuMemoryType::UNIFORM, ctx.m_out->m_pointLightsToken));

		WeakArray<PointLight> gpuLights(data, visiblePointLightCount);

		for(U i = 0; i < visiblePointLightCount; ++i)
		{
			const PointLightQueueElement& in = rqueue.m_pointLights[i];
			PointLight& out = gpuLights[i];

			out.m_posRadius = Vec4(in.m_worldPosition.xyz(), 1.0f / (in.m_radius * in.m_radius));
			out.m_diffuseColorTileSize = in.m_diffuseColor.xyz0();

			if(in.m_shadowRenderQueues[0] == nullptr || !ctx.m_in->m_shadowsEnabled)
			{
				out.m_diffuseColorTileSize.w() = INVALID_TEXTURE_INDEX;
			}
			else
			{
				out.m_diffuseColorTileSize.w() = in.m_atlasTileSize;
				out.m_atlasTiles = UVec2(in.m_atlasTiles.x(), in.m_atlasTiles.y());
			}

			out.m_radiusPad1 = Vec2(in.m_radius);
		}
	}
	else
	{
		ctx.m_out->m_pointLightsToken.markUnused();
	}

	// Write the spot lights
	const U visibleSpotLightCount = rqueue.m_spotLights.getSize();
	if(visibleSpotLightCount)
	{
		SpotLight* data = static_cast<SpotLight*>(ctx.m_in->m_stagingMem->allocateFrame(
			sizeof(SpotLight) * visibleSpotLightCount, StagingGpuMemoryType::UNIFORM, ctx.m_out->m_spotLightsToken));

		WeakArray<SpotLight> gpuLights(data, visibleSpotLightCount);

		for(U i = 0; i < visibleSpotLightCount; ++i)
		{
			const SpotLightQueueElement& in = rqueue.m_spotLights[i];
			SpotLight& out = gpuLights[i];

			F32 shadowmapIndex = INVALID_TEXTURE_INDEX;

			if(in.hasShadow() && ctx.m_in->m_shadowsEnabled)
			{
				// bias * proj_l * view_l
				out.m_texProjectionMat = in.m_textureMatrix;

				shadowmapIndex = 1.0f; // Just set a value
			}

			// Pos & dist
			out.m_posRadius =
				Vec4(in.m_worldTransform.getTranslationPart().xyz(), 1.0f / (in.m_distance * in.m_distance));

			// Diff color and shadowmap ID now
			out.m_diffuseColorShadowmapId = Vec4(in.m_diffuseColor, shadowmapIndex);

			// Light dir & radius
			Vec3 lightDir = -in.m_worldTransform.getRotationPart().getZAxis();
			out.m_lightDirRadius = Vec4(lightDir, in.m_distance);

			// Angles
			out.m_outerCosInnerCos = Vec4(cos(in.m_outerAngle / 2.0f), cos(in.m_innerAngle / 2.0f), 1.0f, 1.0f);
		}
	}
	else
	{
		ctx.m_out->m_spotLightsToken.markUnused();
	}

	// Write the decals
	const U visibleDecalCount = rqueue.m_decals.getSize();
	if(visibleDecalCount)
	{
		Decal* data = static_cast<Decal*>(ctx.m_in->m_stagingMem->allocateFrame(
			sizeof(Decal) * visibleDecalCount, StagingGpuMemoryType::UNIFORM, ctx.m_out->m_decalsToken));

		WeakArray<Decal> gpuDecals(data, visibleDecalCount);
		TextureView* diffuseAtlas = nullptr;
		TextureView* specularRoughnessAtlas = nullptr;

		for(U i = 0; i < visibleDecalCount; ++i)
		{
			const DecalQueueElement& in = rqueue.m_decals[i];
			Decal& out = gpuDecals[i];

			if((diffuseAtlas != nullptr && diffuseAtlas != in.m_diffuseAtlas)
				|| (specularRoughnessAtlas != nullptr && specularRoughnessAtlas != in.m_specularRoughnessAtlas))
			{
				ANKI_R_LOGF("All decals should have the same tex atlas");
			}

			diffuseAtlas = in.m_diffuseAtlas;
			specularRoughnessAtlas = in.m_specularRoughnessAtlas;

			// Diff
			Vec4 uv = in.m_diffuseAtlasUv;
			out.m_diffUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
			out.m_blendFactors[0] = in.m_diffuseAtlasBlendFactor;

			// Other
			uv = in.m_specularRoughnessAtlasUv;
			out.m_normRoughnessUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
			out.m_blendFactors[1] = in.m_specularRoughnessAtlasBlendFactor;

			// bias * proj_l * view
			out.m_texProjectionMat = in.m_textureMatrix;
		}

		ANKI_ASSERT(diffuseAtlas && specularRoughnessAtlas);
		ctx.m_out->m_diffDecalTexView.reset(diffuseAtlas);
		ctx.m_out->m_specularRoughnessDecalTexView.reset(specularRoughnessAtlas);
	}
	else
	{
		ctx.m_out->m_decalsToken.markUnused();
	}

	// Write the probes
	const U visibleProbeCount = rqueue.m_reflectionProbes.getSize();
	if(visibleProbeCount)
	{
		ReflectionProbe* data = static_cast<ReflectionProbe*>(ctx.m_in->m_stagingMem->allocateFrame(
			sizeof(ReflectionProbe) * visibleProbeCount, StagingGpuMemoryType::UNIFORM, ctx.m_out->m_probesToken));

		WeakArray<ReflectionProbe> gpuProbes(data, visibleProbeCount);

		for(U i = 0; i < visibleProbeCount; ++i)
		{
			const ReflectionProbeQueueElement& in = rqueue.m_reflectionProbes[i];
			ReflectionProbe& out = gpuProbes[i];

			out.m_positionCubemapIndex = Vec4(in.m_worldPosition, in.m_textureArrayIndex);
			out.m_aabbMinPad1 = in.m_aabbMin.xyz0();
			out.m_aabbMaxPad1 = in.m_aabbMax.xyz0();
		}
	}
	else
	{
		ctx.m_out->m_probesToken.markUnused();
	}
}

} // end namespace anki