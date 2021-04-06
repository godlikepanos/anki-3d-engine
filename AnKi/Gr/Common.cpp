// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Common.h>

namespace anki
{

Array<CString, U(GpuVendor::COUNT)> GPU_VENDOR_STR = {"UNKNOWN", "ARM", "NVIDIA", "AMD", "INTEL"};

PtrSize computeSurfaceSize(U32 width32, U32 height32, Format fmt)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(fmt != Format::NONE);

	const FormatInfo inf = getFormatInfo(fmt);

	if(inf.m_blockSize > 0)
	{
		// Compressed
		ANKI_ASSERT((width % inf.m_blockWidth) == 0);
		ANKI_ASSERT((height % inf.m_blockHeight) == 0);
		return (width / inf.m_blockWidth) * (height / inf.m_blockHeight) * inf.m_blockSize;
	}
	else
	{
		return width * height * inf.m_texelSize;
	}
}

PtrSize computeVolumeSize(U32 width32, U32 height32, U32 depth32, Format fmt)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	const PtrSize depth = depth32;
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	ANKI_ASSERT(fmt != Format::NONE);

	const FormatInfo inf = getFormatInfo(fmt);

	if(inf.m_blockSize > 0)
	{
		// Compressed
		ANKI_ASSERT((width % inf.m_blockWidth) == 0);
		ANKI_ASSERT((height % inf.m_blockHeight) == 0);
		return (width / inf.m_blockWidth) * (height / inf.m_blockHeight) * inf.m_blockSize * depth;
	}
	else
	{
		return width * height * depth * inf.m_texelSize;
	}
}

} // end namespace anki
