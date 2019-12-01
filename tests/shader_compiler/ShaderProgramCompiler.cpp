// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/shader_compiler/ShaderProgramCompiler.h>

ANKI_TEST(ShaderCompiler, ShaderProgramCompiler)
{
	const CString sourceCode = R"(
#pragma anki start vert
out gl_PerVertex
{
	Vec4 gl_Position;
};

void main()
{
	gl_Position = Vec4(gl_Position);
}
#pragma anki end

#pragma anki start frag
layout(location = 0) out Vec3 out_color;

void main()
{
	out_color = Vec3(0.0);
}
#pragma anki end
	)";

	// Write the file
	{
		File file;
		ANKI_TEST_EXPECT_NO_ERR(file.open("test.glslp", FileOpenFlag::WRITE));
		ANKI_TEST_EXPECT_NO_ERR(file.writeText(sourceCode));
	}

	class Fsystem : public ShaderProgramFilesystemInterface
	{
	public:
		Error readAllText(CString filename, StringAuto& txt) final
		{
			File file;
			ANKI_CHECK(file.open(filename, FileOpenFlag::READ));
			ANKI_CHECK(file.readAllText(txt));
			return Error::NONE;
		}
	} fsystem;

	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ShaderProgramBinaryWrapper binary(alloc);
	ANKI_TEST_EXPECT_NO_ERR(compileShaderProgram("test.glslp", fsystem, alloc, 128, 1, 1, GpuVendor::AMD, binary));
}
