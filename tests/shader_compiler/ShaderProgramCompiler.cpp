// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/shader_compiler/ShaderProgramCompiler.h>

ANKI_TEST(ShaderCompiler, ShaderProgramCompiler)
{
	const CString sourceCode = R"(
#pragma anki mutator instanceCount INSTANCE_COUNT 1 2 4 8 16 32 64
#pragma anki mutator LOD 0 1 2
#pragma anki mutator PASS 0 1 2 3
#pragma anki mutator DIFFUSE_TEX 0 1
#pragma anki mutator SPECULAR_TEX 0 1
#pragma anki mutator ROUGHNESS_TEX 0 1
#pragma anki mutator METAL_TEX 0 1
#pragma anki mutator NORMAL_TEX 0 1
#pragma anki mutator PARALLAX 0 1
#pragma anki mutator EMISSIVE_TEX 0 1
#pragma anki mutator BONES 0 1
#pragma anki mutator VELOCITY 0 1

#pragma anki input instanced Mat4 mvp
#if PASS == 0
#	pragma anki input instanced Mat3 rotationMat
#endif
#if PASS == 0 && PARALLAX == 1
#	pragma anki input instanced Mat4 modelViewMat
#endif
#if PASS == 0 && VELOCITY == 1
#	pragma anki input instanced Mat4 prevMvp
#endif

#if DIFFUSE_TEX == 0 && PASS == 0
#	pragma anki input const Vec3 diffColor
#endif
#if SPECULAR_TEX == 0 && PASS == 0
#	pragma anki input const Vec3 specColor
#endif
#if ROUGHNESS_TEX == 0 && PASS == 0
#	pragma anki input const F32 roughness
#endif
#if METAL_TEX == 0 && PASS == 0
#	pragma anki input const F32 metallic
#endif
#if EMISSIVE_TEX == 0 && PASS == 0
#	pragma anki input const Vec3 emission
#endif
#if PARALLAX == 1 && PASS == 0 && LOD == 0
#	pragma anki input const F32 heightMapScale
#endif
#if PASS == 0
#	pragma anki input const F32 subsurface
#endif
#pragma anki input sampler globalSampler
#if DIFFUSE_TEX == 1 && PASS == 0
#	pragma anki input texture2D diffTex
#endif
#if SPECULAR_TEX == 1 && PASS == 0
#	pragma anki input texture2D specTex
#endif
#if ROUGHNESS_TEX == 1 && PASS == 0
#	pragma anki input texture2D roughnessTex
#endif
#if METAL_TEX == 1 && PASS == 0
#	pragma anki input texture2D metallicTex
#endif
#if NORMAL_TEX == 1 && PASS == 0 && LOD < 2
#	pragma anki input texture2D normalTex
#endif
#if PARALLAX == 1 && PASS == 0 && LOD == 0
#	pragma anki input texture2D heightTex
#endif
#if EMISSIVE_TEX == 1 && PASS == 0
#	pragma anki input texture2D emissiveTex
#endif

#pragma anki start vert
out gl_PerVertex
{
	Vec4 gl_Position;
};

void main()
{
	gl_Position = Vec4(gl_VertexID);
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

	StringAuto dis(alloc);
	disassembleShaderProgramBinary(binary.getBinary(), dis);
	ANKI_LOGI("Binary disassembly:\n%s\n", dis.cstr());
}
