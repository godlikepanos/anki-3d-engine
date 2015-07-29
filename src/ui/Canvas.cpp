// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/ui/Canvas.h"
#include "anki/ui/Widget.h"

namespace anki {

//==============================================================================
Canvas::Canvas(UiInterface* interface)
	: m_interface(interface)
{
	m_rootWidget = newWidget<Widget>();

	setSize(Vec2(128.0));
}

//==============================================================================
Canvas::~Canvas()
{
	if(m_rootWidget)
	{
		m_rootWidget->markForDeletion();
	}
}

//==============================================================================
void Canvas::update(F64 dt)
{
	// TODO
}

//==============================================================================
void Canvas::paint()
{
	Error err = m_rootWidget->visitTree([&](Widget& w) -> Error
	{
		if(m_debugDrawEnabled)
		{
			U c = 0;
			const Vec2& pos = w.getCanvasPosition();
			const Vec2& size = w.getSize();

			Vec2 lb = pos / m_size * 2.0 - 1.0;
			Vec2 rb = (pos + Vec2(size.x(), 0.0)) / m_size * 2.0 - 1.0;
			Vec2 rt = (pos + size.x()) / m_size * 2.0 - 1.0;
			Vec2 lt = (pos + Vec2(0.0, size.y())) / m_size * 2.0 - 1.0;

			Array<Vec2, 2 * 4> positions;
			positions[c++] = lb;
			positions[c++] = rb;
			positions[c++] = rb;
			positions[c++] = rt;
			positions[c++] = rt;
			positions[c++] = lt;
			positions[c++] = lt;
			positions[c++] = lb;

			m_interface->drawLines(
				SArray<Vec2>(&positions[0], positions.getSize()),
				Vec4(1.0, 0.0, 0.0, 1.0));
		}

		return ErrorCode::NONE;
	});
	(void)err;

	//m_interface->
}

//==============================================================================
void Canvas::setSize(const Vec2& size)
{
	m_size = size;
	m_rootWidget->setRelativePosition(Vec2(0.0));
	m_rootWidget->setSize(size);
}

} // end namespace anki
