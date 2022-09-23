// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Common.h>
#include <AnKi/Math.h>

namespace anki {

/// @warning Don't use Array because the compilers can't handle it for some reason.
inline constexpr ShaderVariableDataTypeInfo kShaderVariableDataTypeInfos[] = {
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	{ANKI_STRINGIZE(type), sizeof(type), false, isIntagralType},
#define ANKI_SVDT_MACRO_OPAQUE(constant, type) {ANKI_STRINGIZE(type), kMaxU32, true, false},
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
#undef ANKI_SVDT_MACRO_OPAQUE
};

const ShaderVariableDataTypeInfo& getShaderVariableDataTypeInfo(ShaderVariableDataType type)
{
	ANKI_ASSERT(type > ShaderVariableDataType::kNone && type < ShaderVariableDataType::kCount);
	return kShaderVariableDataTypeInfos[U32(type) - 1];
}

FormatInfo getFormatInfo(Format fmt)
{
	FormatInfo out = {};
	switch(fmt)
	{
#define ANKI_FORMAT_DEF(type, id, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, \
						depthStencil) \
	case Format::k##type: \
		out = {componentCount, \
			   texelSize, \
			   blockWidth, \
			   blockHeight, \
			   blockSize, \
			   shaderType, \
			   DepthStencilAspectBit::k##depthStencil, \
			   ANKI_STRINGIZE(type)}; \
		break;
#include <AnKi/Gr/Format.defs.h>
#undef ANKI_FORMAT_DEF

	default:
		ANKI_ASSERT(0);
	}

	return out;
}

PtrSize computeSurfaceSize(U32 width32, U32 height32, Format fmt)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(fmt != Format::kNone);

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
	ANKI_ASSERT(fmt != Format::kNone);

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

U32 computeMaxMipmapCount2d(U32 w, U32 h, U32 minSizeOfLastMip)
{
	ANKI_ASSERT(w >= minSizeOfLastMip && h >= minSizeOfLastMip);
	U32 s = (w < h) ? w : h;
	U32 count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Compute max number of mipmaps for a 3D texture.
U32 computeMaxMipmapCount3d(U32 w, U32 h, U32 d, U32 minSizeOfLastMip)
{
	U32 s = (w < h) ? w : h;
	s = (s < d) ? s : d;
	U32 count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

} // end namespace anki
