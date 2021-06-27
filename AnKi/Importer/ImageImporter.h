// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Resource/ImageBinary.h>

namespace anki
{

/// @addtogroup importer
/// @{

class ImageImporterConfig
{
public:
	GenericMemoryPoolAllocator<U8> m_allocator;
	ConstWeakArray<CString> m_inputFilenames;
	CString m_outFilename;
	ImageBinaryType m_type = ImageBinaryType::_2D;
	ImageBinaryDataCompression m_compressions = ImageBinaryDataCompression::S3TC;
	U32 m_minMipmapDimension = 4;
	U32 m_mipmapCount = MAX_U32;
	Bool m_noAlpha = true;
};

/// Converts images to AnKi's specific format.
ANKI_USE_RESULT Error importImage(const ImageImporterConfig& config);
/// @}

} // end namespace anki