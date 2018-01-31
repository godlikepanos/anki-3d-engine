// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderCompiler.h>
#include <tests/framework/Framework.h>

ANKI_TEST(Gr, ShaderCompiler)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ShaderCompilerCache cache(alloc, "./");

	const char* SRC = R"(
void main()
{
})";

	DynamicArrayAuto<U8> bin(alloc);
	ShaderCompilerOptions options;
	options.m_outLanguage = ShaderLanguage::SPIRV;
	ANKI_TEST_EXPECT_NO_ERR(cache.compile(SRC, nullptr, options, bin));
}
