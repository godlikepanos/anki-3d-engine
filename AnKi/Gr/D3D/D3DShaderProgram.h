// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>

namespace anki {

// Forward
class GraphicsPipelineFactory;

/// @addtogroup directx
/// @{

/// Shader program implementation.
class ShaderProgramImpl final : public ShaderProgram
{
public:
	RootSignature* m_rootSignature = nullptr;

	class
	{
	public:
		GraphicsPipelineFactory* m_pipelineFactory = nullptr;
		Array<D3D12_SHADER_BYTECODE, U32(ShaderType::kPixel - ShaderType::kVertex) + 1> m_shaderCreateInfos = {};
	} m_graphics;

	class
	{
	public:
		ID3D12PipelineState* m_pipelineState = nullptr;
	} m_compute;

	class
	{
	public:
		ID3D12StateObject* m_stateObject = nullptr;
		D3D12_PROGRAM_IDENTIFIER m_progIdentifier = {};
	} m_workGraph;

	ShaderProgramImpl(CString name)
		: ShaderProgram(name)
	{
	}

	~ShaderProgramImpl();

	Error init(const ShaderProgramInitInfo& inf);

private:
	GrDynamicArray<ShaderInternalPtr> m_shaders;
};
/// @}

} // end namespace anki
