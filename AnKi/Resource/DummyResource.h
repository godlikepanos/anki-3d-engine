// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

/// @addtogroup resource_private
/// @{

/// A dummy resource for the unit tests of the ResourceManager
class DummyResource : public ResourceObject
{
public:
	DummyResource() = default;

	~DummyResource()
	{
		if(m_memory)
		{
			ResourceMemoryPool::getSingleton().free(m_memory);
		}
	}

	Error load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
	{
		Error err = Error::kNone;
		if(filename.find("error") == CString::kNpos)
		{
			m_memory = ResourceMemoryPool::getSingleton().allocate(128, 1);
			void* tempMem = ResourceMemoryPool::getSingleton().allocate(128, 1);

			ResourceMemoryPool::getSingleton().free(tempMem);
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
