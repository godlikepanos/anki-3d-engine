// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_ATOMIC_H
#define ANKI_UTIL_ATOMIC_H

#include "anki/util/StdTypes.h"
#include "anki/util/NonCopyable.h"

namespace anki {

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
#	error "TODO"
#endif
};

/// Atomic template.
template<typename T, AtomicMemoryOrder tmemOrd = AtomicMemoryOrder::RELAXED>
class Atomic: public NonCopyable
{
public:
	using Value = T;
	static constexpr AtomicMemoryOrder MEMORY_ORDER = tmemOrd;

	Atomic()
	:	m_val(static_cast<Value>(0))
	{}

	Atomic(const Value a)
	:	m_val(a)
	{}

	Value load(AtomicMemoryOrder memOrd = MEMORY_ORDER) const
	{
#if defined(__GNUC__)
		return __atomic_load_n(&m_val, static_cast<int>(memOrd));
#else
#	error "TODO"
#endif
	}

	void store(const Value a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		__atomic_store_n(&m_val, a, static_cast<int>(memOrd));
#else
#	error "TODO"
#endif
	}

	template<typename Y>
	Value fetchAdd(const Y a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_fetch_add(&m_val, a, static_cast<int>(memOrd));
#else
#	error "TODO"
#endif
	}

	template<typename Y>
	Value fetchSub(const Y a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_fetch_sub(&m_val, a, static_cast<int>(memOrd));
#else
#	error "TODO"
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
	Bool compareExchange(Value& expected, const Value desired, 
		AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_compare_exchange_n(&m_val, &expected, desired,
			false, static_cast<int>(memOrd), __ATOMIC_RELAXED);
#else
#	error "TODO"
#endif
	}

	/// Set @a a to the atomic and return the previous value.
	Value exchange(const Value a, AtomicMemoryOrder memOrd = MEMORY_ORDER)
	{
#if defined(__GNUC__)
		return __atomic_exchange_n(&m_val, a, static_cast<int>(memOrd));
#else
#	error "TODO"
#endif
	}

private:
	Value m_val;
};
/// @}

} // end namespace anki

#endif

