// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Shader.h>

namespace anki
{

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
	ShaderPtr m_rayGenShader;
	WeakArray<ShaderPtr> m_missShaders;
	WeakArray<RayTracingHitGroup> m_hitGroups;
	U32 m_maxRecursionDepth = 1;
};

/// ShaderProgram init info.
class ShaderProgramInitInfo : public GrBaseInitInfo
{
public:
	/// Option 1
	Array<ShaderPtr, U32(ShaderType::LAST_GRAPHICS + 1)> m_graphicsShaders;

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
	static const GrObjectType CLASS_TYPE = GrObjectType::SHADER_PROGRAM;

	/// Get the shader group handles that will be used in the SBTs. The size of each handle is
	/// GpuDeviceCapabilities::m_shaderGroupHandleSize.
	ConstWeakArray<U8> getShaderGroupHandles() const;

protected:
	/// Construct.
	ShaderProgram(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~ShaderProgram()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT ShaderProgram* newInstance(GrManager* manager, const ShaderProgramInitInfo& init);
};
/// @}

} // end namespace anki
