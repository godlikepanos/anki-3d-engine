#ifndef ANKI_UTIL_SINGLETON_H
#define ANKI_UTIL_SINGLETON_H

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup patterns
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
		return *(instance ? instance : (instance = new Value));
	}

private:
	static Value* instance;
};

template <typename T>
typename Singleton<T>::Value* Singleton<T>::instance = nullptr;

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
		return *(instance ? instance : (instance = new Value));
	}

private:
	static thread_local Value* instance;
};

template <typename T>
thread_local typename SingletonThreadSafe<T>::Value* 
	SingletonThreadSafe<T>::instance = nullptr;

/// @}
/// @}

} // end namespace anki

#endif
