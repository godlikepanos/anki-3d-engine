// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Clusterer.h"
#include "anki/scene/FrustumComponent.h"

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
	F32 neark = m_near * pow(m_calcNearOpt, F32(k));
	return neark;
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

	{
		F32 Sy = m_counts[1];
		F32 theta = m_fovY / 2.0;
		m_calcNearOpt = 1.0 + (2.0 * tan(theta)) / Sy;
	}

	// Calc depth
	U k = 1;
	while(1)
	{
		F32 neark = calcNear(k++);
		if(neark >= far)
		{
			break;
		}
	}
	m_counts[2] = k - 1;

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
	Mat4 proj;
	PerspectiveFrustum::calculateProjectionMatrix(m_fovX, m_fovY, m_near, m_far,
		proj);
	Mat4 invProj = proj.getInverse();

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
				Plane p(Vec4(0.0, 0.0, -1.0, 0.0), near);
				p.intersectVector(min, min);
				min.z() = -far;

				Vec4 max = invProj * Vec4(tileMax, 0.0, 1.0);
				max.w() = 0.0;
				p = Plane(Vec4(0.0, 0.0, -1.0, 0.0), near);
				p.intersectVector(max, max);

				cluster(x, y, z).m_min = min.xyz();
				cluster(x, y, z).m_max = max.xyz();
			}
		}
	}
}

} // end namespace anki

