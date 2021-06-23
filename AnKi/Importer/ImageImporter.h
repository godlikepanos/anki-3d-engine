// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/Common.h>
#include <AnKi/Util/Allocator.h>

namespace anki
{

/// @addtogroup importer
/// @{

class ImageImporterInfo
{
public:
	GenericMemoryPoolAllocator<U8> m_allocator;
};

/// Converts images to AnKi's specific format.
ANKI_USE_RESULT Error importImage(const ImageImporterInfo& info);
/// @}

} // end namespace anki
