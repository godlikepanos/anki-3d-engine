// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiInterface.h>
#include <anki/Gr.h>
#include <anki/resource/ShaderResource.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

// Forward
class GrManager;
struct Vertex;

/// @addtogroup ui
/// @{

/// Implements UiImage.
class UiImageImpl : public UiImage
{
public:
	UiImageImpl(UiInterface* i)
		: UiImage(i)
	{
	}

	TextureResourcePtr m_resource;
	TexturePtr m_texture;
};

/// Implements UiInterface.
class UiInterfaceImpl final : public UiInterface
{
public:
	UiInterfaceImpl(UiAllocator alloc);

	~UiInterfaceImpl();

	ANKI_USE_RESULT Error init(GrManager* gr, ResourceManager* rc);

	ANKI_USE_RESULT Error loadImage(const CString& filename, IntrusivePtr<UiImage>& img) override;

	ANKI_USE_RESULT Error createR8Image(
		const WeakArray<U8>& data, const UVec2& size, IntrusivePtr<UiImage>& img) override;

	ANKI_USE_RESULT Error readFile(const CString& filename, DynamicArrayAuto<U8>& data) override;

	void drawImage(UiImagePtr image, const Rect& uvs, const Rect& drawingRect, const UVec2& canvasSize) override;

	void drawLines(const WeakArray<UVec2>& positions, const Color& color, const UVec2& canvasSize) override;

	void beginRendering(CommandBufferPtr cmdb);

	void endRendering();

private:
	const U MAX_VERTS = 128;
	WeakPtr<GrManager> m_gr;
	WeakPtr<ResourceManager> m_rc;

	enum StageId
	{
		LINES,
		TEXTURED_TRIANGLES,
		COUNT
	};

	class Stage
	{
	public:
		ShaderResourcePtr m_vShader;
		ShaderResourcePtr m_fShader;
		Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_vertBuffs;
	};

	Array<Stage, StageId::COUNT> m_stages;

	class ResourceGroupKey
	{
	public:
		StageId m_stage;
		U64 m_textureId;
	};

	// Intermediate
	U8 m_timestamp = 0; ///< Local timestamp.
	CommandBufferPtr m_cmdb;
	Array<WeakPtr<Vertex>, StageId::COUNT> m_vertMappings;
	Array<U32, StageId::COUNT> m_vertCounts;
};
/// @}

} // end namespace anki
