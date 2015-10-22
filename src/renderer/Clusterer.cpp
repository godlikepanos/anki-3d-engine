// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Clusterer.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/util/Rtti.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
class UpdatePlanesPerspectiveCameraTask: public ThreadPool::Task
{
public:
	Clusterer* m_clusterer = nullptr;
	Bool m_frustumChanged;

	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		m_clusterer->update(threadId, threadsCount, m_frustumChanged);
		return ErrorCode::NONE;
	}
};

//==============================================================================
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

//==============================================================================
// Clusterer                                                                   =
//==============================================================================

//==============================================================================
Clusterer::~Clusterer()
{
	m_allPlanes.destroy(m_alloc);
}

//==============================================================================
void Clusterer::initTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
	ClustererTestResult& rez) const
{
	rez.m_clusterIds.create(alloc, getClusterCount());
	rez.m_count = 0;
	rez.m_alloc = alloc;
}

//==============================================================================
F32 Clusterer::calcNear(U k) const
{
	F32 neark = m_calcNearOpt * pow(k, 2.0) + m_near;
	return neark;
}

//==============================================================================
U Clusterer::calcZ(F32 zVspace) const
{
	zVspace = clamp(zVspace, -m_far, -m_near);
	zVspace = -zVspace;
	F32 fk = sqrt((zVspace - m_near) / m_calcNearOpt);
	ANKI_ASSERT(fk >= 0.0);
	U k = min<U>(static_cast<U>(fk), m_counts[2]);

	return k;
}

//==============================================================================
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

//==============================================================================
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

//==============================================================================
void Clusterer::init(const GenericMemoryPoolAllocator<U8>& alloc,
	U clusterCountX, U clusterCountY, U clusterCountZ)
{
	m_alloc = alloc;
	m_counts[0] = clusterCountX;
	m_counts[1] = clusterCountY;
	m_counts[2] = clusterCountZ;

	// Init planes. One plane for each direction, plus near/far plus the world
	// space of those
	U planesCount =
		(m_counts[0] - 1) * 2 // planes J
		+ (m_counts[1] - 1) * 2 // planes I
		+ 2; // Near and far planes

	m_allPlanes.create(m_alloc, planesCount);

	Plane* base = &m_allPlanes[0];
	U count = 0;

	m_planesX = std::move(SArray<Plane>(base + count, m_counts[0] - 1));
	count += m_planesX.getSize();

	m_planesY = std::move(SArray<Plane>(base + count, m_counts[1] - 1));
	count += m_planesY.getSize();

	m_planesXW = std::move(SArray<Plane>(base + count, m_counts[0] - 1));
	count += m_planesXW.getSize();

	m_planesYW = std::move(SArray<Plane>(base + count, m_counts[1] - 1));
	count += m_planesYW.getSize();

	m_nearPlane = base + count;
	++count;

	m_farPlane = base + count;
	++count;

	ANKI_ASSERT(count == m_allPlanes.getSize());
}

//==============================================================================
void Clusterer::prepare(ThreadPool& threadPool, const SceneNode& node)
{
	// Get some things
	const FrustumComponent& frc = node.getComponent<FrustumComponent>();
	Timestamp frcTimestamp = frc.getTimestamp();
	const Frustum& fr = frc.getFrustum();
	ANKI_ASSERT(fr.getType() == Frustum::Type::PERSPECTIVE);
	const PerspectiveFrustum& pfr = static_cast<const PerspectiveFrustum&>(fr);

	// Set some things
	m_node = &node;
	m_frc = &frc;
	m_near = pfr.getNear();
	m_far = pfr.getFar();
	m_calcNearOpt = (m_far - m_near) / pow(m_counts[2], 2.0);

	//
	// Issue parallel jobs
	//
	Array<UpdatePlanesPerspectiveCameraTask, ThreadPool::MAX_THREADS> jobs;

	// Do a job that transforms only the planes when:
	// - it's the same frustum component as before and
	// - the component has not changed
	Bool frustumChanged =
		frcTimestamp >= m_planesLSpaceTimestamp || m_node != &node;

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].m_clusterer = this;
		jobs[i].m_frustumChanged = frustumChanged;
		threadPool.assignNewTask(i, &jobs[i]);
	}

	// Update timestamp
	if(frustumChanged)
	{
		m_planesLSpaceTimestamp = frcTimestamp;
	}

	// Sync threads
	Error err = threadPool.waitForAllThreadsToFinish();
	(void)err;
}

//==============================================================================
void Clusterer::computeSplitRange(const CollisionShape& cs, U& zBegin,
	U& zEnd) const
{
	// Find the distance between cs and near plane
	F32 dist = cs.testPlane(*m_nearPlane);
	dist = max(0.0f, dist);

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

//==============================================================================
void Clusterer::bin(const CollisionShape& cs, const Aabb& csBox,
	ClustererTestResult& rez) const
{
	rez.m_count = 0;

	if(isa<Sphere>(cs))
	{
		binSphere(dcast<const Sphere&>(cs), csBox, rez);
	}
	else
	{
		U zBegin, zEnd;
		computeSplitRange(cs, zBegin, zEnd);
		binGeneric(cs, 0, m_counts[0], 0, m_counts[1], zBegin, zEnd, rez);
	}
}

//==============================================================================
void Clusterer::binSphere(const Sphere& s, const Aabb& aabb,
	ClustererTestResult& rez) const
{
	const Mat4& vp = m_frc->getViewProjectionMatrix();
	const Mat4& v = m_frc->getViewMatrix();

	const Vec4& scent = s.getCenter();
	const F32 srad = s.getRadius();

	U zBegin, zEnd;
	computeSplitRange(s, zBegin, zEnd);

	// Do a quick check
	Vec4 eye = m_frc->getFrustumOrigin() - scent;
	if(ANKI_UNLIKELY(eye.getLengthSquared() <= srad * srad))
	{
		// Camera totaly inside the sphere
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
		return;
	}

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
		p = vp * p;
		p /= abs(p.w());

		for(U i = 0; i < 2; ++i)
		{
			min2[i] = min(min2[i], p[i]);
			max2[i] = max(max2[i], p[i]);
		}
	}

	min2 = min2 * 0.5 + 0.5;
	max2 = max2 * 0.5 + 0.5;

	// Do a box test
	F32 tcountX = m_counts[0];
	F32 tcountY = m_counts[1];

	I xBegin = floor(tcountX * min2.x());
	xBegin = clamp<I>(xBegin, 0, m_counts[0]);

	I xEnd = ceil(tcountX * max2.x());
	xEnd = min<U>(xEnd, m_counts[0]);

	I yBegin = floor(tcountY * min2.y());
	yBegin = clamp<I>(yBegin, 0, m_counts[1]);

	I yEnd = ceil(tcountY * max2.y());
	yEnd = min<I>(yEnd, m_counts[1]);

	ANKI_ASSERT(xBegin >= 0 && xBegin <= tcountX
		&& xEnd >= 0 && xEnd <= tcountX);
	ANKI_ASSERT(yBegin >= 0 && yBegin <= tcountX
		&& yEnd >= 0 && yBegin <= tcountY);

	Vec2 tileSize(1.0 / tcountX, 1.0 / tcountY);

	Vec4 a = vp * s.getCenter().xyz1();
	Vec2 c = a.xy() / a.w();
	c = c * 0.5 + 0.5;

	Vec4 sphereCenterVSpace = (v * scent.xyz1()).xyz0();

	for(I y = yBegin; y < yEnd; ++y)
	{
		for(I x = xBegin; x < xEnd; ++x)
		{
			// Do detailed tests

			Vec2 tileMin = Vec2(x, y) * tileSize;
			Vec2 tileMax = Vec2(x + 1, y + 1) * tileSize;

			// Find closest point of sphere center and tile
			Vec2 cp(0.0);
			for(U i = 0; i < 2; ++i)
			{
				if(c[i] > tileMax[i])
				{
					cp[i] = tileMax[i];
				}
				else if (c[i] < tileMin[i])
				{
					cp[i] = tileMin[i];
				}
				else
				{
					// the c lies between min and max
					cp[i] = c[i];
				}
			}

			// Unproject the closest point to view space
			Vec4 view = unproject(
				1.0, cp * 2.0 - 1.0, m_frc->getProjectionParameters());

			// Do a simple ray-sphere test
			Vec4 dir = view;
			Vec4 proj = sphereCenterVSpace.projectTo(dir);
			F32 lenSq = (sphereCenterVSpace - proj).getLengthSquared();
			Bool inside = lenSq <= (srad * srad);

			if(inside)
			{
				for(U z = zBegin; z < zEnd; ++z)
				{
					rez.pushBack(x, y, z);
				}
			}
		}
	}
}

//==============================================================================
void Clusterer::binGeneric(const CollisionShape& cs, U xBegin, U xEnd, U yBegin,
	U yEnd, U zBegin, U zEnd, ClustererTestResult& rez) const
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
				binGeneric(cs, xBegin, xEnd, yBegin, yBegin + my, zBegin, zEnd,
					rez);
			}

			if(test >= 0.0)
			{
				binGeneric(cs, xBegin, xEnd, yBegin + my, yEnd, zBegin, zEnd,
					rez);
			}
		}
		else
		{
			const Plane& rightPlane = m_planesXW[xBegin + mx - 1];
			F32 test = cs.testPlane(rightPlane);

			if(test <= 0.0)
			{
				binGeneric(cs, xBegin, xBegin + mx, yBegin, yEnd, zBegin, zEnd,
					rez);
			}

			if(test >= 0.0)
			{
				binGeneric(cs, xBegin + mx, xEnd, yBegin, yEnd, zBegin, zEnd,
					rez);
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
		binGeneric(cs, xBegin, xBegin + mx, yBegin, yBegin + my, zBegin, zEnd,
			rez);
	}

	if(inside[0][1])
	{
		binGeneric(cs, xBegin + mx, xEnd, yBegin, yBegin + my, zBegin, zEnd,
			rez);
	}

	if(inside[1][0])
	{
		binGeneric(cs, xBegin, xBegin + mx, yBegin + my, yEnd, zBegin, zEnd,
			rez);
	}

	if(inside[1][1])
	{
		binGeneric(cs, xBegin + mx, xEnd, yBegin + my, yEnd, zBegin, zEnd,
			rez);
	}
}

//==============================================================================
void Clusterer::update(U32 threadId, PtrSize threadsCount, Bool frustumChanged)
{
	PtrSize start, end;
	const MoveComponent& move = m_node->getComponent<MoveComponent>();
	const FrustumComponent& frc = *m_frc;
	ANKI_ASSERT(frc.getFrustum().getType() == Frustum::Type::PERSPECTIVE);

	const Transform& trf = move.getWorldTransform();
	const Vec4& projParams = frc.getProjectionParameters();

	if(frustumChanged)
	{
		// Re-calculate the planes in local space

		// First the top looking planes
		ThreadPool::Task::choseStartEnd(threadId, threadsCount,
			m_planesYW.getSize(), start, end);

		for(U i = start; i < end; i++)
		{
			calcPlaneY(i, projParams);

			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		ThreadPool::Task::choseStartEnd(threadId, threadsCount,
			m_planesXW.getSize(), start, end);

		for(U j = start; j < end; j++)
		{
			calcPlaneX(j, projParams);

			m_planesXW[j] = m_planesX[j].getTransformed(trf);
		}
	}
	else
	{
		// Only transform planes

		// First the top looking planes
		ThreadPool::Task::choseStartEnd(threadId, threadsCount,
			m_planesYW.getSize(), start, end);

		for(U i = start; i < end; i++)
		{
			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		ThreadPool::Task::choseStartEnd(threadId, threadsCount,
			m_planesXW.getSize(), start, end);

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

} // end namespace anki

