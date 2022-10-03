// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Resource/ImageBinary.h>

namespace anki {

/// @addtogroup importer
/// @{

/// Config for importImage().
/// @relates importImage.
class ImageImporterConfig
{
public:
	BaseMemoryPool* m_pool = nullptr;
	ConstWeakArray<CString> m_inputFilenames;
	CString m_outFilename;
	ImageBinaryType m_type = ImageBinaryType::k2D;
	ImageBinaryDataCompression m_compressions = ImageBinaryDataCompression::kS3tc;
	U32 m_minMipmapDimension = 4;
	U32 m_mipmapCount = kMaxU32;
	Bool m_noAlpha = true;
	CString m_tempDirectory;
	CString m_compressonatorFilename; ///< Optional.
	CString m_astcencFilename; ///< Optional.
	Vec3 m_hdrScale = Vec3(1.0f); ///< Scale the values of HDR textures.
	Vec3 m_hdrBias = Vec3(0.0f); ///< Add that value to the HDR textures.
	UVec2 m_astcBlockSize = UVec2(8u);
	Bool m_sRgbToLinear = false;
	Bool m_linearToSRgb = false;
	Bool m_flipImage = true;
};

/// Converts images to AnKi's specific format.
Error importImage(const ImageImporterConfig& config);
/// @}

} // end namespace anki
