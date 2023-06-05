// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/StatsSet.h>

namespace anki {

#if ANKI_STATS_ENABLED
StatsSet::~StatsSet()
{
	if(m_statCounterArr)
	{
		ANKI_ASSERT(m_statCounterArrSize > 0);
		free(m_statCounterArr);
		m_statCounterArr = nullptr;
		m_statCounterArrSize = 0;
		m_statCounterArrStorageSize = 0;
	}
}

void StatsSet::endFrame()
{
	for(U32 i = 0; i < m_statCounterArrSize; ++i)
	{
		StatCounter& counter = *m_statCounterArr[i];
		const Bool needsReset = !!(counter.m_flags & StatFlag::kZeroEveryFrame);
		const Bool atomic = !!(counter.m_flags & StatFlag::kThreadSafe);
		if(needsReset && atomic)
		{
			counter.m_atomic.store(0);
		}
		else if(needsReset)
		{
			counter.m_u = 0;
		}
	}
}

void StatsSet::registerCounter(StatCounter* counter)
{
	ANKI_ASSERT(counter);

	// Try grow the array
	if(m_statCounterArrSize + 1 > m_statCounterArrStorageSize)
	{
		m_statCounterArrStorageSize = max(2u, m_statCounterArrStorageSize * 2);

		StatCounter** newArr = static_cast<StatCounter**>(malloc(sizeof(StatCounter*) * m_statCounterArrStorageSize));

		if(m_statCounterArrSize > 0)
		{
			memcpy(newArr, m_statCounterArr, sizeof(StatCounter*) * m_statCounterArrSize);
			free(m_statCounterArr);
		}

		m_statCounterArr = newArr;
	}

	m_statCounterArr[m_statCounterArrSize++] = counter;

	std::sort(m_statCounterArr, m_statCounterArr + m_statCounterArrSize, [](const StatCounter* a, const StatCounter* b) {
		if(a->m_category != b->m_category)
		{
			return a->m_category < b->m_category;
		}
		else
		{
			return strcmp(a->m_name, b->m_name) < 0;
		}
	});
}
#endif

} // end namespace anki
