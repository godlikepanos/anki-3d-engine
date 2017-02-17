// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

	Atomic(const Value& a)
		: m_val(a)
	{
	}

	/// Set the value without protection.
	void set(const Value& a)
	{
		m_val = a;
	}

	/// Get the value without protection.
	const Value& get() const
	{
		return m_val;
	}

	/// Get the value of the atomic.
	Value load(AtomicMemoryOrder memOrd = MEMORY_ORDER) const
	{
		return m_att.load(static_cast<std::memory_order>(memOrd));
	}

	/// Store
	void store(const Value& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		m_att.store(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and add.
	template<typename Y>
	Value fetchAdd(const Y& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.fetch_add(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and subtract.
	template<typename Y>
	Value fetchSub(const Y& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.fetch_sub(a, static_cast<std::memory_order>(memOrd));
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
	Bool compareExchange(Value& expected, const Value& desired, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.compare_exchange_weak(
			expected, desired, static_cast<std::memory_order>(memOrd), static_cast<std::memory_order>(memOrd));
	}

	/// Set @a a to the atomic and return the previous value.
	Value exchange(const Value& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
		return m_att.exchange(a, static_cast<std::memory_order>(memOrd));
	}

	/// Store the minimum using compare-and-swap.
	void min(const Value& a)
	{
		Value prev = load();
		while(a < prev && !compareExchange(prev, a))
		{
		}
	}

	/// Store the maximum using compare-and-swap.
	void max(const Value& a)
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
