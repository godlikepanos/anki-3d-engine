// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/shader_compiler/ShaderProgramParser.h>

ANKI_TEST(ShaderCompiler, ShaderCompilerParser)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	class FilesystemInterface : public ShaderProgramParserFilesystemInterface
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

#if M0 == 1
#pragma anki input Vec3 var0
#endif

#if M1 == 4
#pragma anki input const Vec3 var1
#endif

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

	ShaderProgramParser parser("filename0", &interface, alloc, 128, 1, 1, GpuVendor::AMD);
	ANKI_TEST_EXPECT_NO_ERR(parser.parse());

	// Check inputs
	ANKI_TEST_EXPECT_EQ(parser.getInputs().getSize(), 2);

	// Test a variant
	Array<ShaderProgramParserMutatorState, 2> arr = {{{&parser.getMutators()[0], 2}, {&parser.getMutators()[1], 4}}};
	ConstWeakArray<ShaderProgramParserMutatorState> mutatorStates(arr);

	ShaderProgramParserVariant variant;
	ANKI_TEST_EXPECT_NO_ERR(parser.generateVariant(mutatorStates, variant));
	ANKI_TEST_EXPECT_EQ(variant.isInputActive(parser.getInputs()[0]), false);
	ANKI_TEST_EXPECT_EQ(variant.isInputActive(parser.getInputs()[1]), true);
	// printf("%s\n", variant.getSource(ShaderType::VERTEX).cstr());
}
