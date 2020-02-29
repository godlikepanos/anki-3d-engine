// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/shader_compiler/ShaderProgramParser.h>

ANKI_TEST(ShaderCompiler, ShaderCompilerParser)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	class FilesystemInterface : public ShaderProgramFilesystemInterface
	{
	public:
		U32 count = 0;

		Error readAllText(CString filename, StringAuto& txt) final
		{
			if(count == 0)
			{
				txt = R"(
#pragma anki mutator M0 1 2
#pragma anki mutator M1 3 4

#pragma anki rewrite_mutation M0 2 M1 4 to M0 1 M1 3

#pragma anki start vert

// vert
#pragma anki end

#pragma anki start frag
// frag
#pragma anki end
				)";
			}
			else
			{
				return Error::FUNCTION_FAILED;
			}

			++count;
			return Error::NONE;
		}
	} interface;

	BindlessLimits bindlessLimits;
	GpuDeviceCapabilities gpuCapabilities;
	ShaderProgramParser parser("filename0", &interface, alloc, gpuCapabilities, bindlessLimits);
	ANKI_TEST_EXPECT_NO_ERR(parser.parse());

	// Test a variant
	Array<MutatorValue, 2> mutation = {{2, 4}};

	ShaderProgramParserVariant variant;
	ANKI_TEST_EXPECT_NO_ERR(parser.generateVariant(mutation, variant));

	// Test rewrite
	ANKI_TEST_EXPECT_EQ(parser.rewriteMutation(mutation), true);
	ANKI_TEST_EXPECT_EQ(mutation[0], 1);
	ANKI_TEST_EXPECT_EQ(mutation[1], 3);

	// printf("%s\n", variant.getSource(ShaderType::VERTEX).cstr());
}
