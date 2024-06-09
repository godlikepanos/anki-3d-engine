// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/ShaderCompiler/ShaderParser.h>

ANKI_TEST(ShaderCompiler, ShaderCompilerParser)
{
	class FilesystemInterface : public ShaderCompilerFilesystemInterface
	{
	public:
		U32 count = 0;

		Error readAllText([[maybe_unused]] CString filename, ShaderCompilerString& txt) final
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

	ShaderParser parser("filename0", &interface, {});
	ANKI_TEST_EXPECT_NO_ERR(parser.parse());

#if 0
	// Test a variant
	U32 mutationIdx = 0;
	Array<MutatorValue, 2> mutation = {{2, 4}};

	ShaderCompilerString variant;
	ANKI_TEST_EXPECT_NO_ERR(parser.generateVariant(mutation, "vert", variant));

	// printf("%s\n", variant.getSource(ShaderType::kVertex).cstr());
#endif
}
