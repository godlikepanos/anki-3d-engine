// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_DUMMY_RSRC_H
#define ANKI_RESOURCE_DUMMY_RSRC_H

#include "anki/resource/Common.h"

namespace anki {

/// @addtogroup resource_private
/// @{

/// A dummy resource for the unit tests of the ResourceManager
class DummyRsrc
{
public:
	DummyRsrc(ResourceAllocator<U8>& alloc)
	:	m_alloc(alloc)
	{}

	~DummyRsrc()
	{
		if(m_memory)
		{
			m_alloc.deallocate(m_memory, 128);
		}
	}

	void load(const CString& filename, ResourceInitializer& init)
	{
		if(filename != "exception")
		{
			m_memory = m_alloc.allocate(128);
			void* tempMem = init.m_tempAlloc.allocate(128);
			(void)tempMem;

			init.m_tempAlloc.deallocate(tempMem, 128);
		}
		else
		{
			throw ANKI_EXCEPTION("Dummy exception");
		}
	}

private:
	void* m_memory = nullptr;
	HeapAllocator<U8> m_alloc;
};

/// @}

} // end namespace anki

#endif
