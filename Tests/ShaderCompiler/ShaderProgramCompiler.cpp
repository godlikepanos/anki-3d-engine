// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/ShaderCompiler/ShaderProgramCompiler.h>
#include <AnKi/Util/ThreadHive.h>

ANKI_TEST(ShaderCompiler, ShaderProgramCompilerSimple)
{
	const CString sourceCode = R"(
#pragma anki mutator INSTANCE_COUNT 1 2 4

struct Foo
{
	Mat4 m_mat;
};

struct Instanced
{
	Mat4 m_ankiMvp;
	Mat3 m_ankiRotationMat;
	Mat4 m_ankiModelViewMat;
	Mat4 m_ankiPrevMvp;
	Foo m_foo[2];
};

layout(push_constant) uniform ankiMaterial
{
	Vec4 u_whatever;
	Instanced u_ankiPerInstance[INSTANCE_COUNT];
	Vec4 u_color;
};

layout(set = 0, binding = 1) uniform texture2D u_tex2d;
layout(set = 0, binding = 1) uniform texture3D u_tex3d;
layout(set = 0, binding = 2) uniform sampler u_sampler;

layout(set = 0, binding = 3) buffer ssbo
{
	Foo u_mats[];
};

#pragma anki start vert
out gl_PerVertex
{
	Vec4 gl_Position;
};

void main()
{
	gl_Position = u_ankiPerInstance[gl_InstanceIndex].m_ankiMvp * u_mats[gl_InstanceIndex].m_mat * Vec4(gl_VertexID);
}
#pragma anki end

#pragma anki start frag
layout(location = 0) out Vec3 out_color;

void main()
{
	out_color = Vec3(0);

	if(INSTANCE_COUNT == 1)
		out_color += textureLod(sampler2D(u_tex2d, u_sampler), Vec2(0), 0.0).rgb;
	else if(INSTANCE_COUNT == 2)
		out_color += textureLod(sampler3D(u_tex3d, u_sampler), Vec3(0), 0.0).rgb;
	else
		out_color += textureLod(sampler2D(u_tex2d, u_sampler), Vec2(0), 0.0).rgb
			+ textureLod(sampler3D(u_tex3d, u_sampler), Vec3(0), 0.0).rgb;

	out_color += u_color.xyz;
}
#pragma anki end
	)";

	// Write the file
	{
		File file;
		ANKI_TEST_EXPECT_NO_ERR(file.open("test.glslp", FileOpenFlag::kWrite));
		ANKI_TEST_EXPECT_NO_ERR(file.writeText(sourceCode));
	}

	class Fsystem : public ShaderProgramFilesystemInterface
	{
	public:
		Error readAllText(CString filename, StringRaii& txt) final
		{
			File file;
			ANKI_CHECK(file.open(filename, FileOpenFlag::kRead));
			ANKI_CHECK(file.readAllText(txt));
			return Error::kNone;
		}
	} fsystem;

	HeapMemoryPool pool(allocAligned, nullptr);

	const U32 threadCount = 8;
	ThreadHive hive(threadCount, &pool);

	class TaskManager : public ShaderProgramAsyncTaskInterface
	{
	public:
		ThreadHive* m_hive = nullptr;
		HeapMemoryPool* m_pool;

		void enqueueTask(void (*callback)(void* userData), void* userData)
		{
			struct Ctx
			{
				void (*m_callback)(void* userData);
				void* m_userData;
				HeapMemoryPool* m_pool;
			};
			Ctx* ctx = newInstance<Ctx>(*m_pool);
			ctx->m_callback = callback;
			ctx->m_userData = userData;
			ctx->m_pool = m_pool;

			m_hive->submitTask(
				[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
				   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
					Ctx* ctx = static_cast<Ctx*>(userData);
					ctx->m_callback(ctx->m_userData);
					deleteInstance(*ctx->m_pool, ctx);
				},
				ctx);
		}

		Error joinTasks()
		{
			m_hive->waitAllTasks();
			return Error::kNone;
		}
	} taskManager;
	taskManager.m_hive = &hive;
	taskManager.m_pool = &pool;

	ShaderProgramBinaryWrapper binary(&pool);
	ShaderCompilerOptions compilerOptions;
	ANKI_TEST_EXPECT_NO_ERR(
		compileShaderProgram("test.glslp", fsystem, nullptr, &taskManager, pool, compilerOptions, binary));

#if 1
	StringRaii dis(&pool);
	dumpShaderProgramBinary(binary.getBinary(), dis);
	ANKI_LOGI("Binary disassembly:\n%s\n", dis.cstr());
#endif
}

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
ANKI_SPECIALIZATION_CONSTANT_VEC3(diffColor, 0u);
#else
layout(set = 0, binding = 1) uniform texture2D diffTex;
#endif

#if SPECULAR_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_VEC3(specColor, 3u);
#else
layout(set = 0, binding = 2) uniform texture2D specTex;
#endif

#if ROUGHNESS_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_F32(roughness, 6u);
#else
layout(set = 0, binding = 3) uniform texture2D roughnessTex;
#endif

#if METAL_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_F32(metallic, 7u);
#else
layout(set = 0, binding = 4) uniform texture2D metallicTex;
#endif

#if EMISSIVE_TEX == 0
ANKI_SPECIALIZATION_CONSTANT_VEC3(emission, 8u, Vec3(0.0));
#else
layout(set = 0, binding = 5) uniform texture2D emissiveTex;
#endif

#if PARALLAX == 1 && LOD == 0
ANKI_SPECIALIZATION_CONSTANT_F32(heightMapScale, 11u);
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
		ANKI_TEST_EXPECT_NO_ERR(file.open("test.glslp", FileOpenFlag::kWrite));
		ANKI_TEST_EXPECT_NO_ERR(file.writeText(sourceCode));
	}

	class Fsystem : public ShaderProgramFilesystemInterface
	{
	public:
		Error readAllText(CString filename, StringRaii& txt) final
		{
			File file;
			ANKI_CHECK(file.open(filename, FileOpenFlag::kRead));
			ANKI_CHECK(file.readAllText(txt));
			return Error::kNone;
		}
	} fsystem;

	HeapMemoryPool pool(allocAligned, nullptr);

	const U32 threadCount = 24;
	ThreadHive hive(threadCount, &pool);

	class TaskManager : public ShaderProgramAsyncTaskInterface
	{
	public:
		ThreadHive* m_hive = nullptr;
		HeapMemoryPool* m_pool = nullptr;

		void enqueueTask(void (*callback)(void* userData), void* userData)
		{
			struct Ctx
			{
				void (*m_callback)(void* userData);
				void* m_userData;
				HeapMemoryPool* m_pool;
			};
			Ctx* ctx = newInstance<Ctx>(*m_pool);
			ctx->m_callback = callback;
			ctx->m_userData = userData;
			ctx->m_pool = m_pool;

			m_hive->submitTask(
				[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
				   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
					Ctx* ctx = static_cast<Ctx*>(userData);
					ctx->m_callback(ctx->m_userData);
					deleteInstance(*ctx->m_pool, ctx);
				},
				ctx);
		}

		Error joinTasks()
		{
			m_hive->waitAllTasks();
			return Error::kNone;
		}
	} taskManager;
	taskManager.m_hive = &hive;
	taskManager.m_pool = &pool;

	ShaderProgramBinaryWrapper binary(&pool);
	ANKI_TEST_EXPECT_NO_ERR(
		compileShaderProgram("test.glslp", fsystem, nullptr, &taskManager, pool, ShaderCompilerOptions(), binary));

#if 1
	StringRaii dis(&pool);
	dumpShaderProgramBinary(binary.getBinary(), dis);
	ANKI_LOGI("Binary disassembly:\n%s\n", dis.cstr());
#endif
}
