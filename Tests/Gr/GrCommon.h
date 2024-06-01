// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr.h>
#include <AnKi/ShaderCompiler.h>
#include <AnKi/ShaderCompiler/ShaderParser.h>
#include <AnKi/ShaderCompiler/Dxc.h>
#include <Tests/Framework/Framework.h>

namespace anki {

inline ShaderPtr createShader(CString src, ShaderType type)
{
	ShaderCompilerString header;
	ShaderParser::generateAnkiShaderHeader(type, header);
	header += src;
	ShaderCompilerDynamicArray<U8> bin;
	ShaderCompilerString errorLog;

#if ANKI_GR_BACKEND_VULKAN
	const Error err = compileHlslToSpirv(header, type, false, bin, errorLog);
#else
	const Error err = compileHlslToDxil(header, type, false, bin, errorLog);
#endif
	if(err)
	{
		ANKI_TEST_LOGE("Compile error:\n%s", errorLog.cstr());
	}
	ANKI_TEST_EXPECT_NO_ERR(err);

	ShaderReflection refl;
	ShaderCompilerString errorStr;
#if ANKI_GR_BACKEND_VULKAN
	ANKI_TEST_EXPECT_NO_ERR(doReflectionSpirv(WeakArray(bin.getBegin(), bin.getSize()), type, refl, errorStr));
#else
	ANKI_TEST_EXPECT_NO_ERR(doReflectionDxil(bin, type, refl, errorStr));
#endif

	ShaderInitInfo initInf(type, bin);
	initInf.m_reflection = refl;

	return GrManager::getSingleton().newShader(initInf);
}

} // end namespace anki
