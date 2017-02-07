// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Common.h>

namespace anki
{

const CString& shaderTypeToFileExtension(ShaderType type)
{
	static const Array<CString, U(ShaderType::COUNT)> mapping = {
		{".vert.glsl", ".tc.glsl", ".te.glsl", ".geom.glsl", ".frag.glsl", ".comp.glsl"}};

	return mapping[type];
}

Error fileExtensionToShaderType(const CString& filename, ShaderType& type)
{
	type = ShaderType::COUNT;
	Error err = ErrorCode::NONE;

	// Find the shader type
	if(filename.find(".vert.glsl") != ResourceFilename::NPOS)
	{
		type = ShaderType::VERTEX;
	}
	else if(filename.find(".tc.glsl") != ResourceFilename::NPOS)
	{
		type = ShaderType::TESSELLATION_CONTROL;
	}
	else if(filename.find(".te.glsl") != ResourceFilename::NPOS)
	{
		type = ShaderType::TESSELLATION_EVALUATION;
	}
	else if(filename.find(".geom.glsl") != ResourceFilename::NPOS)
	{
		type = ShaderType::GEOMETRY;
	}
	else if(filename.find(".frag.glsl") != ResourceFilename::NPOS)
	{
		type = ShaderType::FRAGMENT;
	}
	else if(filename.find(".comp.glsl") != ResourceFilename::NPOS)
	{
		type = ShaderType::COMPUTE;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Wrong shader file format: %s", &filename[0]);
		err = ErrorCode::USER_DATA;
	}

	return err;
}

} // end namespace anki
