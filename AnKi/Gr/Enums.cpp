// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Enums.h>
#include <AnKi/Math.h>

namespace anki {

/// @warning Don't use Array because the compilers can't handle it for some reason.
static constexpr ShaderVariableDataTypeInfo SVD_INFOS[] = {
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) {ANKI_STRINGIZE(type), sizeof(type), false},
#define ANKI_SVDT_MACRO_OPAQUE(capital, type) {ANKI_STRINGIZE(type), MAX_U32, true},
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
#undef ANKI_SVDT_MACRO_OPAQUE
};

const ShaderVariableDataTypeInfo& getShaderVariableDataTypeInfo(ShaderVariableDataType type)
{
	ANKI_ASSERT(type > ShaderVariableDataType::NONE && type < ShaderVariableDataType::COUNT);
	return SVD_INFOS[U32(type) - 1];
}

/// @warning Don't use Array because the compilers can't handle it for some reason.
static constexpr FormatInfo FORMAT_INFOS[] = {
#define ANKI_FORMAT_DEF(type, id, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, \
						depthStencil) \
	{componentCount, \
	 texelSize, \
	 blockWidth, \
	 blockHeight, \
	 blockSize, \
	 shaderType, \
	 DepthStencilAspectBit::depthStencil, \
	 ANKI_STRINGIZE(type)},
#include <AnKi/Gr/Format.defs.h>
#undef ANKI_FORMAT_DEF
};

const FormatInfo& getFormatInfo(Format fmt)
{
	ANKI_ASSERT(fmt > Format::NONE && fmt < Format::COUNT);
	return FORMAT_INFOS[U32(fmt) - 1];
}

} // namespace anki
