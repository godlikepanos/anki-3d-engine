// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Shader program implementation.
class ShaderProgramImpl final : public ShaderProgram
{
public:
	ShaderProgramImpl(CString name)
		: ShaderProgram(name)
	{
	}

	~ShaderProgramImpl();

	Error init(const ShaderProgramInitInfo& inf);

	RootSignature& getRootSignature() const
	{
		ANKI_ASSERT(m_rootSignature);
		return *m_rootSignature;
	}

	ID3D12PipelineState& getComputePipelineState() const
	{
		ANKI_ASSERT(m_compute.m_pipelineState);
		return *m_compute.m_pipelineState;
	}

private:
	GrDynamicArray<ShaderPtr> m_shaders;

	RootSignature* m_rootSignature = nullptr;

	class
	{
	public:
		ID3D12PipelineState* m_pipelineState = nullptr;
	} m_compute;
};
/// @}

} // end namespace anki
