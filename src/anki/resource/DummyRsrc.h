// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>

namespace anki
{

/// @addtogroup resource_private
/// @{

/// A dummy resource for the unit tests of the ResourceManager
class DummyRsrc : public ResourceObject
{
public:
	DummyRsrc(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~DummyRsrc()
	{
		if(m_memory)
		{
			getAllocator().deallocate(m_memory, 128);
		}
	}

	ANKI_USE_RESULT Error load(const ResourceFilename& filename)
	{
		Error err = ErrorCode::NONE;
		if(filename.find("error") == ResourceFilename::NPOS)
		{
			m_memory = getAllocator().allocate(128);
			void* tempMem = getTempAllocator().allocate(128);
			(void)tempMem;

			getTempAllocator().deallocate(tempMem, 128);
		}
		else
		{
			ANKI_RESOURCE_LOGE("Dummy error");
			err = ErrorCode::USER_DATA;
		}

		return err;
	}

private:
	void* m_memory = nullptr;
};
/// @}

} // end namespace anki
