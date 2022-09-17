// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <atomic>

namespace anki {

/// @addtogroup util_other
/// @{

enum class AtomicMemoryOrder
{
	kRelaxed = std::memory_order_relaxed,
	kConsume = std::memory_order_consume,
	kAcquire = std::memory_order_acquire,
	kRelease = std::memory_order_release,
	kAcqRel = std::memory_order_acq_rel,
	kSeqCst = std::memory_order_seq_cst
};

/// Atomic template. At the moment it doesn't work well with pointers.
template<typename T, AtomicMemoryOrder kMemOrder = AtomicMemoryOrder::kRelaxed>
class Atomic
{
public:
	using Value = T;
	static constexpr AtomicMemoryOrder kDefaultMemoryOrder = kMemOrder;

	/// It will not set itself to zero.
	Atomic()
	{
	}

	Atomic(Value a)
		: m_val(a)
	{
	}

	Atomic(const Atomic&) = delete; // Non-copyable

	Atomic& operator=(const Atomic&) = delete; // Non-copyable

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
	Value load(AtomicMemoryOrder memOrd = kDefaultMemoryOrder) const
	{
		return m_att.load(static_cast<std::memory_order>(memOrd));
	}

	/// Store
	void store(Value a, AtomicMemoryOrder memOrd = kDefaultMemoryOrder)
	{
		m_att.store(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and add.
	template<typename Y>
	Value fetchAdd(Y a, AtomicMemoryOrder memOrd = kDefaultMemoryOrder)
	{
		return m_att.fetch_add(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and subtract.
	template<typename Y>
	Value fetchSub(Y a, AtomicMemoryOrder memOrd = kDefaultMemoryOrder)
	{
		return m_att.fetch_sub(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and do bitwise or.
	template<typename Y>
	Value fetchOr(Y a, AtomicMemoryOrder memOrd = kDefaultMemoryOrder)
	{
		return m_att.fetch_or(a, static_cast<std::memory_order>(memOrd));
	}

	/// Fetch and do bitwise and.
	template<typename Y>
	Value fetchAnd(Y a, AtomicMemoryOrder memOrd = kDefaultMemoryOrder)
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
	Bool compareExchange(Value& expected, Value desired, AtomicMemoryOrder successMemOrd = kDefaultMemoryOrder,
						 AtomicMemoryOrder failMemOrd = kDefaultMemoryOrder)
	{
		return m_att.compare_exchange_weak(expected, desired, static_cast<std::memory_order>(successMemOrd),
										   static_cast<std::memory_order>(failMemOrd));
	}

	/// Set @a a to the atomic and return the previous value.
	Value exchange(Value a, AtomicMemoryOrder memOrd = kDefaultMemoryOrder)
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
