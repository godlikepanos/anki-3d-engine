// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Assert.h>
#include <utility>

namespace anki
{

/// @addtogroup util_patterns
/// @{

/// This template makes a class singleton
template<typename T>
class Singleton
{
public:
	typedef T Value;

	// Non copyable
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	// Non constructable
	Singleton() = delete;
	~Singleton() = delete;

	/// Get instance
	static Value& get()
	{
		return *(m_instance ? m_instance : (m_instance = new Value));
	}

	/// Cleanup
	static void destroy()
	{
		if(m_instance)
		{
			delete m_instance;
		}
	}

private:
	static Value* m_instance;
};

template<typename T>
typename Singleton<T>::Value* Singleton<T>::m_instance = nullptr;

/// This template makes a class with a destructor with arguments singleton
template<typename T>
class SingletonInit
{
public:
	typedef T Value;

	// Non copyable
	SingletonInit(const SingletonInit&) = delete;
	SingletonInit& operator=(const SingletonInit&) = delete;

	// Non constructable
	SingletonInit() = delete;
	~SingletonInit() = delete;

	/// Init the singleton
	template<typename... TArgs>
	static void init(TArgs... args)
	{
		ANKI_ASSERT(m_instance == nullptr);
		m_instance = new Value(args...);
	}

	/// Get instance
	static Value& get()
	{
		ANKI_ASSERT(m_instance != nullptr);
		return *m_instance;
	}

	/// Cleanup
	static void destroy()
	{
		if(m_instance)
		{
			delete m_instance;
		}
	}

private:
	static Value* m_instance;
};

template<typename T>
typename SingletonInit<T>::Value* SingletonInit<T>::m_instance = nullptr;

/// This template makes a class singleton with thread local instance
template<typename T>
class SingletonThreadSafe
{
public:
	typedef T Value;

	// Non copyable
	SingletonThreadSafe(const SingletonThreadSafe&) = delete;
	SingletonThreadSafe& operator=(const SingletonThreadSafe&) = delete;

	// Non constructable
	SingletonThreadSafe() = delete;
	~SingletonThreadSafe() = delete;

	/// Get instance
	static Value& get()
	{
		return *(m_instance ? m_instance : (m_instance = new Value));
	}

	/// Cleanup
	void destroy()
	{
		if(m_instance)
		{
			delete m_instance;
		}
	}

private:
	static thread_local Value* m_instance;
};

template<typename T>
thread_local typename SingletonThreadSafe<T>::Value* SingletonThreadSafe<T>::m_instance = nullptr;
/// @}

} // end namespace anki
