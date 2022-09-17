// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrUpscaler.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/GpuMemoryManager.h>

// Ngx Sdk forward declarations
struct NVSDK_NGX_Parameter;
struct NVSDK_NGX_Handle;

namespace anki {

/// @addtogroup vulkan
/// @{

class DLSSRecommendedSettings
{
public:
	UVec2 m_optimalRenderSize = UVec2(kMaxU32);
	UVec2 m_dynamicMaximumRenderSize = UVec2(kMaxU32);
	UVec2 m_dynamicMinimumRenderSize = UVec2(kMaxU32);
};

class GrUpscalerImpl final : public GrUpscaler, public VulkanObject<GrUpscaler, GrUpscalerImpl>
{
public:
	GrUpscalerImpl(GrManager* manager, CString name)
		: GrUpscaler(manager, name)
	{
	}

	~GrUpscalerImpl();

	Error initInternal(const GrUpscalerInitInfo& initInfo);

	/// @name DLSS data accessors
	/// @{
#if ANKI_DLSS
	NVSDK_NGX_Parameter& getParameters() const
	{
		return *m_ngxParameters;
	}

	NVSDK_NGX_Handle& getFeature() const
	{
		return *m_dlssFeature;
	}

	const DLSSRecommendedSettings& getRecommendedSettings() const
	{
		return m_recommendedSettings;
	}
#endif
	/// @}

private:
#if ANKI_DLSS
	NVSDK_NGX_Parameter* m_ngxParameters = nullptr;
	NVSDK_NGX_Handle* m_dlssFeature = nullptr;
	DLSSRecommendedSettings m_recommendedSettings;
	Bool m_ngxInitialized = false;

	Error initDlss(const GrUpscalerInitInfo& initInfo);

	void destroyDlss();

	Error createDlssFeature(const UVec2& srcRes, const UVec2& dstRes, const GrUpscalerQualityMode mode);
#endif
};
/// @}

} // end namespace anki
