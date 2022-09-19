// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	ShaderPtr m_closestHitShader;
	ShaderPtr m_anyHitShader;
};

/// @memberof ShaderProgramInitInfo
class RayTracingShaders
{
public:
	WeakArray<ShaderPtr> m_rayGenShaders;
	WeakArray<ShaderPtr> m_missShaders;
	WeakArray<RayTracingHitGroup> m_hitGroups;
	U32 m_maxRecursionDepth = 1;
};

/// ShaderProgram init info.
class ShaderProgramInitInfo : public GrBaseInitInfo
{
public:
	/// Option 1
	Array<ShaderPtr, U32(ShaderType::kLastGraphics + 1)> m_graphicsShaders;

	/// Option 2
	ShaderPtr m_computeShader;

	/// Option 3
	RayTracingShaders m_rayTracingShaders;

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

	/// Get the shader group handles that will be used in the SBTs. The size of each handle is
	/// GpuDeviceCapabilities::m_shaderGroupHandleSize. To access a handle use:
	/// @code
	/// const U8* handleBegin = &getShaderGroupHandles()[handleIdx * devCapabilities.m_shaderGroupHandleSize];
	/// const U8* handleEnd = &getShaderGroupHandles()[(handleIdx + 1) * devCapabilities.m_shaderGroupHandleSize];
	/// @endcode
	/// The handleIdx is defined via a convention. The ray gen shaders appear first where handleIdx is in the same order
	/// as the shader in RayTracingShaders::m_rayGenShaders. Then miss shaders follow with a similar rule. Then hit
	/// groups follow.
	ConstWeakArray<U8> getShaderGroupHandles() const;

protected:
	/// Construct.
	ShaderProgram(GrManager* manager, CString name)
		: GrObject(manager, kClassType, name)
	{
	}

	/// Destroy.
	~ShaderProgram()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static ShaderProgram* newInstance(GrManager* manager, const ShaderProgramInitInfo& init);
};
/// @}

} // end namespace anki
