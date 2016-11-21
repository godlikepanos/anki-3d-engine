// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/NonCopyable.h>
#include <type_traits>

namespace anki
{

/// @addtogroup util_other
/// @{

enum class AtomicMemoryOrder
{
#if defined(__GNUC__)
	RELAXED = __ATOMIC_RELAXED,
	CONSUME = __ATOMIC_CONSUME,
	ACQUIRE = __ATOMIC_ACQUIRE,
	RELEASE = __ATOMIC_RELEASE,
	ACQ_REL = __ATOMIC_ACQ_REL,
	SEQ_CST = __ATOMIC_SEQ_CST
#else
#error "TODO"
#endif
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
		static_assert(std::is_pointer<T>::value || std::is_arithmetic<T>::value, "Check the type");
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
#if defined(__GNUC__)
		return __atomic_load_n(&m_val, static_cast<int>(memOrd));
#else
#error "TODO"
#endif
	}

	/// Store
	void store(const Value& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		__atomic_store_n(&m_val, a, static_cast<int>(memOrd));
#else
#error "TODO"
#endif
	}

	/// Fetch and add. Pointer specialization
	template<typename Y, typename Q = T>
	typename std::enable_if<std::is_pointer<Q>::value, Q>::type fetchAdd(
		const Y& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_fetch_add(&m_val, a * sizeof(typename std::remove_pointer<Q>::type), static_cast<int>(memOrd));
#else
#error "TODO"
#endif
	}

	/// Fetch and add. Arithmetic specialization.
	template<typename Y, typename Q = T>
	typename std::enable_if<!std::is_pointer<Q>::value, Q>::type fetchAdd(
		const Y& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_fetch_add(&m_val, a, static_cast<int>(memOrd));
#else
#error "TODO"
#endif
	}

	/// Fetch and subtract. Pointer specialization.
	template<typename Y, typename Q = T>
	typename std::enable_if<std::is_pointer<Q>::value, Q>::type fetchSub(
		const Y& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_fetch_sub(&m_val, a * sizeof(typename std::remove_pointer<Q>::type), static_cast<int>(memOrd));
#else
#error "TODO"
#endif
	}

	/// Fetch and subtract. Arithmetic specialization.
	template<typename Y, typename Q = T>
	typename std::enable_if<!std::is_pointer<Q>::value, Q>::type fetchSub(
		const Y& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_fetch_sub(&m_val, a, static_cast<int>(memOrd));
#else
#error "TODO"
#endif
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
#if defined(__GNUC__)
		return __atomic_compare_exchange_n(
			&m_val, &expected, desired, false, static_cast<int>(memOrd), __ATOMIC_RELAXED);
#else
#error "TODO"
#endif
	}

	/// Set @a a to the atomic and return the previous value.
	Value exchange(const Value& a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_exchange_n(&m_val, a, static_cast<int>(memOrd));
#else
#error "TODO"
#endif
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
	Value m_val;
};
/// @}

} // end namespace anki
