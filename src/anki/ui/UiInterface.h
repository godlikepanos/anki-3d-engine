// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>

namespace anki
{

/// @addtogroup ui
/// @{

using UiImagePtr = IntrusivePtr<UiImage>;

/// Interfacing UI system with external systems.
class UiInterface
{
public:
	UiInterface(UiAllocator alloc)
		: m_alloc(alloc)
	{
	}

	virtual ~UiInterface() = default;

	UiAllocator getAllocator() const
	{
		return m_alloc;
	}

	/// @name Image related methods.
	/// @{
	virtual ANKI_USE_RESULT Error loadImage(const CString& filename, UiImagePtr& img) = 0;

	/// Create a 8bit image. Used for fonts.
	virtual ANKI_USE_RESULT Error createR8Image(const WeakArray<U8>& data, const UVec2& size, UiImagePtr& img) = 0;
	/// @}

	/// @name Misc methods.
	/// @{
	virtual ANKI_USE_RESULT Error readFile(const CString& filename, DynamicArrayAuto<U8>& data) = 0;
	/// @}

	/// @name Painting related methods.
	/// @{
	virtual void drawImage(UiImagePtr image, const Rect& uvs, const Rect& drawingRect, const UVec2& canvasSize) = 0;

	virtual void drawLines(const WeakArray<UVec2>& lines, const Color& color, const UVec2& canvasSize) = 0;
	/// @}

protected:
	UiAllocator m_alloc;
};

/// UI image interface.
class UiImage
{
	friend class IntrusivePtr<UiImage>;
	friend IntrusivePtr<UiImage>::Deleter;

public:
	UiImage(UiInterface* interface)
		: m_alloc(interface->getAllocator())
	{
	}

	virtual ~UiImage() = default;

private:
	UiAllocator m_alloc;
	Atomic<I32> m_refcount = {0};

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	UiAllocator getAllocator() const
	{
		return m_alloc;
	}
};
/// @}

} // end namespace anki
