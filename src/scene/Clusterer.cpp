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
void Clusterer::prepare(FrustumComponent* frc,
	const SArray<Vec2>& minMaxTileDepth)
{
	ANKI_ASSERT(frc);

	if(frc == m_frc && m_frc->getTimestamp() == m_frcTimestamp)
	{
		// Not dirty, early exit
		return;
	}

	m_frc = frc;
	m_frcTimestamp = m_frc->getTimestamp();

	// Calc depth
	ANKI_ASSERT(frc->getFrustum().getType() == Frustum::Type::PERSPECTIVE);
	const PerspectiveFrustum& fr =
		static_cast<const PerspectiveFrustum&>(frc->getFrustum());

	F32 near = fr.getNear();
	F32 far = fr.getFar();
	F32 Sy = m_counts[1];
	F32 theta = fr.getFovY() / 2.0;
	F32 opt = 1.0 + (4.0 * tan(theta)) / Sy;

	U k = 1;
	while(1)
	{
		F32 neark = near * pow(opt, F32(k));
		if(neark >= far)
		{
			break;
		}
		++k;
	}
	m_counts[2] = k;

	// Alloc clusters
	U clusterCount = m_counts[0] * m_counts[1] * m_counts[2];
	if(clusterCount != m_clusters.getSize())
	{
		m_clusters.resize(m_alloc, clusterCount);
	}
}

} // end namespace anki
