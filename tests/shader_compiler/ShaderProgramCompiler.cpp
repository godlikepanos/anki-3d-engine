// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/shader_compiler/ShaderProgramCompiler.h>

ANKI_TEST(ShaderCompiler, ShaderProgramCompiler)
{
	const CString sourceCode = R"(
#pragma anki mutator LOD 0 1 2
#pragma anki mutator PASS 0 1 2 3
#pragma anki mutator DIFFUSE_TEX 0 1

#pragma anki rewrite_mutation PASS 1 DIFFUSE_TEX 1 to PASS 1 DIFFUSE_TEX 0
#pragma anki rewrite_mutation PASS 2 DIFFUSE_TEX 1 to PASS 2 DIFFUSE_TEX 0
#pragma anki rewrite_mutation PASS 3 DIFFUSE_TEX 1 to PASS 2 DIFFUSE_TEX 0

ANKI_SPECIALIZATION_CONSTANT_I32(INSTANCE_COUNT, 0, 1);

struct PerInstance
{
	Mat4 m_mvp;
#if PASS > 1
	Mat3 m_normalMat;
#endif
};

layout(set = 1, binding = 0) uniform perInstance
{
	PerInstance u_perInstance[INSTANCE_COUNT];
};

layout(set = 1, binding = 1) buffer perDrawcall
{
	Vec4 u_color;
#if PASS > 1
	Mat3 u_someMat;
#endif
};

#if DIFFUSE_TEX == 1
layout(set = 0, binding = 0) uniform texture2D u_tex[3];
#endif
layout(set = 0, binding = 1) uniform sampler u_sampler;

#pragma anki start vert
ANKI_SPECIALIZATION_CONSTANT_F32(specConst, 1, 0);

out gl_PerVertex
{
	Vec4 gl_Position;
};

void main()
{
	gl_Position = u_perInstance[gl_InstanceID].m_mvp * Vec4(specConst);
}
#pragma anki end

#pragma anki start frag
layout(location = 0) out Vec4 out_color;

void main()
{
#if DIFFUSE_TEX == 1
	out_color = texture(sampler2D(u_tex[0], u_sampler), Vec2(0));
#else
	out_color = u_color;
#endif
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

#if 1
	StringAuto dis(alloc);
	disassembleShaderProgramBinary(binary.getBinary(), dis);
	ANKI_LOGI("Binary disassembly:\n%s\n", dis.cstr());
#endif
}
