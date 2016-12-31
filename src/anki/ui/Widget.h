// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>
#include <anki/util/Hierarchy.h>
#include <anki/util/BitMask.h>

namespace anki
{

/// @addtogroup ui
/// @{

/// Widget state.
enum class WidgetState : U8
{
	DEFAULT,
	HOVER,
	ACTIVATED,
	VISIBLE,
	ENABLED
};

/// UI widget.
class Widget : public UiObject, private Hierarchy<Widget>
{
	ANKI_WIDGET
	friend Hierarchy<Widget>;

public:
	Widget(Canvas* canvas);

	~Widget();

	void markForDeletion();

	Bool isMarkedForDeletion() const
	{
		return m_flags.get(MARKED_FOR_DELETION);
	}

	virtual void paint()
	{
	}

	/// Set position relatively to the parent.
	void setRelativePosition(const UVec2& pos);

	const UVec2& getRelativePosition() const
	{
		return m_posLocal;
	}

	const UVec2& getCanvasPosition() const
	{
		return m_posCanvas;
	}

	/// Set prefered size.
	void setSize(const UVec2& size);

	const UVec2& getSize() const
	{
		return m_size;
	}

	/// Set the limts of size. The widget cannot exceede those.
	void setSizeLimits(const UVec2& min, const UVec2& max);

	void addWidget(Widget* widget)
	{
		addChild(getAllocator(), widget);
	}

	void addWidgetGridLayout(Widget* widget, U colum, U row);

	Widget* getParentWidget()
	{
		return getParent();
	}

	const Widget* getParentWidget() const
	{
		return getParent();
	}

#ifdef ANKI_BUILD
	void markForRepaint()
	{
		m_flags.set(NEEDS_REPAINT, true);
	}

	Bool isMarkedForRepaint() const
	{
		return m_flags.get(NEEDS_REPAINT);
	}
#endif

private:
	enum
	{
		MARKED_FOR_DELETION = 1 << 0,
		NEEDS_REPAINT = 1 << 1
	};

	UVec2 m_minSize = UVec2(0u);
	UVec2 m_maxSize = UVec2(MAX_U32);
	UVec2 m_size = UVec2(0u);

	UVec2 m_posLocal = UVec2(0u); ///< Local space.
	UVec2 m_posCanvas = UVec2(0u); ///< World space.

	BitMask<U8> m_flags;
};
/// @}

} // end namespace anki
