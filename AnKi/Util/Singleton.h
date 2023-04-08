// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Assert.h>
#include <utility>

namespace anki {

#if ANKI_ENABLE_ASSERTIONS
extern I32 g_singletonsAllocated;
#endif

/// @addtogroup util_patterns
/// @{

/// If class inherits that it will become a singleton.
template<typename T>
class MakeSingleton
{
public:
	ANKI_FORCE_INLINE static T& getSingleton()
	{
		ANKI_ASSERT(m_initialized);
		return *reinterpret_cast<T*>(m_global);
	}

	template<typename... TArgs>
	static T& allocateSingleton(TArgs... args)
	{
		ANKI_ASSERT(!m_initialized);
		::new(m_global) T(args...);
		m_initialized = true;
#if ANKI_ENABLE_ASSERTIONS
		++g_singletonsAllocated;
#endif

		return *reinterpret_cast<T*>(m_global);
	}

	static void freeSingleton()
	{
		if(m_initialized)
		{
			reinterpret_cast<T*>(m_global)->~T();
			m_initialized = false;
#if ANKI_ENABLE_ASSERTIONS
			--g_singletonsAllocated;
#endif
		}
	}

	static Bool isAllocated()
	{
		return m_initialized;
	}

private:
	static U8 m_global[];
	static inline Bool m_initialized = false;
};

template<typename T>
alignas(ANKI_SAFE_ALIGNMENT) U8 MakeSingleton<T>::m_global[sizeof(T)];

/// If class inherits that it will become a singleton.
template<typename T>
class MakeSingletonPtr
{
public:
	ANKI_FORCE_INLINE static T& getSingleton()
	{
		return *m_global;
	}

	template<typename... TArgs>
	static T& allocateSingleton(TArgs... args)
	{
		ANKI_ASSERT(m_global == nullptr);
		m_global = new T(args...);

#if ANKI_ENABLE_ASSERTIONS
		++g_singletonsAllocated;
#endif

		return *m_global;
	}

	static void freeSingleton()
	{
		if(m_global)
		{
			delete m_global;
			m_global = nullptr;
#if ANKI_ENABLE_ASSERTIONS
			--g_singletonsAllocated;
#endif
		}
	}

	static Bool isAllocated()
	{
		return m_global != nullptr;
	}

private:
	static inline T* m_global = nullptr;
};
/// @}

} // end namespace anki
