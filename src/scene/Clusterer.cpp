// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Clusterer.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/util/Rtti.h"

namespace anki {

//==============================================================================
void Clusterer::initTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
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
void Clusterer::prepare(const PerspectiveFrustum& fr)
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

	if(m_counts[2] != m_splitInfo.getSize())
	{
		m_splitInfo.resize(m_alloc, m_counts[2]);
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

				// Set split info
				if(x == 0 && y == 0)
				{
					Vec2 xy(min.x(), min.y());
					Vec2 sizes(max.x() - min.x(), max.y() - min.y());

					m_splitInfo[z].m_xy = xy;
					m_splitInfo[z].m_sizes = sizes;
				}
			}
		}
	}
}

//==============================================================================
void Clusterer::bin(const CollisionShape& cs, ClustererTestResult& rez) const
{
	rez.m_count = 0;

	Aabb box;
	cs.computeAabb(box);

	U beginZ, endZ;

	// Find splits that cover the box
	findSplitsFromAabb(box, beginZ, endZ);

	// For each split find the x and y clusters
	for(U z = beginZ; z < endZ; ++z)
	{
		const SplitInfo& info = m_splitInfo[z];

		I beginX = -1, endX = -1, beginY = -1, endY = -1;

		// Find beginX
		F32 x = box.getMin().x();
		if(x <= info.m_xy.x())
		{
			beginX = 0;
		}
		else if(x >= (info.m_xy.x() + info.m_sizes.x() * m_counts[0]))
		{
			beginX = m_counts[0];
		}
		else
		{
			beginX = (x - info.m_xy.x()) / info.m_sizes.x();
		}
		ANKI_ASSERT(beginX >= 0 && beginX <= m_counts[0]);

		// Find endX
		x = box.getMax().x();
		if(x <= info.m_xy.x())
		{
			endX = 0;
		}
		else if(x >= (info.m_xy.x() + info.m_sizes.x() * m_counts[0]))
		{
			endX = m_counts[0];
		}
		else
		{
			endX = ceil((x - info.m_xy.x()) / info.m_sizes.x());
		}
		ANKI_ASSERT(endX >= 0 && endX <= m_counts[0]);

		// Find beginY
		F32 y = box.getMin().y();
		if(y <= info.m_xy.y())
		{
			beginY = 0;
		}
		else if(y >= (info.m_xy.y() + info.m_sizes.y() * m_counts[1]))
		{
			beginY = m_counts[1];
		}
		else
		{
			beginY = (y - info.m_xy.y()) / info.m_sizes.y();
		}
		ANKI_ASSERT(beginY >= 0 && beginY <= m_counts[1]);

		// Find endY
		y = box.getMax().y();
		if(y <= info.m_xy.y())
		{
			endY = 0;
		}
		else if(y >= (info.m_xy.y() + info.m_sizes.y() * m_counts[1]))
		{
			endY = m_counts[1];
		}
		else
		{
			endY = ceil((y - info.m_xy.y()) / info.m_sizes.y());
		}
		ANKI_ASSERT(endY >= 0 && endY <= m_counts[1]);

		for(I y = beginY; y < endY; ++y)
		{
			for(I x = beginX; x < endX; ++x)
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

//==============================================================================
void Clusterer::findSplitsFromAabb(const Aabb& box, U& zFrom, U& zTo) const
{
	zFrom = calcK(box.getMax().z());
	zTo = calcK(box.getMin().z()) + 1;
	ANKI_ASSERT(zFrom <= zTo);
}

} // end namespace anki

