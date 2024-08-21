// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/Shader.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// @memberof RayTracingShaders
class RayTracingHitGroup
{
public:
	Shader* m_closestHitShader = nullptr;
	Shader* m_anyHitShader = nullptr;
};

/// @memberof ShaderProgramInitInfo
class WorkGraphNodeSpecialization
{
public:
	CString m_nodeName;
	UVec3 m_maxNodeDispatchGrid;
};

/// ShaderProgram init info.
class ShaderProgramInitInfo : public GrBaseInitInfo
{
public:
	/// Option 1
	Array<Shader*, U32(ShaderType::kLastGraphics + 1)> m_graphicsShaders = {};

	/// Option 2
	Shader* m_computeShader = nullptr;

	/// Option 3
	class
	{
	public:
		WeakArray<Shader*> m_rayGenShaders;
		WeakArray<Shader*> m_missShaders;
		WeakArray<RayTracingHitGroup> m_hitGroups;
		U32 m_maxRecursionDepth = 1;
	} m_rayTracingShaders;

	/// Option 4
	class
	{
	public:
		Shader* m_shader = nullptr;
		ConstWeakArray<WorkGraphNodeSpecialization> m_nodeSpecializations;
	} m_workGraph;

	ShaderProgramInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	Bool isValid() const;
};

/// GPU program.
class ShaderProgram : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kShaderProgram;

	/// Get the shader group handles that will be used in the SBTs. The size of each handle is GpuDeviceCapabilities::m_shaderGroupHandleSize. To
	/// access a handle use:
	/// @code
	/// const U8* handleBegin = &getShaderGroupHandles()[handleIdx * devCapabilities.m_shaderGroupHandleSize];
	/// const U8* handleEnd = &getShaderGroupHandles()[(handleIdx + 1) * devCapabilities.m_shaderGroupHandleSize];
	/// @endcode
	/// The handleIdx is defined via a convention. The ray gen shaders appear first where handleIdx is in the same order as the shader in
	/// RayTracingShaders::m_rayGenShaders. Then miss shaders follow with a similar rule. Then hit groups follow.
	ConstWeakArray<U8> getShaderGroupHandles() const;

	/// Same as getShaderGroupHandles but the data live in a GPU buffer.
	Buffer& getShaderGroupHandlesGpuBuffer() const;

	ShaderTypeBit getShaderTypes() const
	{
		return m_shaderTypes;
	}

	/// Get the size of the shader. Can be an indication of the complexity of the shader.
	U32 getShaderBinarySize(ShaderType type) const
	{
		ANKI_ASSERT(!!(ShaderTypeBit(1u << type) & m_shaderTypes));
		ANKI_ASSERT(m_shaderBinarySizes[type] > 0);
		return m_shaderBinarySizes[type];
	}

	/// The pixel shader of the program has a discard.
	Bool hasDiscard() const
	{
		ANKI_ASSERT(!!(m_shaderTypes & ShaderTypeBit::kPixel));
		return m_refl.m_pixel.m_discards;
	}

	const ShaderReflection& getReflection() const
	{
		return m_refl;
	}

	PtrSize getWorkGraphMemoryRequirements() const
	{
		ANKI_ASSERT(m_workGraphScratchBufferSize);
		return m_workGraphScratchBufferSize;
	}

protected:
	Array<U32, U32(ShaderType::kCount)> m_shaderBinarySizes = {};

	PtrSize m_workGraphScratchBufferSize = 0;

	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;

	ShaderReflection m_refl;

	/// Construct.
	ShaderProgram(CString name)
		: GrObject(kClassType, name)
	{
	}

	/// Destroy.
	~ShaderProgram()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static ShaderProgram* newInstance(const ShaderProgramInitInfo& init);
};
/// @}

} // end namespace anki
