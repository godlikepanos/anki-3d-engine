// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr.h>
#include <AnKi/ShaderCompiler.h>
#include <AnKi/ShaderCompiler/ShaderProgramParser.h>
#include <Tests/Framework/Framework.h>

namespace anki {

inline ShaderPtr createShader(CString src, ShaderType type, GrManager& gr,
							  ConstWeakArray<ShaderSpecializationConstValue> specVals = {})
{
	HeapMemoryPool pool(allocAligned, nullptr);
	StringRaii header(&pool);
	ShaderCompilerOptions compilerOptions;
	ShaderProgramParser::generateAnkiShaderHeader(type, compilerOptions, header);
	header.append(src);
	DynamicArrayRaii<U8> spirv(&pool);
	StringRaii errorLog(&pool);

	ANKI_ASSERT(!"TODO");
	// ANKI_TEST_EXPECT_NO_ERR(compileGlslToSpirv(header, type, pool, spirv, errorLog));

	ShaderInitInfo initInf(type, spirv);
	initInf.m_constValues = specVals;

	return gr.newShader(initInf);
}

} // end namespace anki
