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
	UVec2 m_recommendedOptimalRenderSize = {~(0u), ~(0u)};
	UVec2 m_dynamicMaximumRenderSize = {~(0u), ~(0u)};
	UVec2 m_dynamicMinimumRenderSize = {~(0u), ~(0u)};
	DLSSQualityMode m_qualityMode;
};

class GrUpscalerImpl final : public GrUpscaler, public VulkanObject<GrUpscaler, GrUpscalerImpl>
{
public:
	GrUpscalerImpl(GrManager* manager, CString name)
		: GrUpscaler(manager, name)
		, m_ngxInitialized(false)
		, m_ngxParameters(nullptr)
		, m_dlssFeature(nullptr)
	{
	}

	~GrUpscalerImpl();

	[[nodiscard]] Error initInternal();

	Bool isNgxInitialized() const
	{
		return m_ngxInitialized;
	}

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
	Error initAsDLSS();

	void shutdown();

	void shutdownDLSS();

	Error createDLSSFeature(const UVec2& srcRes, const UVec2& dstRes, const DLSSQualityMode mode);

	void releaseDLSSFeature();

	Error queryOptimalSettings(const UVec2& displayRes, const DLSSQualityMode mode,
							   DLSSRecommendedSettings* outRecommendedSettings);

	// DLSS related
	Bool m_ngxInitialized;
	NVSDK_NGX_Parameter* m_ngxParameters;
	NVSDK_NGX_Handle* m_dlssFeature;
	DLSSRecommendedSettings m_recommendedSettings;
};
/// @}

} // end namespace anki