// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Widget.h>
#include <anki/ui/Canvas.h>

namespace anki
{

Widget::Widget(Canvas* canvas)
	: UiObject(canvas)
{
	markForRepaint();
}

Widget::~Widget()
{
	ANKI_ASSERT(isMarkedForDeletion());
	Hierarchy<Widget>::destroy(getAllocator());
}

void Widget::markForDeletion()
{
	if(!isMarkedForDeletion())
	{
		m_flags.set(MARKED_FOR_DELETION);
		getCanvas().getMarkedForDeletionCount().fetchAdd(1);
	}

	Error err = visitChildren([](Widget& obj) -> Error {
		obj.markForDeletion();
		return ErrorCode::NONE;
	});

	(void)err;
}

void Widget::setRelativePosition(const UVec2& pos)
{
	m_posLocal = pos;
	markForRepaint();
}

void Widget::setSizeLimits(const UVec2& min, const UVec2& max)
{
	m_minSize = min;
	m_maxSize = max;
	markForRepaint();
}

void Widget::setSize(const UVec2& size)
{
	m_size.x() = clamp(size.x(), m_minSize.x(), m_maxSize.x());
	m_size.y() = clamp(size.y(), m_minSize.y(), m_maxSize.y());
	markForRepaint();
}

} // end namespace anki
