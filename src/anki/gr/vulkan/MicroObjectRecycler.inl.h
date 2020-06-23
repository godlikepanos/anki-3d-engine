// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/MicroObjectRecycler.h>

namespace anki
{

template<typename T>
inline void MicroObjectRecycler<T>::destroy()
{
	LockGuard<Mutex> lock(m_mtx);

	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		T* obj = m_objects[i];
		ANKI_ASSERT(obj);
		ANKI_ASSERT(!obj->getFence() || obj->getFence()->done());

		auto alloc = obj->getAllocator();
		alloc.deleteInstance(obj);
#if ANKI_EXTRA_CHECKS
		--m_createdAndNotRecycled;
#endif
	}

	m_objects.destroy(m_alloc);
	ANKI_ASSERT(m_createdAndNotRecycled == 0 && "Destroying the recycler while objects have not recycled yet");
}

template<typename T>
inline void MicroObjectRecycler<T>::releaseFences()
{
	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		T& obj = *m_objects[i];
		if(obj.getFence() && obj.getFence()->done())
		{
			obj.getFence().reset(nullptr);
		}
	}
}

template<typename T>
inline T* MicroObjectRecycler<T>::findToReuse()
{
	T* out = nullptr;
	LockGuard<Mutex> lock(m_mtx);

	if(m_objects.getSize() > 0)
	{
		releaseFences();

		for(U32 i = 0; i < m_objects.getSize(); ++i)
		{
			if(!m_objects[i]->getFence())
			{
				out = m_objects[i];
				m_objects[i] = m_objects[m_objects.getSize() - 1];
				m_objects.popBack(m_alloc);
				break;
			}
		}
	}

	ANKI_ASSERT(out == nullptr || out->getRefcount().getNonAtomically() == 0);

#if ANKI_EXTRA_CHECKS
	if(out == nullptr)
	{
		++m_createdAndNotRecycled;
	}
#endif

	return out;
}

template<typename T>
inline void MicroObjectRecycler<T>::recycle(T* s)
{
	ANKI_ASSERT(s);
	ANKI_ASSERT(s->getRefcount().getNonAtomically() == 0);

	LockGuard<Mutex> lock(m_mtx);

	releaseFences();

	m_objects.emplaceBack(m_alloc, s);
}

template<typename T>
inline void MicroObjectRecycler<T>::trimCache()
{
	LockGuard<Mutex> lock(m_mtx);

	releaseFences();

	DynamicArray<T*> newObjects;

	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		T* obj = m_objects[i];
		ANKI_ASSERT(obj);
		ANKI_ASSERT(obj->getRefcount().getNonAtomically() == 0);

		if(obj->getFence())
		{
			// Can't delete it
			newObjects.emplaceBack(m_alloc, obj);
		}
		else
		{
			auto alloc = obj->getAllocator();
			alloc.deleteInstance(obj);
		}
	}

	if(newObjects.getSize() > 0)
	{
		m_objects.destroy(m_alloc);
		m_objects = std::move(newObjects);
	}
}

} // end namespace anki
