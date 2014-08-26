// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_HANDLE_H
#define ANKI_GL_GL_HANDLE_H

#include "anki/gl/GlCommon.h"

namespace anki {

// Forward
class GlDevice;

/// @addtogroup opengl_private
/// @{

/// State of the handle
enum class GlHandleState: U8
{
	NEW,
	TO_BE_CREATED,
	CREATED,
	TO_BE_DELETED,
	DELETED
};

/// Default deleter for the GlHandle
template<typename T, typename TAlloc>
class GlHandleDefaultDeleter
{
public:
	void operator()(T* ptr, TAlloc alloc, GlDevice*)
	{
		alloc.deleteInstance(ptr);
	}
};

/// It's essentialy a custom shared pointer. Created to have full control over 
/// allocations, it takes less space than a regular shared pointer (1 pointer 
/// instead of 2) and it can have custom control blocks
///
/// @tparam T The type of the pointer
template<typename T>
class GlHandle
{
public:
	using Type = T;

	/// @name Constructor/Destructor
	/// @{
	GlHandle()
	:	m_cb(nullptr)
	{}

	/// Create an object and initialize the handle with that
	/// @param[in,out] manager If it's not nullptr then it will create a more 
	///                        sophisticated handle. Pass nullptr otherwise
	/// @param[in] alloc The allocator that will be used for all internal 
	///                  allocations of the handle
	/// @param[in] del The deleter that will be used to cleanup the allocated
	///                object. The containers for example require deferred 
	///                deleters
	/// @param args The arguments to pass to the object's constructor
	template<typename TAlloc, typename TDeleter, typename... TArgs>
	GlHandle(GlDevice* manager, TAlloc alloc, TDeleter del, TArgs&&... args)
	{
		// Create the object
		T* ptr = alloc.template newInstance<T>(std::forward<TArgs>(args)...);

		if(manager)
		{
			// Container block

			using Block = CtrlBlockContainer<T, TAlloc, TDeleter>;

			Block* blk = alloc.template newInstance<Block>();

			blk->m_state = GlHandleState::NEW;
			blk->m_manager = manager;
			blk->m_alloc = alloc;
			blk->m_del = del;

			m_cb = blk;
		}
		else
		{
			// Simple block

			using Block = CtrlBlockSimple<T, TAlloc, TDeleter>;

			Block* blk = alloc.template newInstance<Block>();

			blk->m_alloc = alloc;
			blk->m_del = del;

			m_cb = blk;
		}

		// Set common
		m_cb->m_refcount = 1;
		m_cb->m_ptr = ptr;
	}

	GlHandle(const GlHandle& b)
	:	GlHandle()
	{
		*this = b;
	}

	GlHandle(GlHandle&& b)
	:	GlHandle()
	{
		*this = std::move(b);
	}

	~GlHandle()
	{
		reset();
	}
	/// @}

	/// Move
	GlHandle& operator=(GlHandle&& b)
	{
		reset();
	
		if(b.m_cb)
		{
			m_cb = b.m_cb;
			b.m_cb = nullptr;
		}

		return *this;
	}

	/// Copy
	GlHandle& operator=(const GlHandle& b)
	{
		reset();
	
		if(b.m_cb)
		{
			auto count = b.m_cb->m_refcount.fetch_add(1);
			ANKI_ASSERT(count > 0);
			(void)count;

			m_cb = b.m_cb;
		}

		return *this;
	}

	/// Compare
	Bool operator==(const GlHandle& b) const
	{
		return m_cb == b.m_cb;
	}

	/// Compare
	Bool operator!=(const GlHandle& b) const
	{
		return m_cb != b.m_cb;
	}

	/// Compare
	Bool operator<(const GlHandle& b) const
	{
		return m_cb < b.m_cb;
	}

	/// Compare
	Bool operator<=(const GlHandle& b) const
	{
		return m_cb <= b.m_cb;
	}

	/// Compare
	Bool operator>(const GlHandle& b) const
	{
		return m_cb > b.m_cb;
	}

	/// Compare
	Bool operator>=(const GlHandle& b) const
	{
		return m_cb >= b.m_cb;
	}

	/// Return true if it's pointing to actual data
	Bool isCreated() const
	{
		return m_cb != nullptr;
	}

	/// Return the number of users
	U getReferenceCount() const
	{
		ANKI_ASSERT(m_cb != nullptr && m_cb->m_ptr != nullptr);
		return m_cb->m_refcount.load();
	}

	/// Get the mutable pointer 
	T& _get()
	{
		ANKI_ASSERT(m_cb != nullptr && m_cb->m_ptr != nullptr);
		return *m_cb->m_ptr;
	}

	/// Get the immutable pointer
	const T& _get() const
	{
		ANKI_ASSERT(m_cb != nullptr && m_cb->m_ptr != nullptr);
		return *m_cb->m_ptr;
	}

	/// Get the current state
	GlHandleState _getState() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->getStateAtomically(nullptr);
	}

	/// Set the handle state
	GlHandleState _setState(GlHandleState newVal)
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->getStateAtomically(&newVal);
	}

	GlDevice& _getManager() const
	{
		ANKI_ASSERT(m_cb != nullptr && m_cb->getManager() != nullptr);
		return *m_cb->getManager(); 
	}

private:
	/// Control block base
	/// @note The non-pure virtuals indicate an optional feature
	template<typename Y>
	class CtrlBlockBase
	{
	public:
		Y* m_ptr;
		std::atomic<I32> m_refcount;

		virtual ~CtrlBlockBase()
		{}
		
		/// Destroy the object in m_ptr
		virtual void deletePtr() = 0;

		/// Delete the block itself
		virtual void deleteSelf() = 0;

		/// The container handles want the manager
		virtual GlDevice* getManager() const
		{
			return nullptr;
		}

		/// Manipulate the state. It will return the value of the state and if
		/// newVal is not nullptr it will set a new value
		virtual GlHandleState getStateAtomically(GlHandleState* newVal)
		{
			(void)newVal;
			return GlHandleState::NEW;
		}
	};

	/// Control block for simple objects
	template<typename Y, typename YAlloc, typename YDeleter>
	class CtrlBlockSimple: public CtrlBlockBase<Y>
	{
	public:	
		using Base = CtrlBlockBase<Y>;

		/// Copy of the allocator
		YAlloc m_alloc; 

		/// The deleter functor
		YDeleter m_del;
		
		void deletePtr()
		{
			// Delete object
			m_del(Base::m_ptr, m_alloc, nullptr);
		}

		void deleteSelf()
		{
			// Delete control block
			m_alloc.deleteInstance(this);
		}
	};

	/// Control block for container objects
	template<typename Y, typename YAlloc, typename YDeleter>
	class CtrlBlockContainer: public CtrlBlockBase<Y>
	{
	public:	
		using Base = CtrlBlockBase<Y>;

		/// Copy of the allocator
		YAlloc m_alloc; 

		/// The deleter functor
		YDeleter m_del;

		/// @note Its mutable because we want read/write access to it
		mutable GlDevice* m_manager; 

		std::atomic<GlHandleState> m_state;
		
		void deletePtr()
		{
			// Delete object
			m_del(Base::m_ptr, m_alloc, getManager());
		}

		void deleteSelf()
		{
			// Delete control block
			m_alloc.deleteInstance(this);
		}

		GlDevice* getManager() const override
		{
			return m_manager;
		}

		GlHandleState getStateAtomically(GlHandleState* newVal) override
		{
			GlHandleState crntVal;
			if(newVal)
			{
				crntVal = m_state.exchange(*newVal);
			}
			else
			{
				crntVal = m_state.load();
			}
			return crntVal;
		}
	};

	CtrlBlockBase<T>* m_cb;

	/// Reset the pointer to it's original state and remove the references to
	/// any object
	void reset()
	{
		if(m_cb)
		{
			auto count = m_cb->m_refcount.fetch_sub(1);
			if(count == 1)
			{
				m_cb->deletePtr();
				m_cb->deleteSelf();
			}

			m_cb = nullptr;
		}
	}
};

/// @}

} // end namespace anki

#endif

