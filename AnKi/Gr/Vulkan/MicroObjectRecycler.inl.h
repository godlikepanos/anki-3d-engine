// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/MicroObjectRecycler.h>

namespace anki {

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
inline U32 MicroObjectRecycler<T>::releaseFences()
{
	U32 objectsThatCanBeDestroyed = 0;
	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		T& obj = *m_objects[i];
		if(obj.getFence() && obj.getFence()->done())
		{
			obj.getFence().reset(nullptr);
			++objectsThatCanBeDestroyed;
		}
	}

	return objectsThatCanBeDestroyed;
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
inline void MicroObjectRecycler<T>::trimCache(U32 objectsToNotDestroy)
{
	LockGuard<Mutex> lock(m_mtx);

	U32 objectsThatCoultBeDeletedCount = releaseFences();

	DynamicArray<T*> aliveObjects;

	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		T* obj = m_objects[i];
		ANKI_ASSERT(obj);
		ANKI_ASSERT(obj->getRefcount().getNonAtomically() == 0);

		if(obj->getFence() || objectsThatCoultBeDeletedCount <= objectsToNotDestroy)
		{
			// Can't delete it
			aliveObjects.emplaceBack(m_alloc, obj);
		}
		else
		{
			auto alloc = obj->getAllocator();
			alloc.deleteInstance(obj);
			--objectsThatCoultBeDeletedCount;
#if ANKI_EXTRA_CHECKS
			--m_createdAndNotRecycled;
#endif
		}
	}

	if(aliveObjects.getSize() > 0)
	{
		// Some alive, store the alive
		m_objects.destroy(m_alloc);
		m_objects = std::move(aliveObjects);
	}
	else if(aliveObjects.getSize() == 0 && m_objects.getSize() >= 0)
	{
		// All dead, destroy the array
		m_objects.destroy(m_alloc);
	}
}

} // end namespace anki
