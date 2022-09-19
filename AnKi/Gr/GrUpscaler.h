// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Math.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/Enum.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Different upscalers supported internally by GrUpscaler
enum class GrUpscalerType : U8
{
	kDlss2 = 0,
	kCount
};

/// Quality preset to be used by the upscaler if available
enum class GrUpscalerQualityMode : U8
{
	kPerformance,
	kBalanced,
	kQuality,

	kCount
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GrUpscalerQualityMode)

/// Initialization structure for the upscaler
class GrUpscalerInitInfo : public GrBaseInitInfo
{
public:
	UVec2 m_sourceTextureResolution = UVec2(0u);
	UVec2 m_targetTextureResolution = UVec2(0u);
	GrUpscalerType m_upscalerType = GrUpscalerType::kCount;
	GrUpscalerQualityMode m_qualityMode = GrUpscalerQualityMode::kPerformance;
};

class GrUpscaler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kGrUpscaler;

	GrUpscalerType getUpscalerType() const
	{
		ANKI_ASSERT(m_upscalerType != GrUpscalerType::kCount);
		return m_upscalerType;
	}

protected:
	GrUpscalerType m_upscalerType = GrUpscalerType::kCount;

	/// Construct.
	GrUpscaler(GrManager* manager, CString name)
		: GrObject(manager, kClassType, name)
	{
	}

	/// Destroy.
	~GrUpscaler()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static GrUpscaler* newInstance(GrManager* manager, const GrUpscalerInitInfo& initInfo);
};
/// @}

} // end namespace anki
