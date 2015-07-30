// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/ui/UiObject.h"

namespace anki {

/// @addtogroup ui
/// @{

/// Interfacing UI system with external systems.
class UiInterface
{
public:
	UiInterface(UiAllocator alloc)
		: m_alloc(alloc)
	{}

	virtual ~UiInterface() = default;

	UiAllocator getAllocator() const
	{
		return m_alloc;
	}

	/// @name Image related methods.
	/// @{
	virtual ANKI_USE_RESULT Error loadImage(
		const CString& filename, IntrusivePtr<UiImage>& img) = 0;

#if 0
	virtual ANKI_USE_RESULT Error createRgba8Image(
		const void* data, PtrSize dataSize, const Vec2& size,
		IntrusivePtr<UiImage>& img) = 0;
	/// @}
#endif

	/// @name Misc methods.
	/// @{
	virtual ANKI_USE_RESULT Error readFile(const CString& filename,
		DArrayAuto<U8>& data) = 0;
	/// @}

	/// @name Painting related methods.
	/// @{
#if 0
	virtual void drawImage(ImagePtr image, const Rect& subImageRect,
		const Rect& drawingRect) = 0;
#endif

	virtual void drawLines(const SArray<Vec2>& lines, const Color& color) = 0;
	/// @}

protected:
	UiAllocator m_alloc;
};

/// UI image interface.
class UiImage
{
public:
	UiImage(UiInterface* interface)
		: m_alloc(interface->getAllocator())
	{}

	virtual ~UiImage() = default;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	UiAllocator getAllocator() const
	{
		return m_alloc;
	}

private:
	UiAllocator m_alloc;
	Atomic<I32> m_refcount = {0};
};

using UiImagePtr = IntrusivePtr<UiImage>;
/// @}

} // end namespace anki
