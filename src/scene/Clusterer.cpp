// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Clusterer.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/util/Rtti.h"

namespace anki {

//==============================================================================
void Clusterer::initTempTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
	ClustererTestResult& rez) const
{
	rez.m_clusterIds.create(alloc, m_clusters.getSize());
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
U Clusterer::calcK(F32 zVspace) const
{
	zVspace = clamp(zVspace, -m_far, -m_near);
	zVspace = -zVspace;
	F32 fk = sqrt((zVspace - m_near) / m_calcNearOpt);
	ANKI_ASSERT(fk >= 0.0);
	U k = min<U>(static_cast<U>(fk), m_counts[2]);

	return k;
}

//==============================================================================
void Clusterer::prepare(const PerspectiveFrustum& fr,
	const SArray<Vec2>& minMaxTileDepth)
{
	// Get some things
	F32 near = fr.getNear();
	F32 far = fr.getFar();
	F32 fovY = fr.getFovY();
	F32 fovX = fr.getFovX();

	if(near == m_near && far == m_far && m_fovY == fovY && m_fovX == fovX)
	{
		// Not dirty, early exit
		return;
	}

	m_fovY = fovY;
	m_fovX = fovX;
	m_near = near;
	m_far = far;

	m_calcNearOpt = (m_far - m_near) / pow(m_counts[2], 2.0);

	// Alloc and init clusters
	U clusterCount = m_counts[0] * m_counts[1] * m_counts[2];
	if(clusterCount != m_clusters.getSize())
	{
		m_clusters.resize(m_alloc, clusterCount);
	}

	initClusters();
}

//==============================================================================
void Clusterer::initClusters()
{
	PerspectiveFrustum::calculateProjectionMatrix(m_fovX, m_fovY, m_near, m_far,
		m_projMat);
	Mat4 invProj = m_projMat.getInverse();

	// For every claster
	for(U z = 0; z < m_counts[2]; ++z)
	{
		for(U y = 0; y < m_counts[1]; ++y)
		{
			for(U x = 0; x < m_counts[0]; ++x)
			{
				// Compute projection matrix
				F32 near = calcNear(z);
				F32 far = calcNear(z + 1);
				if(far > m_far)
				{
					far = m_far;
				}

				// Project some points
				Vec2 tileMin, tileMax;
				tileMin.x() = F32(x) / m_counts[0];
				tileMax.x() = F32(x + 1) / m_counts[0];
				tileMin.y() = F32(y) / m_counts[1];
				tileMax.y() = F32(y + 1) / m_counts[1];

				tileMin = tileMin * 2.0 - 1.0;
				tileMax = tileMax * 2.0 - 1.0;

				Vec4 min = invProj * Vec4(tileMin, 0.0, 1.0);
				min.w() = 0.0;
				Plane p(Vec4(0.0, 0.0, -1.0, 0.0), far);
				p.intersectVector(min, min);

				Vec4 max = invProj * Vec4(tileMax, 0.0, 1.0);
				max.w() = 0.0;
				p.intersectVector(max, max);
				max.z() = -near;

				ANKI_ASSERT(min.xyz() < max.xyz());
				cluster(x, y, z).m_min = min.xyz();
				cluster(x, y, z).m_max = max.xyz();
			}
		}
	}
}

//==============================================================================
void Clusterer::bin(const CollisionShape& cs, ClustererTestResult& rez) const
{
	rez.m_count = 0;

	if(isa<Sphere>(cs))
	{
		// Specialized
		binSphere(dcast<const Sphere&>(cs), rez);
	}
	else
	{
		binGeneric(cs, rez);
	}
}

//==============================================================================
void Clusterer::findTilesFromAabb(const Aabb& box, U& tileBeginX, U& tileBeginY,
	U& tileEndX, U& tileEndY) const
{
	// Project front face of the AABB
	const Vec4& minv = box.getMin();
	const Vec4& maxv = box.getMax();

	Array<Vec4, 8> projPoints;
	projPoints[0] = Vec4(minv.x(), minv.y(), maxv.z(), 1.0);
	projPoints[1] = Vec4(maxv.x(), minv.y(), maxv.z(), 1.0);
	projPoints[2] = Vec4(maxv.x(), maxv.y(), maxv.z(), 1.0);
	projPoints[3] = Vec4(minv.x(), maxv.y(), maxv.z(), 1.0);
	projPoints[4] = Vec4(minv.x(), minv.y(), minv.z(), 1.0);
	projPoints[5] = Vec4(maxv.x(), minv.y(), minv.z(), 1.0);
	projPoints[6] = Vec4(maxv.x(), maxv.y(), minv.z(), 1.0);
	projPoints[7] = Vec4(minv.x(), maxv.y(), minv.z(), 1.0);

	Vec2 min2(MAX_F32), max2(MIN_F32);
	for(Vec4& p : projPoints)
	{
		p = m_projMat * p;
		p /= abs(p.w());

		for(U i = 0; i < 2; ++i)
		{
			min2[i] = min(min2[i], p[i]);
			max2[i] = max(max2[i], p[i]);
		}
	}

	min2 = min2 * 0.5 + 0.5;
	max2 = max2 * 0.5 + 0.5;

	// Find tiles affected
	F32 tcountX = m_counts[0];
	F32 tcountY = m_counts[1];

	I xFrom = floor(tcountX * min2.x());
	xFrom = clamp<I>(xFrom, 0, tcountX);

	I xTo = ceil(tcountX * max2.x());
	xTo = min<U>(xTo, tcountX);

	I yFrom = floor(tcountY * min2.y());
	yFrom = clamp<I>(yFrom, 0, tcountY);

	I yTo = ceil(tcountY * max2.y());
	yTo = min<I>(yTo, tcountY);

	ANKI_ASSERT(xFrom >= 0 && xFrom <= tcountX
		&& xTo >= 0 && xTo <= tcountX);
	ANKI_ASSERT(yFrom >= 0 && yFrom <= tcountX
		&& yTo >= 0 && yFrom <= tcountY);

	tileBeginX = xFrom;
	tileBeginY = yFrom;
	tileEndX = xTo;
	tileEndY = yTo;
}

//==============================================================================
void Clusterer::findSplitsFromAabb(const Aabb& box, U& zFrom, U& zTo) const
{
	zFrom = calcK(box.getMax().z());
	zTo = calcK(box.getMin().z()) + 1;
	ANKI_ASSERT(zFrom <= zTo);
}

//==============================================================================
void Clusterer::binSphere(const Sphere& s, ClustererTestResult& rez) const
{
	Aabb aabb;
	s.computeAabb(aabb);

	const Vec4& scent = s.getCenter();
	const F32 srad = s.getRadius();

	// Find the tiles that are covered
	//

	U tileBeginX, tileBeginY, tileEndX, tileEndY;

	if(ANKI_UNLIKELY(scent.getLengthSquared() <= srad * srad))
	{
		// The eye is inside the sphere. All tiles are covered
		tileBeginX = tileBeginY = 0;
		tileEndX = m_counts[0];
		tileEndY = m_counts[1];
	}
	else
	{
		findTilesFromAabb(aabb, tileBeginX, tileBeginY, tileEndX, tileEndY);
	}

	U zFrom, zTo;
	findSplitsFromAabb(aabb, zFrom, zTo);

	// Detailed tests
	for(U z = zFrom; z < zTo; ++z)
	{
		for(U y = tileBeginY; y < tileEndY; ++y)
		{
			for(U x = tileBeginX; x < tileEndX; ++x)
			{
				const Cluster& cl = cluster(x, y, z);
				Aabb clusterAabb(cl.m_min.xyz0(), cl.m_max.xyz0());
				if(testCollisionShapes(s, clusterAabb))
				{
					Array<U8, 3> ids = {static_cast<U8>(x),
						static_cast<U8>(y), static_cast<U8>(z)};
					rez.m_clusterIds[rez.m_count++] = ids;
				}
			}
		}
	}
}

//==============================================================================
void Clusterer::binGeneric(const CollisionShape& cs,
	ClustererTestResult& rez) const
{
	Aabb aabb;
	cs.computeAabb(aabb);

	U tileBeginX, tileBeginY, tileEndX, tileEndY;
	if(ANKI_UNLIKELY(aabb.getMax().z() >= -m_near))
	{
		// The eye is inside the sphere. All tiles are covered
		tileBeginX = tileBeginY = 0;
		tileEndX = m_counts[0];
		tileEndY = m_counts[1];
	}
	else
	{
		findTilesFromAabb(aabb, tileBeginX, tileBeginY, tileEndX, tileEndY);
	}

	U zFrom, zTo;
	findSplitsFromAabb(aabb, zFrom, zTo);

	// Detailed tests
	for(U z = zFrom; z < zTo; ++z)
	{
		for(U y = tileBeginY; y < tileEndY; ++y)
		{
			for(U x = tileBeginX; x < tileEndX; ++x)
			{
				const Cluster& cl = cluster(x, y, z);
				Aabb clusterAabb(cl.m_min.xyz0(), cl.m_max.xyz0());
				if(testCollisionShapes(cs, clusterAabb))
				{
					Array<U8, 3> ids = {static_cast<U8>(x),
						static_cast<U8>(y), static_cast<U8>(z)};
					rez.m_clusterIds[rez.m_count++] = ids;
				}
			}
		}
	}
}

} // end namespace anki

