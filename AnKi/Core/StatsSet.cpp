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
		const Bool atomic = !(counter.m_flags & StatFlag::kMainThreadUpdates);
		const Bool isFloat = !!(counter.m_flags & StatFlag::kFloat);

		// Store the previous value
		if(isFloat)
		{
			if(atomic)
			{
				LockGuard lock(counter.m_floatLock);
				counter.m_prevValuef = counter.m_f;

				if(needsReset)
				{
					counter.m_f = 0.0;
				}
			}
			else
			{
				counter.m_prevValuef = counter.m_f;

				if(needsReset)
				{
					counter.m_f = 0;
				}
			}
		}
		else
		{
			if(atomic && needsReset)
			{
				counter.m_prevValueu = counter.m_atomic.exchange(0);
			}
			else if(atomic && !needsReset)
			{
				counter.m_prevValueu = counter.m_atomic.load();
			}
			else if(!atomic && needsReset)
			{
				counter.m_prevValueu = counter.m_u;
				counter.m_u = 0;
			}
			else if(!atomic && !needsReset)
			{
				counter.m_prevValueu = counter.m_u;
			}
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
