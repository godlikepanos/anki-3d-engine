// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/ui/UiObject.h"

namespace anki {

/// @addtogroup ui
/// @{

/// UI image interface.
class IImage: public UiObject
{
public:
	IImage(Canvas* canvas)
		: UiObject(canvas)
	{}
};

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

#if 0
	/// @name Image related methods.
	/// @{
	virtual ANKI_USE_RESULT Error loadImage(
		const CString& filename, ImagePtr& img) = 0;

	virtual ANKI_USE_RESULT Error createRgba8Image(
		const void* data, PtrSize dataSize, const Vec2& size,
		ImagePtr& img) = 0;
	/// @}

	/// @name Misc methods.
	/// @{
	virtual ANKI_USE_RESULT Error readFile(const CString& filename,
		DArrayAuto<U8>& data) = 0;
	/// @}

	/// @name Painting related methods.
	/// @{
	virtual void drawImage(ImagePtr image, const Rect& subImageRect,
		const Rect& drawingRect) = 0;

	virtual void drawText(const CString& text, const Rect& drawingRect,
		const Color& color) = 0;
#endif

	virtual void drawLines(const SArray<Vec2>& lines, const Color& color) = 0;
	/// @}

protected:
	UiAllocator m_alloc;
};
/// @}

} // end namespace anki
