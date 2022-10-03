// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/ShaderCompiler/ShaderProgramParser.h>

ANKI_TEST(ShaderCompiler, ShaderCompilerParser)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	class FilesystemInterface : public ShaderProgramFilesystemInterface
	{
	public:
		U32 count = 0;

		Error readAllText([[maybe_unused]] CString filename, StringRaii& txt) final
		{
			if(count == 0)
			{
				txt = R"(
#pragma anki mutator M0 1 2
#pragma anki mutator M1 3 4

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
				return Error::kFunctionFailed;
			}

			++count;
			return Error::kNone;
		}
	} interface;

	ShaderProgramParser parser("filename0", &interface, &pool, ShaderCompilerOptions());
	ANKI_TEST_EXPECT_NO_ERR(parser.parse());

	// Test a variant
	Array<MutatorValue, 2> mutation = {{2, 4}};

	ShaderProgramParserVariant variant;
	ANKI_TEST_EXPECT_NO_ERR(parser.generateVariant(mutation, variant));

	// printf("%s\n", variant.getSource(ShaderType::kVertex).cstr());
}
