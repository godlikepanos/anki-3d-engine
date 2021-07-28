// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/Forward.h>
#include <AnKi/Util/Array.h>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// This is almost like a re-implementation of std::function. The difference is that it has some functionality to avoid
/// allocations. Can be used like:
/// @code
/// Function<Error(U32, F32), 16> func;
/// func.init(allocator, [&someInt](U32 u, F32 f) {someInt = xxx + u + f; return Error::NONE;});
/// func.call(10, 1.2f);
/// @endcode
/// @tparam T_INLINE_STORAGE_SIZE Optional inline storage to avoid deallocations (small object optimization)
template<typename TReturn, typename... TArgs, PtrSize T_INLINE_STORAGE_SIZE>
class Function<TReturn(TArgs...), T_INLINE_STORAGE_SIZE>
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
	template<typename TAlloc, typename T>
	Function(TAlloc alloc, const T& func)
	{
		init(alloc, func);
	}

	// Does nothing important.
	~Function()
	{
		ANKI_ASSERT(getState() == STATE_UNINITIALIZED && "Forgot to call destroy()");
	}

	// Non-copyable.
	Function& operator=(const Function&) = delete;

	/// Move.
	Function& operator=(Function&& b)
	{
		ANKI_ASSERT(getState() == STATE_UNINITIALIZED);
		m_state = b.m_state;
		b.m_state = STATE_UNINITIALIZED;
		memcpy(&m_callableInlineStorage[0], &b.m_callableInlineStorage[0], sizeof(m_callableInlineStorage));
		m_destroyCallback = b.m_destroyCallback;
		b.m_destroyCallback = nullptr;
		return *this;
	}

	/// Initialize the function.
	/// @param alloc The allocator (it might be used).
	/// @param func The lambda.
	template<typename TAlloc, typename T>
	void init(TAlloc alloc, const T& func)
	{
		ANKI_ASSERT(getState() == STATE_UNINITIALIZED);

		// Init storage
		const Bool useInlineStorage = sizeof(T) <= INLINE_STORAGE_SIZE && std::is_trivially_copyable<T>::value;
		if(useInlineStorage)
		{
			setState(STATE_INLINE_STORAGE);
			memcpy(&m_callableInlineStorage[0], &func, sizeof(func));

			setFunctionCallback([](Function& self, TArgs... args) -> TReturn {
				return (*reinterpret_cast<T*>(&self.m_callableInlineStorage[0]))(args...);
			});

			if(!std::is_trivially_destructible<T>::value)
			{
				m_destroyCallback = [](Function& self) {
					reinterpret_cast<T*>(&self.m_callableInlineStorage[0])->~T();
				};
			}
			else
			{
				m_destroyCallback = nullptr;
			}
		}
		else
		{
			setState(STATE_ALLOCATED);
			m_callablePtr = alloc.template newInstance<T>(func);

			setFunctionCallback([](Function& self, TArgs... args) -> TReturn {
				return (*static_cast<T*>(self.m_callablePtr))(args...);
			});

			if(!std::is_trivially_destructible<T>::value)
			{
				m_destroyCallback = [](Function& self) { static_cast<T*>(self.m_callablePtr)->~T(); };
			}
			else
			{
				m_destroyCallback = nullptr;
			}
		}
	}

	/// Destroy the object.
	template<typename TAlloc>
	void destroy(TAlloc alloc)
	{
		if(m_destroyCallback)
		{
			m_destroyCallback(*this);
		}

		if(getState() == STATE_ALLOCATED)
		{
			alloc.deallocate(m_callablePtr, 1);
		}

		m_state = STATE_UNINITIALIZED;
		m_destroyCallback = nullptr;
	}

	/// Call the Function with some arguments.
	TReturn call(TArgs... args)
	{
		return getFunctionCallback()(*this, args...);
	}

	/// Same as call().
	TReturn operator()(TArgs... args)
	{
		return getFunctionCallback()(*this, args...);
	}

private:
	using FunctionCallback = TReturn (*)(Function&, TArgs...);
	using DestroyCallback = void (*)(Function&);

	static constexpr PtrSize STATE_UNINITIALIZED = PtrSize(0b1001) << PtrSize(60);
	static constexpr PtrSize STATE_ALLOCATED = PtrSize(0b1101) << PtrSize(60);
	static constexpr PtrSize STATE_INLINE_STORAGE = PtrSize(0b1011) << PtrSize(60);
	static constexpr PtrSize STATE_ALL_BITS = PtrSize(0b1111) << PtrSize(60);
	static_assert(sizeof(void*) == 8, "Wrong assumption");

	static constexpr PtrSize lmax(PtrSize a, PtrSize b)
	{
		return (a > b) ? a : b;
	}

	static constexpr PtrSize INLINE_STORAGE_SIZE =
		lmax(T_INLINE_STORAGE_SIZE, lmax(ANKI_SAFE_ALIGNMENT, sizeof(void*)));

	union
	{
		void* m_callablePtr;
		alignas(ANKI_SAFE_ALIGNMENT) Array<U8, INLINE_STORAGE_SIZE> m_callableInlineStorage;
	};

	union
	{
		// Hide the state in the high bits of the m_functionCallback pointer.
		PtrSize m_state = STATE_UNINITIALIZED;

		FunctionCallback m_functionCallback;
	};

	DestroyCallback m_destroyCallback = nullptr;

	PtrSize getState() const
	{
		const PtrSize s = m_state & STATE_ALL_BITS;
		ANKI_ASSERT(s == STATE_UNINITIALIZED || s == STATE_ALLOCATED || s == STATE_INLINE_STORAGE);
		return s;
	}

	void setState(PtrSize s)
	{
		ANKI_ASSERT(s == STATE_UNINITIALIZED || s == STATE_ALLOCATED || s == STATE_INLINE_STORAGE);
		m_state = (m_state & ~STATE_ALL_BITS) | s;
	}

	FunctionCallback getFunctionCallback() const
	{
		return numberToPtr<FunctionCallback>(m_state & ~STATE_ALL_BITS);
	}

	void setFunctionCallback(FunctionCallback f)
	{
		m_state = (m_state & STATE_ALL_BITS) | ptrToNumber(f);
		ANKI_ASSERT(f == getFunctionCallback());
	}
};
/// @}

} // end namespace anki
