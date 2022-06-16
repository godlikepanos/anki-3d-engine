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

enum class DLSSQualityMode : U8
{
	DISABLED = 0,
	PERFORMANCE,
	BALANCED,
	QUALITY,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DLSSQualityMode)

class DLSSCtxInitInfo
{
public:
	UVec2 m_srcRes = {0, 0};
	UVec2 m_dstRes = {0, 0};
	DLSSQualityMode m_mode = DLSSQualityMode::PERFORMANCE;
};

class DLSSCtx : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType CLASS_TYPE = GrObjectType::DLSS_CTX;

	void upscale(CommandBufferPtr cmdb, const TextureViewPtr& srcRt, const TextureViewPtr& dstRt,
				 const TextureViewPtr& mvRt, const TextureViewPtr& depthRt, const TextureViewPtr& exposure,
				 const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& mVScale);

protected:
	/// Construct.
	DLSSCtx(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~DLSSCtx()
	{
	}

private:
	/// Allocate and initialize a new instance.
	static [[nodiscard]] DLSSCtx* newInstance(GrManager* manager, const DLSSCtxInitInfo& init);
};
/// @}

} // end namespace anki
