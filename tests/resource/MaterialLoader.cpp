// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/resource/MaterialLoader.h"
#include "anki/misc/Xml.h"
#include <unordered_map>

namespace anki {

static const char* MTL = R"(
<material>
	<levelsOfDetail>3</levelsOfDetail>
	<shadow>1</shadow>

	<programs>
		<program>
			<type>vert</type>
			<inputs>
				<input>
					<name>anki_mvp</name>
					<type>mat4</type>
				</input>
				<input>
					<name>c</name>
					<type>float</type>
					<value>1.0</value>
					<const>1</const>
				</input>
				<input>
					<name>anki_msDepthRt</name>
					<type>sampler2D</type>
				</input>
				<input>
					<name>sampl</name>
					<type>sampler2D</type>
					<value>aasdfasdf/asdfasdf</value>
				</input>
				<input>
					<name>mouse</name>
					<type>float</type>
					<value>1.0</value>
					<inShadow>0</inShadow>
				</input>
				<input>
					<name>cat</name>
					<type>vec3</type>
					<value>1.0 1.0 1.0</value>
					<inShadow>1</inShadow>
				</input>
			</inputs>

			<includes><include>file.glsl</include></includes>

			<operations>
				<operation>
					<id>12</id>
					<returnType>vec3</returnType>
					<function>ha</function>
				</operation>
				<operation>
					<id>123</id>
					<returnType>vec3</returnType>
					<function>foo</function>
					<arguments>
						<argument>anki_mvp</argument>
						<argument>c</argument>
						<argument>cat</argument>
						<argument>out12</argument>
						<argument>anki_msDepthRt</argument>
					</arguments>
				</operation>
				<operation>
					<id>124</id>
					<returnType>void</returnType>
					<function>boo</function>
					<arguments>
						<argument>out123</argument>
						<argument>mouse</argument>
						<argument>sampl</argument>
					</arguments>
				</operation>
			</operations>
		</program>
	</programs>
</material>
)";

//==============================================================================
ANKI_TEST(Resource, MaterialLoader)
{
	XmlDocument doc;
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	MaterialLoader loader(alloc);

	ANKI_TEST_EXPECT_NO_ERR(doc.parse(MTL, alloc));
	ANKI_TEST_EXPECT_NO_ERR(loader.parseXmlDocument(doc));

	{
		RenderingKey key(Pass::SM, 1, false, 3);
		loader.mutate(key);
		printf("%s\n", &loader.getShaderSource(ShaderType::VERTEX)[0]);

		std::unordered_map<std::string, Bool> map;
		map["anki_mvp"] = true;
		map["c"] = false;
		map["anki_msDepthRt"] = false;
		map["sampl"] = true;
		map["mouse"] = true;
		map["cat"] = true;

		Error err = loader.iterateAllInputVariables([&](
			const MaterialLoaderInputVariable& in) -> Error
		{
			ANKI_TEST_EXPECT_EQ(map[&in.m_name.toCString()[0]], true);

			if(in.m_flags.m_inBlock)
			{
				printf("var in block: %s %d %d %d %d\n", &in.m_name[0],
					in.m_blockInfo.m_offset, in.m_blockInfo.m_arraySize,
					in.m_blockInfo.m_arrayStride,
					in.m_blockInfo.m_matrixStride);
			}

			return ErrorCode::NONE;
		});
		(void)err;
	}

	// Check block size
	/*{
		RenderingKey key(Pass::MS_FS, 1, false, 4);
		loader.mutate(key);

		ANKI_TEST_EXPECT_EQ(loader.getUniformBlockSize(),
			16 * 4 * sizeof(F32) + 4 * sizeof(F32) + 3 * sizeof(F32));
	}*/
}

} // end namespace anki
