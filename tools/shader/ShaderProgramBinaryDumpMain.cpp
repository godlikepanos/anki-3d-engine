// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramCompiler.h>

using namespace anki;

static const char* USAGE = R"(Dump the shader binary to stdout
Usage: %s in_file
)";

Error dump(const char* fname)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	ShaderProgramBinaryWrapper bin(alloc);
	ANKI_CHECK(bin.deserializeFromFile(fname));

	StringAuto txt(alloc);
	dumpShaderProgramBinary(bin.getBinary(), txt);

	printf("%s\n", txt.cstr());

	return Error::NONE;
}

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		ANKI_LOGE(USAGE, argv[0]);
		return 1;
	}

	const Error err = dump(argv[1]);
	if(err)
	{
		ANKI_LOGE("Can't dump due to an error. Bye");
		return 1;
	}

	return 0;
}
