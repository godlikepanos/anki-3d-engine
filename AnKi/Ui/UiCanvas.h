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
	void resize(UVec2 size)
	{
		ANKI_ASSERT(size > 0);
		m_size = size;
	}

	UVec2 getSize() const
	{
		return m_size;
	}

	Vec2 getSizef() const
	{
		return Vec2(m_size);
	}

	/// @name building commands. The order matters.
	/// @{
	virtual void handleInput();

	void beginBuilding();

	ImFont* addFont(CString fname);

	/// Merge fonts into one
	ImFont* addFonts(ConstWeakArray<CString> fnames);

	void endBuilding();

	template<typename TFunc>
	void visitTexturesForUpdate(TFunc func)
	{
		for(const UploadRequest& req : m_texturesPendingUpload)
		{
			func(*req.m_texture, req.m_isNew);
		}
	}

	void appendNonGraphicsCommands(CommandBuffer& cmdb) const;

	void appendGraphicsCommands(CommandBuffer& cmdb) const;
	/// @}

private:
	ImGuiContext* m_imCtx = nullptr;
	ImFont* m_defaultFont = nullptr;
	UVec2 m_size;

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
		UiDynamicArray<GenericResourcePtr> m_resources;
	};

	UiHashMap<U64, FontCacheEntry> m_fontCache;

	class UploadRequest
	{
	public:
		Texture* m_texture = nullptr;
		TextureRect m_rect;
		UiDynamicArray<U8> m_data;
		Bool m_isNew = true;
	};

	UiDynamicArray<UploadRequest> m_texturesPendingUpload;

	Error init(UVec2 size);
};
/// @}

} // end namespace anki
