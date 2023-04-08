// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
/// Function<Error(U32, F32), 16> func([&someInt](U32 u, F32 f) {someInt = xxx + u + f; return Error::kNone;});
/// func.call(10, 1.2f);
/// @endcode
/// @tparam kTInlineStorageSize Optional inline storage to avoid deallocations (small object optimization)
template<typename TReturn, typename... TArgs, typename TMemoryPool, PtrSize kTInlineStorageSize>
class Function<TReturn(TArgs...), TMemoryPool, kTInlineStorageSize>
{
public:
	Function() = default;

	/// Initialize.
	template<typename TFunc>
	Function(TFunc func, const TMemoryPool& pool = TMemoryPool())
		: m_pool(pool)
	{
		// Init storage
		constexpr Bool useInlineStorage = sizeof(TFunc) <= kInlineStorageSize
										  && std::is_trivially_copyable<TFunc>::value
										  && std::is_trivially_destructible<TFunc>::value;
		if(useInlineStorage)
		{
			setState(kStateInlineStorage);
			memcpy(&m_callableInlineStorage[0], &func, sizeof(func));

			setFunctionCallback([](const Function& self, TArgs... args) -> TReturn {
				// Yes I know, a const_cast hack follows. If the TFunc was in some pointer then all would be fine. Look
				// at the setFunctionCallback() of the kStateAllocated. Only a static_cast there. It's unfair.
				const TFunc* t0 = reinterpret_cast<const TFunc*>(&self.m_callableInlineStorage[0]);
				TFunc* t1 = const_cast<TFunc*>(t0);
				return (*t1)(args...);
			});
		}
		else
		{
			setState(kStateAllocated);
			using CallableT = Callable<TFunc>;
			CallableT* callable = newInstance<CallableT>(m_pool, func);
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

	/// Move.
	Function(Function&& b)
	{
		*this = std::move(b);
	}

	/// Copy.
	Function(const Function& b)
	{
		*this = b;
	}

	~Function()
	{
		destroy();
	}

	/// Copy.
	Function& operator=(const Function& b)
	{
		destroy();
		m_pool = b.m_pool;

		if(b.getState() == kStateUninitialized)
		{
			// Nothing to do
		}
		else if(b.getState() == kStateInlineStorage)
		{
			// It should be trivially copyable, can use memcpy then
			m_state = b.m_state;
			memcpy(&m_callableInlineStorage[0], &b.m_callableInlineStorage[0], sizeof(m_callableInlineStorage));
		}
		else
		{
			ANKI_ASSERT(b.getState() == kStateAllocated);
			m_state = b.m_state;

			// Allocate callable
			ANKI_ASSERT(b.m_callablePtr && b.m_callablePtr->m_alignment > 0 && b.m_callablePtr->m_size > 0);
			m_callablePtr =
				static_cast<CallableBase*>(m_pool.allocate(b.m_callablePtr->m_size, b.m_callablePtr->m_alignment));

			// Copy
			b.m_callablePtr->m_copyCallback(*b.m_callablePtr, *m_callablePtr);
		}

		return *this;
	}

	/// Move.
	Function& operator=(Function&& b)
	{
		destroy();
		m_pool = b.m_pool;
		m_state = b.m_state;
		b.m_state = kStateUninitialized;
		memcpy(&m_callableInlineStorage[0], &b.m_callableInlineStorage[0], sizeof(m_callableInlineStorage));
		return *this;
	}

	/// Destroy the object.
	void destroy()
	{
		if(getState() == kStateAllocated)
		{
			ANKI_ASSERT(m_callablePtr && m_callablePtr->m_destroyCallback);
			m_callablePtr->m_destroyCallback(*m_callablePtr);
			m_pool.free(m_callablePtr);
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

	TMemoryPool m_pool;

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
