// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/Common.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Resource/GenericResource.h>
#include <AnKi/Gr/Sampler.h>

namespace anki {

/// @addtogroup ui
/// @{

/// UI canvas. It's more of a context.
class UiCanvas : public UiObject
{
	friend class UiManager;

public:
	UiCanvas() = default;

	~UiCanvas();

	/// Resize canvas.
	void resize(U32 width, U32 height)
	{
		ANKI_ASSERT(width > 0 && height > 0);
		m_width = width;
		m_height = height;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	/// @name building commands. The order matters.
	/// @{
	virtual void handleInput();

	void beginBuilding();

	ImFont* addFont(CString fname);

	void endBuilding();

	template<typename TFunc>
	void visitTexturesForUpdate(TFunc func)
	{
		for(const auto& pair : m_texturesPendingUpload)
		{
			func(*pair.first->GetTexID().m_texture, pair.second);
		}
	}

	void appendNonGraphicsCommands(CommandBuffer& cmdb) const;

	void appendGraphicsCommands(CommandBuffer& cmdb) const;
	/// @}

private:
	ImGuiContext* m_imCtx = nullptr;
	ImFont* m_defaultFont = nullptr;
	U32 m_width;
	U32 m_height;

	enum ShaderType
	{
		kNoTex,
		kRgbaTex,
		kShaderCount
	};

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, kShaderCount> m_grProgs;
	SamplerPtr m_linearLinearRepeatSampler;
	SamplerPtr m_nearestNearestRepeatSampler;

	class FontCacheEntry
	{
	public:
		ImFont* m_font = nullptr;
		GenericResourcePtr m_resource;
	};

	UiHashMap<CString, FontCacheEntry> m_fontCache;

	UiDynamicArray<std::pair<const ImTextureData*, Bool>> m_texturesPendingUpload;

	Error init(U32 width, U32 height);
};
/// @}

} // end namespace anki
