// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup core
/// @{

enum class StatFlag : U16
{
	kNone = 0,

	kZeroEveryFrame = 1 << 0,
	kThreadSafe = 1 << 1,
	kFloat = 1 << 2,

	kShowAverage = 1 << 3,
	kSecond = (1 << 4) | kFloat,
	kMilisecond = (1 << 5) | kFloat,
	kNanoSeconds = 1 << 6,
	kBytes = 1 << 7,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(StatFlag)

enum class StatCategory : U8
{
	kTime,
	kCpuMem,
	kGpuMem,
	kGpuMisc,
	kRenderer,
	kMisc,

	kCount,
};

inline constexpr Array<CString, U32(StatCategory::kCount)> kStatCategoryTexts = {"Time", "CPU memory", "GPU memory", "GPU misc", "Renderer", "Misc"};

/// A stats counter.
class StatCounter
{
	friend class StatsSet;

public:
	/// Construct.
	/// @param name Name of the counter. The object will share ownership of the pointer.
	StatCounter(StatCategory category, const Char* name, StatFlag flags = StatFlag::kNone);

	template<std::integral T>
	U64 increment(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		const U64 orig = m_u;
		m_u += value;
		return orig;
#else
		(void)value;
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 increment(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		const F64 orig = m_f;
		m_f += value;
		return orig;
#else
		(void)value;
		return 0.0;
#endif
	}

	template<std::integral T>
	U64 atomicIncrement(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		return m_atomic.fetchAdd(value);
#else
		(void)value;
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 atomicIncrement(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		LockGuard lock(m_floatLock);
		return increment(value);
#else
		(void)value;
		return 0.0;
#endif
	}

	template<std::integral T>
	U64 decrement(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		const U64 orig = m_u;
		ANKI_ASSERT(orig >= value);
		m_u -= value;
		return orig;
#else
		(void)value;
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 decrement(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		const F64 orig = m_f;
		ANKI_ASSERT(orig >= value);
		m_f -= value;
		return orig;
#else
		(void)value;
		return 0;
#endif
	}

	template<std::integral T>
	U64 atomicDecrement(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		const U64 out = m_atomic.fetchSub(value);
		ANKI_ASSERT(out >= value);
		return out;
#else
		(void)value;
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 atomicDecrement(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		LockGuard lock(m_floatLock);
		return decrement(value);
#else
		(void)value;
		return 0;
#endif
	}

	template<std::integral T>
	U64 set(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		const U64 orig = m_u;
		m_u = value;
		return orig;
#else
		(void)value;
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 set(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		const F64 orig = m_f;
		m_f = value;
		return orig;
#else
		(void)value;
		return 0.0;
#endif
	}

	template<std::integral T>
	U64 atomicSet(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		return m_atomic.exchange(value);
#else
		(void)value;
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 atomicSet(T value)
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		LockGuard lock(m_floatLock);
		return set(value);
#else
		(void)value;
		return 0;
#endif
	}

	template<std::integral T>
	U64 getValue() const
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		return m_u;
#else
		return 0;
#endif
	}

	template<std::floating_point T>
	F64 getValue() const
	{
#if ANKI_STATS_ENABLED
		ANKI_ASSERT(!!(m_flags & StatFlag::kThreadSafe));
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		return m_f;
#else
		return -1.0;
#endif
	}

private:
#if ANKI_STATS_ENABLED
	union
	{
		Atomic<U64> m_atomic = {0};
		U64 m_u;
		F64 m_f;
	};

	const Char* m_name = nullptr;

	mutable SpinLock m_floatLock;

	StatFlag m_flags = StatFlag::kNone;
	StatCategory m_category = StatCategory::kCount;

	template<std::floating_point T>
	F64& get()
	{
		ANKI_ASSERT(!!(m_flags & StatFlag::kFloat));
		return m_f;
	}

	template<std::integral T>
	U64& get()
	{
		ANKI_ASSERT(!(m_flags & StatFlag::kFloat));
		return m_u;
	}
#endif
};

/// A collection of stat counters.
class StatsSet : public MakeSingletonSimple<StatsSet>
{
	friend class StatCounter;

	template<typename>
	friend class MakeSingletonSimple;

public:
	template<typename TFuncUint, typename TFuncFloat>
	void iterateStats(TFuncUint funcUint, TFuncFloat funcFloat)
	{
#if ANKI_STATS_ENABLED
		for(U32 i = 0; i < m_statCounterArrSize; ++i)
		{
			const StatCounter& counter = *m_statCounterArr[i];
			if(!!(counter.m_flags & StatFlag::kFloat))
			{
				F64 value;
				if(!!(counter.m_flags & StatFlag::kThreadSafe))
				{
					LockGuard lock(counter.m_floatLock);
					value = counter.m_f;
				}
				else
				{
					value = counter.m_f;
				}
				funcFloat(counter.m_category, counter.m_name, value, counter.m_flags);
			}
			else
			{
				const U64 value = !!(counter.m_flags & StatFlag::kThreadSafe) ? counter.m_atomic.load() : counter.m_u;
				funcUint(counter.m_category, counter.m_name, value, counter.m_flags);
			}
		}
#else
		(void)funcUint;
		(void)funcFloat;
#endif
	}

	void endFrame()
#if ANKI_STATS_ENABLED
		;
#else
	{
	}
#endif

	U32 getCounterCount() const
	{
#if ANKI_STATS_ENABLED
		return m_statCounterArrSize;
#else
		return 0;
#endif
	}

private:
#if ANKI_STATS_ENABLED
	StatCounter** m_statCounterArr = nullptr;
	U32 m_statCounterArrSize = 0;
	U32 m_statCounterArrStorageSize = 0;
#endif

	StatsSet() = default;

#if ANKI_STATS_ENABLED
	~StatsSet();

	void registerCounter(StatCounter* counter);
#endif
};

inline StatCounter::StatCounter(StatCategory category, const Char* name, StatFlag flags)
#if ANKI_STATS_ENABLED
	: m_name(name)
	, m_flags(flags)
	, m_category(category)
{
	ANKI_ASSERT(name);
	ANKI_ASSERT(m_category < StatCategory::kCount);
	StatsSet::getSingleton().registerCounter(this);
}
#else
{
	(void)category;
	(void)name;
	(void)flags;
}
#endif
/// @}

} // end namespace anki
