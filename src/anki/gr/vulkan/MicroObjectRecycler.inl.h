// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

	U32 count = m_objectCount;
	while(count--)
	{
		T* obj = m_objects[count];
		ANKI_ASSERT(obj);
		ANKI_ASSERT(!obj->getFence() || obj->getFence()->done());

		auto alloc = obj->getAllocator();
		alloc.deleteInstance(obj);
#if ANKI_EXTRA_CHECKS
		--m_createdAndNotRecycled;
#endif
	}

	m_objectCount = 0;
	m_objects.destroy(m_alloc);
	ANKI_ASSERT(m_createdAndNotRecycled == 0 && "Destroying the recycler while objects have not recycled yet");
}

template<typename T>
inline void MicroObjectRecycler<T>::releaseFences()
{
	U32 count = m_objectCount;
	while(count--)
	{
		T& obj = *m_objects[count];
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

	if(m_objectCount > 0)
	{
		releaseFences();

		U32 count = m_objectCount;
		while(count--)
		{
			if(!m_objects[count]->getFence())
			{
				out = m_objects[count];

				// Pop it
				for(U32 i = count; i < m_objectCount - 1; ++i)
				{
					m_objects[i] = m_objects[i + 1];
				}

				--m_objectCount;

				break;
			}
		}

		ANKI_ASSERT(out->getRefcount().getNonAtomically() == 0);
	}

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

	if(m_objects.getSize() <= m_objectCount)
	{
		// Grow storage
		m_objects.resize(m_alloc, max<U32>(1, m_objects.getSize() * 2));
	}

	m_objects[m_objectCount] = s;
	++m_objectCount;
}

} // end namespace anki
