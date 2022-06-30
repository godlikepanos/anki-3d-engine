// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Math.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Different upscalers supported internally by GrUpscaler
enum class GrUpscalerType : I8
{
	INVALID = -1,
	DLSS_2 = 0
};

/// Quality preset to be used by the upscaler if available
enum class GrUpscalerQualityMode : U8
{
	DISABLED = 0,
	PERFORMANCE,
	BALANCED,
	QUALITY,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GrUpscalerQualityMode)

/// Initialization structure for the upscaler
class GrUpscalerInitInfo : public GrBaseInitInfo
{
public:
	UVec2 m_sourceImageResolution = UVec2(0, 0);
	UVec2 m_targetImageResolution = UVec2(0, 0);
	GrUpscalerType m_upscalerType = GrUpscalerType::INVALID;
	GrUpscalerQualityMode m_qualityMode = GrUpscalerQualityMode::PERFORMANCE;

	GrUpscalerInitInfo()
		: GrBaseInitInfo()
	{
	}

	GrUpscalerInitInfo(const UVec2& srcRes, const UVec2& dstRes, const GrUpscalerType type, const GrUpscalerQualityMode qualityMode)
		: m_sourceImageResolution(srcRes)
		, m_targetImageResolution(dstRes)
		, m_upscalerType(type)
		, m_qualityMode(qualityMode)
	{
	}
};

class GrUpscaler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType CLASS_TYPE = GrObjectType::GR_UPSCALER;

	GrUpscalerType getUpscalerType() const
	{
		return m_upscalerType;
	}

protected:
	/// Construct.
	GrUpscaler(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~GrUpscaler()
	{
	}

	GrUpscalerType m_upscalerType;

private:

	/// Allocate and initialize a new instance.
	static GrUpscaler* newInstance(GrManager* manager, const GrUpscalerInitInfo& initInfo);
};
/// @}

} // end namespace anki
