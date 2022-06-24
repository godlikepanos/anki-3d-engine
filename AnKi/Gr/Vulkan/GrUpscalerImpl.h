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
	F32 m_recommendedSharpness = 0.01f;
	UVec2 m_recommendedOptimalRenderSize = UVec2(MAX_U32, MAX_U32);
	UVec2 m_dynamicMaximumRenderSize = UVec2(MAX_U32, MAX_U32);
	UVec2 m_dynamicMinimumRenderSize = UVec2(MAX_U32, MAX_U32);
	GrUpscalerQualityMode m_qualityMode;
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

	NVSDK_NGX_Parameter* getParameters() const
	{
		return m_ngxParameters;
	}

	NVSDK_NGX_Handle* getFeature() const
	{
		return m_dlssFeature;
	}

	const DLSSRecommendedSettings& getRecommendedSettings() const
	{
		return m_recommendedSettings;
	}

	/// @}

private:

	// DLSS related
	Bool m_ngxInitialized = false;
	NVSDK_NGX_Parameter* m_ngxParameters = nullptr;
	NVSDK_NGX_Handle* m_dlssFeature = nullptr;
	DLSSRecommendedSettings m_recommendedSettings;

	void shutdown();

#if ANKI_DLSS
	Error initAsDLSS(const GrUpscalerInitInfo& initInfo);

	void shutdownDLSS();

	Error createDLSSFeature(const UVec2& srcRes, const UVec2& dstRes, const GrUpscalerQualityMode mode);

	void releaseDLSSFeature();

	Error queryOptimalSettings(const UVec2& displayRes, const GrUpscalerQualityMode mode,
							   DLSSRecommendedSettings& outRecommendedSettings);
#endif

	Bool isNgxInitialized() const
	{
		return m_ngxInitialized;
	}
};
/// @}

} // end namespace anki
