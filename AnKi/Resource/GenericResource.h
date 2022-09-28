// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup resource
/// @{

/// A generic resource. It just loads a file to memory.
class GenericResource : public ResourceObject
{
public:
	GenericResource(ResourceManager* manager);

	~GenericResource();

	Error load(const ResourceFilename& filename, Bool async);

	ConstWeakArray<U8> getData() const
	{
		return m_data;
	}

private:
	DynamicArray<U8> m_data;
};
/// @}

} // end namespace anki
