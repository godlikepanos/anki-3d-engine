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
enum class UpscalerType : I8
{
	INVALID = -1,
	DLSS_2 = 0
};

enum class DLSSQualityMode : U8
{
	DISABLED = 0,
	PERFORMANCE,
	BALANCED,
	QUALITY,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DLSSQualityMode)

/// Use this for correctly create a GrUpscalerInitInfo for DLSS
class DLSSInitInfo
{
public:
	DLSSQualityMode m_mode = DLSSQualityMode::PERFORMANCE;
};

/// Initialization structure for the upscaler
class GrUpscalerInitInfo : public GrBaseInitInfo
{
public:
	UVec2 m_sourceImageResolution = UVec2(0, 0);
	UVec2 m_targetImageResolution = UVec2(0, 0);
	UpscalerType m_upscalerType = UpscalerType::INVALID;
	union
	{
		DLSSInitInfo m_dlssInitInfo;
	};

	GrUpscalerInitInfo()
		: GrBaseInitInfo()
	{
	}

	GrUpscalerInitInfo(const UVec2& srcRes, const UVec2& dstRes, const DLSSInitInfo& info)
		: m_sourceImageResolution(srcRes)
		, m_targetImageResolution(dstRes)
		, m_upscalerType(UpscalerType::DLSS_2)
		, m_dlssInitInfo(info)
	{
	}
};

class GrUpscaler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType CLASS_TYPE = GrObjectType::GR_UPSCALER;

	UpscalerType getUpscalerType() const
	{
		return m_initInfo.m_upscalerType;
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

	GrUpscalerInitInfo m_initInfo;

private:
	[[nodiscard]] Error init(const GrUpscalerInitInfo& init);

	/// Allocate and initialize a new instance.
	static GrUpscaler* newInstance(GrManager* manager, const GrUpscalerInitInfo& init);
};
/// @}

} // end namespace anki