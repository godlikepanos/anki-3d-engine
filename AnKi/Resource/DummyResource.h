// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>

namespace anki {

/// @addtogroup resource_private
/// @{

/// A dummy resource for the unit tests of the ResourceManager
class DummyResource : public ResourceObject
{
public:
	DummyResource(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~DummyResource()
	{
		if(m_memory)
		{
			getAllocator().deallocate(m_memory, 128);
		}
	}

	Error load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
	{
		Error err = Error::kNone;
		if(filename.find("error") == CString::kNpos)
		{
			m_memory = getAllocator().allocate(128);
			void* tempMem = getTempAllocator().allocate(128);

			getTempAllocator().deallocate(tempMem, 128);
		}
		else
		{
			ANKI_RESOURCE_LOGE("Dummy error");
			err = Error::kUserData;
		}

		return err;
	}

private:
	void* m_memory = nullptr;
};
/// @}

} // end namespace anki
