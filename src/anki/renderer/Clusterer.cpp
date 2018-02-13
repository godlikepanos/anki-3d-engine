// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Clusterer.h>
#include <anki/util/ThreadPool.h>
#include <anki/Collision.h>

namespace anki
{

class UpdatePlanesPerspectiveCameraTask : public ThreadPoolTask
{
public:
	Clusterer* m_clusterer = nullptr;
	Bool m_frustumChanged;

	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		m_clusterer->update(threadId, threadsCount, m_frustumChanged);
		return Error::NONE;
	}
};

static Vec4 unproject(const F32 depth, const Vec2& ndc, const Vec4& projParams)
{
	Vec4 view;
	F32 z = projParams.z() / (projParams.w() + depth);
	Vec2 viewxy = ndc * projParams.xy() * z;
	view.x() = viewxy.x();
	view.y() = viewxy.y();
	view.z() = z;
	view.w() = 0.0;

	return view;
}

static Vec4 unprojectZViewSpace(const F32 zVSpace, const Vec2& ndc, const Vec4& projParams)
{
	Vec4 view;
	view.x() = ndc.x() * projParams.x();
	view.y() = ndc.y() * projParams.y();
	view.z() = 1.0;
	view.w() = 0.0;

	return view * zVSpace;
}

Clusterer::~Clusterer()
{
	m_allPlanes.destroy(m_alloc);
	m_clusterBoxes.destroy(m_alloc);
}

void Clusterer::initTestResults(const GenericMemoryPoolAllocator<U8>& alloc, ClustererTestResult& rez) const
{
	rez.m_clusterIds.create(alloc, getClusterCount());
	rez.m_count = 0;
	rez.m_alloc = alloc;
}

F32 Clusterer::calcNear(U k) const
{
	F32 neark = m_calcNearOpt * pow(k, 2.0) + m_near;
	return neark;
}

U Clusterer::calcZ(F32 zVspace) const
{
	zVspace = clamp(zVspace, -m_far, -m_near);
	zVspace = -zVspace;
	F32 fk = sqrt((zVspace - m_near) / m_calcNearOpt);
	ANKI_ASSERT(fk >= 0.0);
	U k = min<U>(static_cast<U>(fk), m_counts[2]);

	return k;
}

void Clusterer::calcPlaneY(U i, const Vec4& projParams)
{
	Plane& plane = m_planesY[i];
	F32 y = F32(i + 1) / m_counts[1] * 2.0 - 1.0;

	Vec4 viewA = unproject(1.0, Vec2(-1.0, y), projParams);
	Vec4 viewB = unproject(1.0, Vec2(1.0, y), projParams);

	Vec4 n = viewB.cross(viewA);
	n.normalize();

	plane = Plane(n, 0.0);
}

void Clusterer::calcPlaneX(U j, const Vec4& projParams)
{
	Plane& plane = m_planesX[j];
	F32 x = F32(j + 1) / m_counts[0] * 2.0 - 1.0;

	Vec4 viewA = unproject(1.0, Vec2(x, -1.0), projParams);
	Vec4 viewB = unproject(1.0, Vec2(x, 1.0), projParams);

	Vec4 n = viewA.cross(viewB);
	n.normalize();

	plane = Plane(n, 0.0);
}

void Clusterer::setClusterBoxes(const Vec4& projParams, U begin, U end)
{
	ANKI_ASSERT((m_counts[0] % 2) == 0);
	ANKI_ASSERT((m_counts[1] % 2) == 0);
	const U c = m_counts[0] * m_counts[1];

	for(U i = begin; i < end; ++i)
	{
		U z = i / c;
		U y = (i % c) / m_counts[0];
		U x = (i % c) % m_counts[0];

		F32 zMax = -calcNear(z);
		F32 zMin = -calcNear(z + 1);

		F32 xMin, xMax;
		if(x < m_counts[0] / 2)
		{
			xMin = (F32(x) / m_counts[0] * 2.0 - 1.0) * projParams.x() * zMin;
			xMax = (F32(x + 1) / m_counts[0] * 2.0 - 1.0) * projParams.x() * zMax;
		}
		else
		{
			xMin = (F32(x) / m_counts[0] * 2.0 - 1.0) * projParams.x() * zMax;
			xMax = (F32(x + 1) / m_counts[0] * 2.0 - 1.0) * projParams.x() * zMin;
		}

		F32 yMin, yMax;
		if(y < m_counts[1] / 2)
		{
			yMin = (F32(y) / m_counts[1] * 2.0 - 1.0) * projParams.y() * zMin;
			yMax = (F32(y + 1) / m_counts[1] * 2.0 - 1.0) * projParams.y() * zMax;
		}
		else
		{
			yMin = (F32(y) / m_counts[1] * 2.0 - 1.0) * projParams.y() * zMax;
			yMax = (F32(y + 1) / m_counts[1] * 2.0 - 1.0) * projParams.y() * zMin;
		}

		m_clusterBoxes[i] = Aabb(Vec4(xMin, yMin, zMin, 0.0), Vec4(xMax, yMax, zMax, 0.0));
	}
}

void Clusterer::init(const GenericMemoryPoolAllocator<U8>& alloc, U clusterCountX, U clusterCountY, U clusterCountZ)
{
	m_alloc = alloc;
	m_counts[0] = clusterCountX;
	m_counts[1] = clusterCountY;
	m_counts[2] = clusterCountZ;

	// Init planes. One plane for each direction, plus near/far plus the world space of those
	U planesCount = (m_counts[0] - 1) * 2 // planes J
					+ (m_counts[1] - 1) * 2 // planes I
					+ 2; // Near and far planes

	m_allPlanes.create(m_alloc, planesCount);

	Plane* base = &m_allPlanes[0];
	U count = 0;

	m_planesX = WeakArray<Plane>(base + count, m_counts[0] - 1);
	count += m_planesX.getSize();

	m_planesY = WeakArray<Plane>(base + count, m_counts[1] - 1);
	count += m_planesY.getSize();

	m_planesXW = WeakArray<Plane>(base + count, m_counts[0] - 1);
	count += m_planesXW.getSize();

	m_planesYW = WeakArray<Plane>(base + count, m_counts[1] - 1);
	count += m_planesYW.getSize();

	m_nearPlane = base + count;
	++count;

	m_farPlane = base + count;
	++count;

	ANKI_ASSERT(count == m_allPlanes.getSize());

	m_clusterBoxes.create(m_alloc, m_counts[0] * m_counts[1] * m_counts[2]);
}

void Clusterer::prepare(ThreadPool& threadPool, const ClustererPrepareInfo& inf)
{
	Bool frustumChanged = m_projMat != inf.m_projMat;

	// Compute cached values
	m_projMat = inf.m_projMat;
	m_viewMat = inf.m_viewMat;
	m_camTrf = inf.m_camTrf;
	m_near = inf.m_near;
	m_far = inf.m_far;
	ANKI_ASSERT(m_near < m_far && m_near > 0.0);

	m_unprojParams = m_projMat.extractPerspectiveUnprojectionParams();
	m_calcNearOpt = (m_far - m_near) / pow(m_counts[2], 2.0);

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

		const Mat4& vp = inf.m_viewProjMat;
		Plane nearPlane;
		Array<Plane*, U(FrustumPlaneType::COUNT)> planes = {};
		planes[FrustumPlaneType::NEAR] = &nearPlane;
		extractClipPlanes(vp, planes);

		Vec3 A = nearPlane.getNormal().xyz() * (m_counts[2] * m_counts[2]) / (m_far - m_near);
		F32 B = nearPlane.getOffset() * (m_counts[2] * m_counts[2]) / (m_far - m_near);

		m_shaderMagicVals.m_val0 = Vec4(A, B);
	}

	// Compute magic val 1
	{
		m_shaderMagicVals.m_val1.x() = m_calcNearOpt;
		m_shaderMagicVals.m_val1.y() = m_near;
	}

	//
	// Issue parallel jobs
	//
	UpdatePlanesPerspectiveCameraTask job;
	job.m_clusterer = this;
	job.m_frustumChanged = frustumChanged;

	for(U i = 0; i < threadPool.getThreadCount(); i++)
	{
		threadPool.assignNewTask(i, &job);
	}

	// Sync threads
	Error err = threadPool.waitForAllThreadsToFinish();
	(void)err;
}

void Clusterer::computeSplitRange(const CollisionShape& cs, U& zBegin, U& zEnd) const
{
	// Find the distance between cs and near plane
	F32 dist = cs.testPlane(*m_nearPlane);
	dist = max(m_near, dist);

	// Find split
	zBegin = calcZ(-dist);
	ANKI_ASSERT(zBegin <= m_counts[2]);

	// Find the distance between cs and far plane
	dist = cs.testPlane(*m_farPlane);
	dist = max(0.0f, dist);
	dist = m_far - dist;

	// Find split
	zEnd = min<U>(calcZ(-dist) + 1, m_counts[2]);
	ANKI_ASSERT(zEnd <= m_counts[2]);
}

void Clusterer::bin(const CollisionShape& cs, const Aabb& csBox, ClustererTestResult& rez) const
{
	rez.m_count = 0;

	if(cs.getType() == CollisionShapeType::SPHERE)
	{
		binSphere(static_cast<const Sphere&>(cs), csBox, rez);
	}
	else
	{
		binGeneric(cs, csBox, rez);
	}
}

void Clusterer::totallyInsideAllTiles(U zBegin, U zEnd, ClustererTestResult& rez) const
{
	for(U z = zBegin; z < zEnd; ++z)
	{
		for(U y = 0; y < m_counts[1]; ++y)
		{
			for(U x = 0; x < m_counts[0]; ++x)
			{
				rez.pushBack(x, y, z);
			}
		}
	}
}

void Clusterer::quickReduction(const Aabb& aabb, const Mat4& mvp, U& xBegin, U& xEnd, U& yBegin, U& yEnd) const
{
	// Compute projection points
	const Vec4& minv = aabb.getMin();
	const Vec4& maxv = aabb.getMax();
	Array<Vec4, 8> points;
	points[0] = minv.xyz1();
	points[1] = Vec4(minv.x(), maxv.y(), minv.z(), 1.0);
	points[2] = Vec4(minv.x(), maxv.y(), maxv.z(), 1.0);
	points[3] = Vec4(minv.x(), minv.y(), maxv.z(), 1.0);
	points[4] = maxv.xyz1();
	points[5] = Vec4(maxv.x(), minv.y(), maxv.z(), 1.0);
	points[6] = Vec4(maxv.x(), minv.y(), minv.z(), 1.0);
	points[7] = Vec4(maxv.x(), maxv.y(), minv.z(), 1.0);
	Vec2 min2(MAX_F32), max2(MIN_F32);
	for(Vec4& p : points)
	{
		p = mvp * p;
		ANKI_ASSERT(p.w() > 0.0 && "Should have cliped tha aabb before calling this");
		p = p.perspectiveDivide();

		for(U i = 0; i < 2; ++i)
		{
			min2[i] = min(min2[i], p[i]);
			max2[i] = max(max2[i], p[i]);
		}
	}

	min2 = min2 * 0.5 + 0.5;
	max2 = max2 * 0.5 + 0.5;

	// Compute ranges
	xBegin = clamp<F32>(floor(m_counts[0] * min2.x()), 0.0, m_counts[0]);
	xEnd = clamp<F32>(ceil(m_counts[0] * max2.x()), 0.0, m_counts[0]);
	yBegin = clamp<F32>(floor(m_counts[1] * min2.y()), 0.0, m_counts[1]);
	yEnd = clamp<F32>(ceil(m_counts[1] * max2.y()), 0.0, m_counts[1]);

	ANKI_ASSERT(xBegin <= m_counts[0] && xEnd <= m_counts[0]);
	ANKI_ASSERT(yBegin <= m_counts[1] && yEnd <= m_counts[1]);
}

template<typename TFunc>
void Clusterer::boxReduction(
	U xBegin, U xEnd, U yBegin, U yEnd, U zBegin, U zEnd, ClustererTestResult& rez, TFunc func) const
{
	U zcount = zEnd - zBegin;
	U ycount = yEnd - yBegin;
	U xcount = xEnd - xBegin;

	if(xcount > ycount && xcount > zcount)
	{
		for(U z = zBegin; z < zEnd; ++z)
		{
			const U zc = (m_counts[0] * m_counts[1]) * z;

			for(U y = yBegin; y < yEnd; ++y)
			{
				const U yc = zc + m_counts[0] * y;

				// Do a reduction to avoid some checks
				U firstX = MAX_U;

				for(U x = xBegin; x < xEnd; ++x)
				{
					U i = yc + x;

					if(func(m_clusterBoxes[i]))
					{
						firstX = x;
						break;
					}
				}

				for(U x = xEnd - 1; x >= firstX; --x)
				{
					U i = yc + x;

					if(func(m_clusterBoxes[i]))
					{
						for(U a = firstX; a <= x; ++a)
						{
							rez.pushBack(a, y, z);
						}

						break;
					}
				}
			}
		}
	}
	else if(ycount > xcount && ycount > zcount)
	{
		for(U z = zBegin; z < zEnd; ++z)
		{
			const U zc = (m_counts[0] * m_counts[1]) * z;

			for(U x = xBegin; x < xEnd; ++x)
			{
				// Do a reduction to avoid some checks
				U firstY = MAX_U;

				for(U y = yBegin; y < yEnd; ++y)
				{
					const U i = zc + m_counts[0] * y + x;

					if(func(m_clusterBoxes[i]))
					{
						firstY = y;
						break;
					}
				}

				for(U y = yEnd - 1; y >= firstY; --y)
				{
					const U i = zc + m_counts[0] * y + x;

					if(func(m_clusterBoxes[i]))
					{
						for(U a = firstY; a <= y; ++a)
						{
							rez.pushBack(x, a, z);
						}

						break;
					}
				}
			}
		}
	}
	else
	{
		for(U y = yBegin; y < yEnd; ++y)
		{
			for(U x = xBegin; x < xEnd; ++x)
			{
				// Do a reduction to avoid some checks
				U firstZ = MAX_U;

				for(U z = zBegin; z < zEnd; ++z)
				{
					const U i = (m_counts[0] * m_counts[1]) * z + m_counts[0] * y + x;

					if(func(m_clusterBoxes[i]))
					{
						firstZ = z;
						break;
					}
				}

				for(U z = zEnd - 1; z >= firstZ; --z)
				{
					const U i = (m_counts[0] * m_counts[1]) * z + m_counts[0] * y + x;

					if(func(m_clusterBoxes[i]))
					{
						for(U a = firstZ; a <= z; ++a)
						{
							rez.pushBack(x, y, a);
						}

						break;
					}
				}
			}
		}
	}
}

void Clusterer::binSphere(const Sphere& s, const Aabb& aabb, ClustererTestResult& rez) const
{
	// Move the sphere to view space
	Vec4 cVSpace = (m_viewMat * s.getCenter().xyz1()).xyz0();
	Sphere sphere(cVSpace, s.getRadius());

	// Compute a new AABB and clip it
	Aabb box;
	sphere.computeAabb(box);

	F32 maxz = min(box.getMax().z(), -m_near - EPSILON);
	box.setMax(Vec4(box.getMax().xy(), maxz, 0.0));
	ANKI_ASSERT(box.getMax().xyz() > box.getMin().xyz());

	// Quick reduction
	U xBegin, xEnd, yBegin, yEnd, zBegin, zEnd;
	computeSplitRange(s, zBegin, zEnd);
	quickReduction(box, m_projMat, xBegin, xEnd, yBegin, yEnd);

	// Detailed
	boxReduction(xBegin, xEnd, yBegin, yEnd, zBegin, zEnd, rez, [&](const Aabb& aabb) {
		return testCollisionShapes(sphere, aabb);
	});
}

void Clusterer::binGeneric(const CollisionShape& cs, const Aabb& box0, ClustererTestResult& rez) const
{
	// Move the box to view space
	Aabb box = box0.getTransformed(Transform(m_viewMat));

	F32 maxz = min(box.getMax().z(), -m_near - EPSILON);
	box.setMax(Vec4(box.getMax().xy(), maxz, 0.0));
	ANKI_ASSERT(box.getMax().xyz() > box.getMin().xyz());

	// Quick reduction
	U xBegin, xEnd, yBegin, yEnd, zBegin, zEnd;
	computeSplitRange(cs, zBegin, zEnd);
	quickReduction(box, m_projMat, xBegin, xEnd, yBegin, yEnd);

	// Detailed
	binGenericRecursive(cs, xBegin, xEnd, yBegin, yEnd, zBegin, zEnd, rez);
}

void Clusterer::binGenericRecursive(
	const CollisionShape& cs, U xBegin, U xEnd, U yBegin, U yEnd, U zBegin, U zEnd, ClustererTestResult& rez) const
{
	U my = (yEnd - yBegin) / 2;
	U mx = (xEnd - xBegin) / 2;

	// Handle final
	if(ANKI_UNLIKELY(my == 0 && mx == 0))
	{
		for(U z = zBegin; z < zEnd; ++z)
		{
			rez.pushBack(xBegin, yBegin, z);
		}
		return;
	}

	// Handle the edge case
	if(ANKI_UNLIKELY(mx == 0 || my == 0))
	{
		if(mx == 0)
		{
			const Plane& topPlane = m_planesYW[yBegin + my - 1];
			F32 test = cs.testPlane(topPlane);

			if(test <= 0.0)
			{
				binGenericRecursive(cs, xBegin, xEnd, yBegin, yBegin + my, zBegin, zEnd, rez);
			}

			if(test >= 0.0)
			{
				binGenericRecursive(cs, xBegin, xEnd, yBegin + my, yEnd, zBegin, zEnd, rez);
			}
		}
		else
		{
			const Plane& rightPlane = m_planesXW[xBegin + mx - 1];
			F32 test = cs.testPlane(rightPlane);

			if(test <= 0.0)
			{
				binGenericRecursive(cs, xBegin, xBegin + mx, yBegin, yEnd, zBegin, zEnd, rez);
			}

			if(test >= 0.0)
			{
				binGenericRecursive(cs, xBegin + mx, xEnd, yBegin, yEnd, zBegin, zEnd, rez);
			}
		}

		return;
	}

	// Do the checks
	Bool inside[2][2] = {{false, false}, {false, false}};

	// Top looking plane check
	{
		// Pick the correct top lookin plane (y)
		const Plane& topPlane = m_planesYW[yBegin + my - 1];

		F32 test = cs.testPlane(topPlane);
		if(test < 0.0)
		{
			inside[0][0] = inside[0][1] = true;
		}
		else if(test > 0.0)
		{
			inside[1][0] = inside[1][1] = true;
		}
		else
		{
			// Possibly all inside
			for(U i = 0; i < 2; i++)
			{
				for(U j = 0; j < 2; j++)
				{
					inside[i][j] = true;
				}
			}
		}
	}

	// Right looking plane check
	{
		// Pick the correct right looking plane (x)
		const Plane& rightPlane = m_planesXW[xBegin + mx - 1];

		F32 test = cs.testPlane(rightPlane);
		if(test < 0.0)
		{
			inside[0][1] = inside[1][1] = false;
		}
		else if(test > 0.0)
		{
			inside[0][0] = inside[1][0] = false;
		}
		else
		{
			// Do nothing and keep the top looking plane check results
		}
	}

	// Now move lower to the hierarchy
	if(inside[0][0])
	{
		binGenericRecursive(cs, xBegin, xBegin + mx, yBegin, yBegin + my, zBegin, zEnd, rez);
	}

	if(inside[0][1])
	{
		binGenericRecursive(cs, xBegin + mx, xEnd, yBegin, yBegin + my, zBegin, zEnd, rez);
	}

	if(inside[1][0])
	{
		binGenericRecursive(cs, xBegin, xBegin + mx, yBegin + my, yEnd, zBegin, zEnd, rez);
	}

	if(inside[1][1])
	{
		binGenericRecursive(cs, xBegin + mx, xEnd, yBegin + my, yEnd, zBegin, zEnd, rez);
	}
}

void Clusterer::binPerspectiveFrustum(const PerspectiveFrustum& fr, const Aabb& box0, ClustererTestResult& rez) const
{
	rez.m_count = 0;

	// Move the box to view space
	Aabb box = box0.getTransformed(Transform(m_viewMat));

	F32 maxz = min(box.getMax().z(), -m_near - EPSILON);
	box.setMax(Vec4(box.getMax().xy(), maxz, 0.0));
	ANKI_ASSERT(box.getMax().xyz() > box.getMin().xyz());

	// Quick reduction
	U xBegin, xEnd, yBegin, yEnd, zBegin, zEnd;
	computeSplitRange(fr, zBegin, zEnd);
	quickReduction(box, m_projMat, xBegin, xEnd, yBegin, yEnd);

	// Detailed tests
	Array<Plane, 5> vspacePlanes;
	for(U i = 0; i < vspacePlanes.getSize(); ++i)
	{
		vspacePlanes[i] = fr.getPlanesWorldSpace()[i + 1].getTransformed(Transform(m_viewMat));
	}

	boxReduction(xBegin, xEnd, yBegin, yEnd, zBegin, zEnd, rez, [&](const Aabb& aabb) {
		for(const Plane& p : vspacePlanes)
		{
			if(aabb.testPlane(p) < 0.0)
			{
				return false;
			}
		}

		return true;
	});
}

void Clusterer::update(U32 threadId, PtrSize threadsCount, Bool frustumChanged)
{
	PtrSize start, end;

	const Transform& trf = m_camTrf;
	const Vec4& projParams = m_unprojParams;

	if(frustumChanged)
	{
		// Re-calculate the planes in local space

		// First the top looking planes
		ThreadPoolTask::choseStartEnd(threadId, threadsCount, m_planesYW.getSize(), start, end);

		for(U i = start; i < end; i++)
		{
			calcPlaneY(i, projParams);

			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		ThreadPoolTask::choseStartEnd(threadId, threadsCount, m_planesXW.getSize(), start, end);

		for(U j = start; j < end; j++)
		{
			calcPlaneX(j, projParams);

			m_planesXW[j] = m_planesX[j].getTransformed(trf);
		}

		// The boxes
		ThreadPoolTask::choseStartEnd(threadId, threadsCount, m_clusterBoxes.getSize(), start, end);
		setClusterBoxes(projParams, start, end);
	}
	else
	{
		// Only transform planes

		// First the top looking planes
		ThreadPoolTask::choseStartEnd(threadId, threadsCount, m_planesYW.getSize(), start, end);

		for(U i = start; i < end; i++)
		{
			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		ThreadPoolTask::choseStartEnd(threadId, threadsCount, m_planesXW.getSize(), start, end);

		for(U j = start; j < end; j++)
		{
			m_planesXW[j] = m_planesX[j].getTransformed(trf);
		}
	}

	// Finaly tranform the near and far planes
	if(threadId == 0)
	{
		*m_nearPlane = Plane(Vec4(0.0, 0.0, -1.0, 0.0), m_near);
		m_nearPlane->transform(trf);

		*m_farPlane = Plane(Vec4(0.0, 0.0, 1.0, 0.0), -m_far);
		m_farPlane->transform(trf);
	}
}

void Clusterer::debugDraw(ClustererDebugDrawer& drawer) const
{
}

void Clusterer::debugDrawResult(const ClustererTestResult& rez, ClustererDebugDrawer& drawer) const
{
	const Vec4& projParams = m_unprojParams;

	auto it = rez.getClustersBegin();
	auto end = rez.getClustersEnd();
	while(it != end)
	{
		const auto& id = *it;
		Array<Vec3, 8> frustumPoints;
		U count = 0;

		for(U z = id.z(); z <= id.z() + 1u; ++z)
		{
			F32 zVSpace = -calcNear(z);

			for(U y = id.y(); y <= id.y() + 1u; ++y)
			{
				F32 yNdc = F32(y) / m_counts[1] * 2.0 - 1.0;

				for(U x = id.x(); x <= id.x() + 1u; ++x)
				{
					F32 xNdc = F32(x) / m_counts[0] * 2.0 - 1.0;

					frustumPoints[count++] = unprojectZViewSpace(zVSpace, Vec2(xNdc, yNdc), projParams).xyz();
				}
			}
		}

		ANKI_ASSERT(count == 8);
		static const Vec3 COLOR(1.0);
		drawer(frustumPoints[0], frustumPoints[1], COLOR);
		drawer(frustumPoints[1], frustumPoints[3], COLOR);
		drawer(frustumPoints[3], frustumPoints[2], COLOR);
		drawer(frustumPoints[0], frustumPoints[2], COLOR);

		drawer(frustumPoints[4], frustumPoints[5], COLOR);
		drawer(frustumPoints[5], frustumPoints[7], COLOR);
		drawer(frustumPoints[7], frustumPoints[6], COLOR);
		drawer(frustumPoints[4], frustumPoints[6], COLOR);

		drawer(frustumPoints[0], frustumPoints[4], COLOR);
		drawer(frustumPoints[1], frustumPoints[5], COLOR);
		drawer(frustumPoints[2], frustumPoints[6], COLOR);
		drawer(frustumPoints[3], frustumPoints[7], COLOR);

		++it;
	}
}

} // end namespace anki
