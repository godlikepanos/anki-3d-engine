// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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

	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceInitializer& init)
	{
		Error err = ErrorCode::NONE;
		if(filename != "error")
		{
			m_memory = m_alloc.allocate(128);
			void* tempMem = init.m_tempAlloc.allocate(128);
			(void)tempMem;

			init.m_tempAlloc.deallocate(tempMem, 128);
		}
		else
		{
			ANKI_LOGE("Dummy error");
			err = ErrorCode::USER_DATA;
		}

		return err;
	}

private:
	void* m_memory = nullptr;
	HeapAllocator<U8> m_alloc;
};

/// @}

} // end namespace anki

#endif
