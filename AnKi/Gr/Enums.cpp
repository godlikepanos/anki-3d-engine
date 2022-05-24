// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Enums.h>
#include <AnKi/Math.h>

namespace anki {

/// @warning Don't use Array because the compilers can't handle it for some reason.
static constexpr ShaderVariableDataTypeInfo SVD_INFOS[] = {
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount, isIntagralType) \
	{ANKI_STRINGIZE(type), sizeof(type), false, isIntagralType},
#define ANKI_SVDT_MACRO_OPAQUE(capital, type) {ANKI_STRINGIZE(type), MAX_U32, true, false},
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
#undef ANKI_SVDT_MACRO_OPAQUE
};

const ShaderVariableDataTypeInfo& getShaderVariableDataTypeInfo(ShaderVariableDataType type)
{
	ANKI_ASSERT(type > ShaderVariableDataType::NONE && type < ShaderVariableDataType::COUNT);
	return SVD_INFOS[U32(type) - 1];
}

FormatInfo getFormatInfo(Format fmt)
{
	FormatInfo out = {};
	switch(fmt)
	{
#define ANKI_FORMAT_DEF(type, id, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, \
						depthStencil) \
	case Format::type: \
		out = {componentCount, \
			   texelSize, \
			   blockWidth, \
			   blockHeight, \
			   blockSize, \
			   shaderType, \
			   DepthStencilAspectBit::depthStencil, \
			   ANKI_STRINGIZE(type)}; \
		break;
#include <AnKi/Gr/Format.defs.h>
#undef ANKI_FORMAT_DEF

	default:
		ANKI_ASSERT(0);
	}

	return out;
}

} // namespace anki
