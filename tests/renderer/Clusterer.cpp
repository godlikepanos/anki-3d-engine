// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/renderer/Clusterer.h>
#include <anki/Collision.h>
#include <anki/util/ThreadPool.h>
#include "anki/util/HighRezTimer.h"

namespace anki
{

ANKI_TEST(Renderer, Clusterer)
{
	const U CLUSTER_COUNT_X = 32;
	const U CLUSTER_COUNT_Y = 24;
	const U CLUSTER_COUNT_Z = 32;
	const U ITERATION_COUNT = 32;
	const U SPHERE_COUNT = 1024;
	const F32 SPHERE_MAX_RADIUS = 1000.0;
	const F32 E = 0.01;
	const U FRUSTUM_COUNT = 1024;
	const F32 FRUSTUM_MAX_ANGLE = toRad(70.0);
	const F32 FRUSTUM_MAX_DIST = 200.0;

	HeapAllocator<U8> alloc(allocAligned, nullptr);

	Clusterer c;
	c.init(alloc, CLUSTER_COUNT_X, CLUSTER_COUNT_Y, CLUSTER_COUNT_Z);

	PerspectiveFrustum fr(toRad(70.0), toRad(60.0), 0.1, 1000.0);
	Mat4 projMat = fr.calculateProjectionMatrix();
	Vec4 unprojParams = projMat.extractPerspectiveUnprojectionParams();

	ThreadPool threadpool(4);

	// Gen spheres
	DynamicArrayAuto<Sphere> spheres(alloc);
	spheres.create(SPHERE_COUNT);
	DynamicArrayAuto<Aabb> sphereBoxes(alloc);
	sphereBoxes.create(SPHERE_COUNT);
	for(U i = 0; i < SPHERE_COUNT; ++i)
	{
		Vec2 ndc;
		ndc.x() = clamp((i % 64) / 64.0f, E, 1.0f - E) * 2.0f - 1.0f;
		ndc.y() = ndc.x();
		F32 depth = clamp((i % 128) / 128.0f, E, 1.0f - E);

		F32 z = unprojParams.z() / (unprojParams.w() + depth);
		Vec2 xy = ndc.xy() * unprojParams.xy() * z;
		Vec4 sphereC(xy, z, 0.0);

		F32 radius = max((i % 64) / 64.0f, 0.1f) * SPHERE_MAX_RADIUS;

		spheres[i] = Sphere(sphereC, radius);
		spheres[i].computeAabb(sphereBoxes[i]);
	}

	// Bin spheres
	HighRezTimer timer;
	timer.start();
	U clusterBinCount = 0;
	for(U i = 0; i < ITERATION_COUNT; ++i)
	{
		Transform camTrf(Vec4(0.1, 0.1, 0.1, 0.0), Mat3x4::getIdentity(), 1.0);

		ClustererPrepareInfo pinf;
		pinf.m_viewMat = Mat4(camTrf).getInverse();
		pinf.m_projMat = projMat;
		pinf.m_camTrf = camTrf;

		c.prepare(threadpool, pinf);
		ClustererTestResult rez;
		c.initTestResults(alloc, rez);

		for(U s = 0; s < SPHERE_COUNT; ++s)
		{
			c.bin(spheres[s], sphereBoxes[s], rez);
			ANKI_TEST_EXPECT_GT(rez.getClusterCount(), 0);
			clusterBinCount += rez.getClusterCount();
		}
	}
	timer.stop();
	F64 ms = timer.getElapsedTime() * 1000.0;
	printf("Cluster count: %u.\n"
		   "Binned %f spheres/ms.\n"
		   "Avg clusters per sphere %f\n",
		unsigned(c.getClusterCount()),
		F64(SPHERE_COUNT) * F64(ITERATION_COUNT) / ms,
		clusterBinCount / F32(ITERATION_COUNT * SPHERE_COUNT));

	// Gen spheres
	DynamicArrayAuto<PerspectiveFrustum> frs(alloc);
	frs.create(FRUSTUM_COUNT);
	DynamicArrayAuto<Aabb> frBoxes(alloc);
	frBoxes.create(FRUSTUM_COUNT);
	for(U i = 0; i < FRUSTUM_COUNT; ++i)
	{
		Vec2 ndc;
		ndc.x() = clamp((i % 64) / 64.0f, E, 1.0f - E) * 2.0f - 1.0f;
		ndc.y() = ndc.x();
		F32 depth = clamp((i % 128) / 128.0f, E, 1.0f - E);

		F32 z = unprojParams.z() / (unprojParams.w() + depth);
		Vec2 xy = ndc.xy() * unprojParams.xy() * z;
		Vec4 c(xy, z, 0.0);

		F32 dist = max((i % 64) / 64.0f, 0.1f) * FRUSTUM_MAX_DIST;
		F32 ang = max((i % 64) / 64.0f, 0.2f) * FRUSTUM_MAX_ANGLE;

		frs[i] = PerspectiveFrustum(ang, ang, 0.1, dist);
		frs[i].transform(Transform(c, Mat3x4::getIdentity(), 1.0));

		frs[i].computeAabb(frBoxes[i]);
	}

	// Bin frustums
	timer.start();
	clusterBinCount = 0;
	for(U i = 0; i < ITERATION_COUNT; ++i)
	{
		Transform camTrf(Vec4(0.1, 0.1, 0.1, 0.0), Mat3x4::getIdentity(), 1.0);

		ClustererPrepareInfo pinf;
		pinf.m_viewMat = Mat4(camTrf).getInverse();
		pinf.m_projMat = projMat;
		pinf.m_camTrf = camTrf;

		c.prepare(threadpool, pinf);
		ClustererTestResult rez;
		c.initTestResults(alloc, rez);

		for(U s = 0; s < FRUSTUM_COUNT; ++s)
		{
			c.binPerspectiveFrustum(frs[s], frBoxes[s], rez);
			ANKI_TEST_EXPECT_GT(rez.getClusterCount(), 0);
			clusterBinCount += rez.getClusterCount();
		}
	}
	timer.stop();
	ms = timer.getElapsedTime() * 1000.0;
	printf("Binned %f frustums/ms.\n"
		   "Avg clusters per frustum %f\n",
		F64(FRUSTUM_COUNT) * F64(ITERATION_COUNT) / ms,
		clusterBinCount / F32(ITERATION_COUNT * FRUSTUM_COUNT));
}

} // end namespace anki
