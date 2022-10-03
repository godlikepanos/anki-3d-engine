// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/Forward.h>
#include <AnKi/Util/Array.h>

namespace anki {

/// @addtogroup util_containers
/// @{

/// This is almost like a re-implementation of std::function. The difference is that it has some functionality to avoid
/// allocations. Can be used like:
/// @code
/// Function<Error(U32, F32), 16> func;
/// func.init(pool, [&someInt](U32 u, F32 f) {someInt = xxx + u + f; return Error::kNone;});
/// func.call(10, 1.2f);
/// @endcode
/// @tparam kTInlineStorageSize Optional inline storage to avoid deallocations (small object optimization)
template<typename TReturn, typename... TArgs, PtrSize kTInlineStorageSize>
class Function<TReturn(TArgs...), kTInlineStorageSize>
{
public:
	Function() = default;

	/// Move.
	Function(Function&& b)
	{
		*this = std::move(b);
	}

	// Non-copyable.
	Function(const Function&) = delete;

	/// Same as init().
	template<typename TMemPool, typename T>
	Function(TMemPool& pool, const T& func)
	{
		init(pool, func);
	}

	// Does nothing important.
	~Function()
	{
		ANKI_ASSERT(getState() == kStateUninitialized && "Forgot to call destroy()");
	}

	// Non-copyable.
	Function& operator=(const Function&) = delete;

	/// Move.
	Function& operator=(Function&& b)
	{
		ANKI_ASSERT(getState() == kStateUninitialized);
		m_state = b.m_state;
		b.m_state = kStateUninitialized;
		memcpy(&m_callableInlineStorage[0], &b.m_callableInlineStorage[0], sizeof(m_callableInlineStorage));
		return *this;
	}

	/// Initialize the function.
	/// @param pool The memory pool (it might be used).
	/// @param func The lambda.
	template<typename TMemPool, typename T>
	void init(TMemPool& pool, const T& func)
	{
		ANKI_ASSERT(getState() == kStateUninitialized);

		// Init storage
		constexpr Bool useInlineStorage = sizeof(T) <= kInlineStorageSize && std::is_trivially_copyable<T>::value
										  && std::is_trivially_destructible<T>::value;
		if(useInlineStorage)
		{
			setState(kStateInlineStorage);
			memcpy(&m_callableInlineStorage[0], &func, sizeof(func));

			setFunctionCallback([](const Function& self, TArgs... args) -> TReturn {
				// Yes I know, a const_cast hack follows. If the T was in some pointer then all would be fine. Look
				// at the setFunctionCallback() of the kStateAllocated. Only a static_cast there. It's unfair.
				const T* t0 = reinterpret_cast<const T*>(&self.m_callableInlineStorage[0]);
				T* t1 = const_cast<T*>(t0);
				return (*t1)(args...);
			});
		}
		else
		{
			setState(kStateAllocated);
			using CallableT = Callable<T>;
			CallableT* callable = newInstance<CallableT>(pool, func);
			m_callablePtr = callable;

			callable->m_size = sizeof(CallableT);
			callable->m_alignment = alignof(CallableT);

			setFunctionCallback([](const Function& self, TArgs... args) -> TReturn {
				return static_cast<CallableT*>(self.m_callablePtr)->m_func(args...);
			});

			callable->m_destroyCallback = [](CallableBase& c) {
				static_cast<CallableT&>(c).~CallableT();
			};

			callable->m_copyCallback = [](const CallableBase& otherC, CallableBase& c) {
				::new(&static_cast<CallableT&>(c)) CallableT(static_cast<const CallableT&>(otherC));
			};
		}
	}

	/// Destroy the object.
	template<typename TMemPool>
	void destroy(TMemPool& pool)
	{
		if(getState() == kStateAllocated)
		{
			ANKI_ASSERT(m_callablePtr && m_callablePtr->m_destroyCallback);
			m_callablePtr->m_destroyCallback(*m_callablePtr);
			pool.free(m_callablePtr);
		}

		m_state = kStateUninitialized;
	}

	/// Call the Function with some arguments.
	TReturn call(TArgs... args) const
	{
		return getFunctionCallback()(*this, args...);
	}

	/// Same as call().
	TReturn operator()(TArgs... args) const
	{
		return getFunctionCallback()(*this, args...);
	}

	/// Copy from another.
	template<typename TMemPool>
	Function& copy(const Function& other, TMemPool& pool)
	{
		ANKI_ASSERT(getState() == kStateUninitialized && "Need to destroy it first");

		if(other.getState() == kStateUninitialized)
		{
			// Nothing to do
		}
		else if(other.getState() == kStateInlineStorage)
		{
			// It should be trivially copyable, can use memcpy then
			m_state = other.m_state;
			memcpy(&m_callableInlineStorage[0], &other.m_callableInlineStorage[0], sizeof(m_callableInlineStorage));
		}
		else
		{
			ANKI_ASSERT(other.getState() == kStateAllocated);
			m_state = other.m_state;

			// Allocate callable
			ANKI_ASSERT(other.m_callablePtr && other.m_callablePtr->m_alignment > 0 && other.m_callablePtr->m_size > 0);
			m_callablePtr = static_cast<CallableBase*>(
				pool.allocate(other.m_callablePtr->m_size, other.m_callablePtr->m_alignment));

			// Copy
			other.m_callablePtr->m_copyCallback(*other.m_callablePtr, *m_callablePtr);
		}

		return *this;
	}

private:
	class CallableBase;

	using FunctionCallback = TReturn (*)(const Function&, TArgs...);
	using DestroyCallback = void (*)(CallableBase&);
	using CopyCallback = void (*)(const CallableBase&, CallableBase&);

	class CallableBase
	{
	public:
		DestroyCallback m_destroyCallback = nullptr;
		CopyCallback m_copyCallback = nullptr;
		U32 m_size = 0;
		U32 m_alignment = 0;

		CallableBase() = default;

		CallableBase(const CallableBase&) = default;

		CallableBase& operator=(const CallableBase&) = delete; // You won't need it
	};

	template<typename T>
	class Callable : public CallableBase
	{
	public:
		T m_func;

		Callable(const T& t)
			: m_func(t)
		{
		}

		Callable(const Callable& b)
			: CallableBase(b)
			, m_func(b.m_func)
		{
		}

		Callable& operator=(const Callable&) = delete; // You won't need it
	};

	static constexpr PtrSize kStateUninitialized = PtrSize(0b1001) << PtrSize(60);
	static constexpr PtrSize kStateAllocated = PtrSize(0b1101) << PtrSize(60);
	static constexpr PtrSize kStateInlineStorage = PtrSize(0b1011) << PtrSize(60);
	static constexpr PtrSize kStateAllBits = PtrSize(0b1111) << PtrSize(60);
	static_assert(sizeof(void*) == 8, "Wrong assumption");

	static constexpr PtrSize lmax(PtrSize a, PtrSize b)
	{
		return (a > b) ? a : b;
	}

	static constexpr PtrSize kInlineStorageSize = lmax(kTInlineStorageSize, lmax(ANKI_SAFE_ALIGNMENT, sizeof(void*)));

	union
	{
		CallableBase* m_callablePtr;
		alignas(ANKI_SAFE_ALIGNMENT) Array<U8, kInlineStorageSize> m_callableInlineStorage;
	};

	union
	{
		// Hide the state in the high bits of the m_functionCallback pointer.
		PtrSize m_state = kStateUninitialized;

		FunctionCallback m_functionCallback;
	};

	PtrSize getState() const
	{
		const PtrSize s = m_state & kStateAllBits;
		ANKI_ASSERT(s == kStateUninitialized || s == kStateAllocated || s == kStateInlineStorage);
		return s;
	}

	void setState(PtrSize s)
	{
		ANKI_ASSERT(s == kStateUninitialized || s == kStateAllocated || s == kStateInlineStorage);
		m_state = (m_state & ~kStateAllBits) | s;
	}

	FunctionCallback getFunctionCallback() const
	{
		return numberToPtr<FunctionCallback>(m_state & ~kStateAllBits);
	}

	void setFunctionCallback(FunctionCallback f)
	{
		m_state = (m_state & kStateAllBits) | ptrToNumber(f);
		ANKI_ASSERT(f == getFunctionCallback());
	}
};
/// @}

} // end namespace anki
