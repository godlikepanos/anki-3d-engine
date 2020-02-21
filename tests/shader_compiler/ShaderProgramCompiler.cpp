// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/shader_compiler/ShaderProgramCompiler.h>

ANKI_TEST(ShaderCompiler, ShaderProgramCompiler)
{
	const CString sourceCode = R"(
#pragma anki mutator INSTANCE_COUNT 1 2 4 8 16 32 64
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

#pragma anki rewrite_mutation PASS 1 DIFFUSE_TEX 1 to PASS 1 DIFFUSE_TEX 0
#pragma anki rewrite_mutation PASS 2 DIFFUSE_TEX 1 to PASS 2 DIFFUSE_TEX 0
#pragma anki rewrite_mutation PASS 3 DIFFUSE_TEX 1 to PASS 2 DIFFUSE_TEX 0

#pragma anki rewrite_mutation PASS 1 SPECULAR_TEX 1 to PASS 1 SPECULAR_TEX 0
#pragma anki rewrite_mutation PASS 2 SPECULAR_TEX 1 to PASS 2 SPECULAR_TEX 0
#pragma anki rewrite_mutation PASS 3 SPECULAR_TEX 1 to PASS 2 SPECULAR_TEX 0

#pragma anki rewrite_mutation PASS 1 ROUGHNESS_TEX 1 to PASS 1 ROUGHNESS_TEX 0
#pragma anki rewrite_mutation PASS 2 ROUGHNESS_TEX 1 to PASS 2 ROUGHNESS_TEX 0
#pragma anki rewrite_mutation PASS 3 ROUGHNESS_TEX 1 to PASS 2 ROUGHNESS_TEX 0

#pragma anki rewrite_mutation PASS 1 METAL_TEX 1 to PASS 1 METAL_TEX 0
#pragma anki rewrite_mutation PASS 2 METAL_TEX 1 to PASS 2 METAL_TEX 0
#pragma anki rewrite_mutation PASS 3 METAL_TEX 1 to PASS 2 METAL_TEX 0

#pragma anki rewrite_mutation PASS 1 NORMAL_TEX 1 to PASS 1 NORMAL_TEX 0
#pragma anki rewrite_mutation PASS 2 NORMAL_TEX 1 to PASS 2 NORMAL_TEX 0
#pragma anki rewrite_mutation PASS 3 NORMAL_TEX 1 to PASS 2 NORMAL_TEX 0

#pragma anki rewrite_mutation PASS 1 EMISSIVE_TEX 1 to PASS 1 EMISSIVE_TEX 0
#pragma anki rewrite_mutation PASS 2 EMISSIVE_TEX 1 to PASS 2 EMISSIVE_TEX 0
#pragma anki rewrite_mutation PASS 3 EMISSIVE_TEX 1 to PASS 2 EMISSIVE_TEX 0

#pragma anki rewrite_mutation PASS 1 VELOCITY 1 to PASS 1 VELOCITY 0
#pragma anki rewrite_mutation PASS 2 VELOCITY 1 to PASS 2 VELOCITY 0
#pragma anki rewrite_mutation PASS 3 VELOCITY 1 to PASS 2 VELOCITY 0

layout(set = 0, binding = 0) uniform ankiMaterial
{
	Mat4 u_ankiMvp[INSTANCE_COUNT];

#if PASS == 0
	Mat3 u_ankiRotationMat[INSTANCE_COUNT];
#endif

#if PASS == 0 && PARALLAX == 1
	Mat4 u_ankiModelViewMat[INSTANCE_COUNT];
#endif

#if PASS == 0 && VELOCITY == 1
	Mat4 u_ankiPrevMvp[INSTANCE_COUNT];
#endif
};

#if PASS == 0

#if DIFFUSE_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_VEC3(diffColor, 0, Vec3(0));
#else
layout(set = 0, binding = 1) uniform texture2D diffTex;
#endif

#if SPECULAR_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_VEC3(specColor, 3, Vec3(0));
#else
layout(set = 0, binding = 2) uniform texture2D specTex;
#endif

#if ROUGHNESS_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_F32(roughness, 6, 0.0);
#else
layout(set = 0, binding = 3) uniform texture2D roughnessTex;
#endif

#if METAL_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_F32(metallic, 7, 0.0);
#else
layout(set = 0, binding = 4) uniform texture2D metallicTex;
#endif

#if EMISSIVE_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_VEC3(emission, 8, Vec3(0.0));
#else
layout(set = 0, binding = 5) uniform texture2D emissiveTex;
#endif

#if PARALLAX == 1 && LOD == 0
ANKI_SPECIALIZATION_CONSTANT_F32(heightMapScale, 11, 0.0);
layout(set = 0, binding = 6) uniform texture2D heightTex;
#endif

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
	BindlessLimits bindlessLimits;
	ANKI_TEST_EXPECT_NO_ERR(
		compileShaderProgram("test.glslp", fsystem, alloc, 128, 1, 1, GpuVendor::AMD, bindlessLimits, binary));

#if 0
	StringAuto dis(alloc);
	disassembleShaderProgramBinary(binary.getBinary(), dis);
	ANKI_LOGI("Binary disassembly:\n%s\n", dis.cstr());
#endif
}
