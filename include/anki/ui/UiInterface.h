// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/ui/UiObject.h"
#include "anki/input/KeyCode.h"

namespace anki {

/// @addtogroup ui
/// @{

/// UI image interface.
class IImage: public UiObject
{
	// Empty.
};

/// Interfacing UI system with external systems.
class UiInterface
{
public:
	virtual ~UiInterface() = default;

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

	virtual void drawLines(const SArray<Vec2>& lines, const Color& color) = 0;
	/// @}

	/// @name Input related methods
	/// @{
	virtual void injectMouseMove(const Vec2& pos) = 0;

	virtual void injectKeyDown(KeyCode key) = 0;

	virtual void injectKeyUp(KeyCode key) = 0;

	virtual void injectMouseButtonDown(U button) = 0;
	/// @}
};
/// @}

} // end namespace anki
