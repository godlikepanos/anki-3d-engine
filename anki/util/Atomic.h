// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/NonCopyable.h>
#include <atomic>

namespace anki
{

/// @addtogroup util_other
/// @{

enum class AtomicMemoryOrder
{
	RELAXED = std::memory_order_relaxed,
	CONSUME = std::memory_order_consume,
	ACQUIRE = std::memory_order_acquire,
	RELEASE = std::memory_order_release,
	ACQ_REL = std::memory_order_acq_rel,
	SEQ_CST = std::memory_order_seq_cst
};

/// Atomic template. At the moment it doesn't work well with pointers.
template<typename T, AtomicMemoryOrder tmemOrd = AtomicMemoryOrder::RELAXED>
class Atomic : public NonCopyable
{
public:
	using Value = T;
	static constexpr AtomicMemoryOrder MEMORY_ORDER = tmemOrd;

	/// It will not set itself to zero.
	Atomic()
	{
	}

	Atomic(Value a)
		: m_val(a)
	{
	}

	/// Set the value without protection.
	void setNonAtomically(Value a)
	{
		m_val = a;
	}

	/// Get the value without protection.
	Value getNonAtomically() const
	{
		return m_val;
	}

	/// Get the value of the atomic.
	Value load(AtomicMemoryOrder memOrd = MEMORY_ORDER) const
	{
		return m_att.load(static_cast<std::memory_order>(memOrd));
	}

	/// Store
	void store(Value a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		m_att.store(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and add.
	template<typename Y>
	Value fetchAdd(Y a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.fetch_add(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and subtract.
	template<typename Y>
	Value fetchSub(Y a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.fetch_sub(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and do bitwise or.
	template<typename Y>
	Value fetchOr(Y a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.fetch_or(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and do bitwise and.
	template<typename Y>
	Value fetchAnd(Y a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.fetch_and(a, static_cast<std::memory_order>(memOrd));
	}

	/// @code
	/// if(m_val == expected) {
	/// 	m_val = desired;
	/// 	return true;
	/// } else {
	/// 	expected = m_val;
	/// 	return false;
	/// }
	/// @endcode
	Bool compareExchange(Value& expected, Value desired, AtomicMemoryOrder successMemOrd = MEMORY_ORDER,
						 AtomicMemoryOrder failMemOrd = MEMORY_ORDER)
	{
		return m_att.compare_exchange_weak(expected, desired, static_cast<std::memory_order>(successMemOrd),
										   static_cast<std::memory_order>(failMemOrd));
	}

	/// Set @a a to the atomic and return the previous value.
	Value exchange(Value a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.exchange(a, static_cast<std::memory_order>(memOrd));
	}

	/// Store the minimum using compare-and-swap.
	void min(Value a)
	{
		Value prev = load();
		while(a < prev && !compareExchange(prev, a))
		{
		}
	}

	/// Store the maximum using compare-and-swap.
	void max(Value a)
	{
		Value prev = load();
		while(a > prev && !compareExchange(prev, a))
		{
		}
	}

private:
	union
	{
		Value m_val;
		std::atomic<Value> m_att;
	};
};
/// @}

} // end namespace anki
