// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// A generic resource. It just loads a file to memory.
class GenericResource : public ResourceObject
{
public:
	GenericResource(ResourceManager* manager);

	~GenericResource();

	/// Load a texture
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	const DArray<U8>& getData() const
	{
		return m_data;
	}

private:
	DArray<U8> m_data;
};
/// @}

} // end namespace anki
