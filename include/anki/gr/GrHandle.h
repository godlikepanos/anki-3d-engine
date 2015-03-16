// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GR_HANDLE_H
#define ANKI_GR_GR_HANDLE_H

#include "anki/gr/GrObject.h"
#include "anki/gr/GrManager.h"

namespace anki {

/// @addtogroup graphics_private
/// @{

/// Default deleter for the GrHandle
template<typename T>
class GrHandleDefaultDeleter
{
public:
	void operator()(T* ptr)
	{
		ptr->getAllocator().deleteInstance(ptr);
	}
};

/// It's essentialy a custom shared pointer. Created to have full control over 
/// allocations, it takes less space than a regular shared pointer (1 pointer 
/// instead of 2) and it can have custom control blocks
///
/// @tparam T The type of the pointer
template<typename T>
class GrHandle
{
public:
	using Type = T;

	GrHandle()
	:	m_cb(nullptr)
	{}

	GrHandle(const GrHandle& b)
	:	GrHandle()
	{
		*this = b;
	}

	~GrHandle()
	{
		reset();
	}

	/// Copy
	GrHandle& operator=(const GrHandle& b)
	{
		reset();
	
		if(b.m_cb)
		{
			auto count = b.m_cb->getRefcount().fetchAdd(1);
			ANKI_ASSERT(count > 0);
			(void)count;

			m_cb = b.m_cb;
		}

		return *this;
	}

	/// Compare
	Bool operator==(const GrHandle& b) const
	{
		return m_cb == b.m_cb;
	}

	/// Compare
	Bool operator!=(const GrHandle& b) const
	{
		return m_cb != b.m_cb;
	}

	/// Compare
	Bool operator<(const GrHandle& b) const
	{
		return m_cb < b.m_cb;
	}

	/// Compare
	Bool operator<=(const GrHandle& b) const
	{
		return m_cb <= b.m_cb;
	}

	/// Compare
	Bool operator>(const GrHandle& b) const
	{
		return m_cb > b.m_cb;
	}

	/// Compare
	Bool operator>=(const GrHandle& b) const
	{
		return m_cb >= b.m_cb;
	}

	/// Create an object and initialize the handle with that.
	/// @param[in,out] manager The manager.
	/// @param[in] del The deleter that will be used to cleanup the allocated
	///                object. Some objects example may require deferred 
	///                deleters.
	template<typename TDeleter>
	ANKI_USE_RESULT Error create(GrManager* manager, TDeleter del)
	{
		ANKI_ASSERT(manager);

		using Cb = CtrlBlock<TDeleter>;

		Error err = ErrorCode::NONE;

		// Create the object
		auto alloc = manager->getAllocator();
		T* ptr = alloc.template newInstance<T>(manager);

		if(ptr != nullptr)
		{
			Cb* cb = alloc.template newInstance<Cb>();

			if(cb != nullptr)
			{
				cb->m_del = del;
				cb->getRefcount().store(1);
				cb->m_ptr = ptr;

				m_cb = cb;
			}
			else
			{
				alloc.template deleteInstance(ptr);
				err = ErrorCode::OUT_OF_MEMORY;
			}
		}
		else
		{
			err = ErrorCode::OUT_OF_MEMORY;
		}

		return err;
	}

	/// Return true if it's pointing to actual data
	Bool isCreated() const
	{
		return m_cb != nullptr;
	}

	/// Get the mutable pointer 
	T& get()
	{
		ANKI_ASSERT(m_cb != nullptr && m_cb->m_ptr != nullptr);
		return *static_cast<T*>(m_cb->m_ptr);
	}

	/// Get the immutable pointer
	const T& get() const
	{
		ANKI_ASSERT(m_cb != nullptr && m_cb->m_ptr != nullptr);
		return *static_cast<T*>(m_cb->m_ptr);
	}

private:
	/// Control block base
	class CtrlBlockBase
	{
	public:
		GrObject* m_ptr;

		virtual ~CtrlBlockBase()
		{}

		Atomic<I32>& getRefcount()
		{
			ANKI_ASSERT(m_ptr);
			return m_ptr->getRefcount();
		}
		
		/// Delete the m_ptr
		virtual void deletePtr() = 0;
	};

	/// Control block with deleter.
	template<typename YDeleter>
	class CtrlBlock: public CtrlBlockBase
	{
	public:	
		using Base = CtrlBlockBase;

		/// The deleter functor
		YDeleter m_del;
		
		void deletePtr()
		{
			// Delete object
			m_del(Base::m_ptr);
		}
	};

	CtrlBlockBase* m_cb;

	/// Reset the pointer to it's original state and remove the references to
	/// any object
	void reset()
	{
		if(m_cb && m_cb->m_ptr)
		{
			auto count = m_cb->getRefcount().fetchSub(1);
			if(count == 1)
			{
				auto alloc = m_cb->m_ptr->getAllocator();
				// Delete pointer
				m_cb->deletePtr();
				m_cb->m_ptr = nullptr;
				// Delete control block
				alloc.deleteInstance(m_cb);
			}

			m_cb = nullptr;
		}
	}
};
/// @}

} // end namespace anki

#endif

