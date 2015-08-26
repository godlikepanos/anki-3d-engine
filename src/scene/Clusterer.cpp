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
	F32 Sy = m_counts[1];
	F32 theta = m_fovY / 2.0;
	F32 a = 1.0 + (4.0 * tan(theta)) / Sy;
	F32 neark = m_near * pow(a, F32(k));
	return neark;
}

//==============================================================================
void Clusterer::prepare(FrustumComponent* frc,
	const SArray<Vec2>& minMaxTileDepth)
{
	ANKI_ASSERT(frc);

	// Get some things
	ANKI_ASSERT(frc->getFrustum().getType() == Frustum::Type::PERSPECTIVE);
	const PerspectiveFrustum& fr =
		static_cast<const PerspectiveFrustum&>(frc->getFrustum());

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

	// Calc depth
	U k = 1;
	while(1)
	{
		F32 neark = calcNear(k);
		if(neark >= far)
		{
			break;
		}
		++k;
	}
	m_counts[2] = k + 1;

	// Alloc clusters
	U clusterCount = m_counts[0] * m_counts[1] * m_counts[2];
	if(clusterCount != m_clusters.getSize())
	{
		m_clusters.resize(m_alloc, clusterCount);
	}
}

//==============================================================================
void Clusterer::initClusters()
{
	for(U z = 0; z < U(m_counts[2] - 1); ++z)
	{
		for(U y = 0; y < m_counts[1]; ++y)
		{
			for(U x = 0; x < m_counts[0]; ++x)
			{
				// Compute projection matrix
				F32 near = calcNear(z);
				F32 far = calcNear(z + 1);

				Mat4 proj;
				PerspectiveFrustum::calculateProjectionMatrix(m_fovX, m_fovY,
					near, far, proj);

				Mat4 invProj = proj.getInverse();

				// Project some points
				Vec2 tileMin, tileMax;
				tileMin.x() = F32(x) / m_counts[0];
				tileMax.x() = F32(x + 1) / m_counts[0];
				tileMin.y() = F32(y) / m_counts[1];
				tileMax.y() = F32(y + 1) / m_counts[1];

				tileMin = tileMin * 2.0 - 1.0;
				tileMax = tileMax * 2.0 - 1.0;

				Aabb box;
				Vec4 a = invProj * Vec4(tileMin, 0.0, 1.0);
				box.setMin(a.xyz0());
				a = invProj * Vec4(tileMax, 1.0, 1.0);
				box.setMax(a.xyz0());

				cluster(x, y, z).m_box = box;
			}
		}
	}
}

} // end namespace anki

