// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Canvas.h>
#include <anki/ui/Widget.h>

namespace anki
{

Canvas::Canvas(UiInterface* interface)
	: m_interface(interface)
{
	m_rootWidget = newWidget<Widget>();

	setSize(UVec2(128u));
}

Canvas::~Canvas()
{
	if(m_rootWidget)
	{
		m_rootWidget->markForDeletion();
	}
}

void Canvas::update(F64 dt)
{
	// TODO
}

void Canvas::paint()
{
	Error err = m_rootWidget->visitTree([&](Widget& w) -> Error {
		if(m_debugDrawEnabled)
		{
			U c = 0;
			const UVec2& pos = w.getCanvasPosition();
			const UVec2& size = w.getSize();

			UVec2 lb = pos;
			UVec2 rb = pos + UVec2(size.x(), 0u);
			UVec2 rt = pos + size;
			UVec2 lt = pos + UVec2(0u, size.y());

			Array<UVec2, 2 * 4> positions;
			positions[c++] = lb;
			positions[c++] = rb;
			positions[c++] = rb;
			positions[c++] = rt;
			positions[c++] = rt;
			positions[c++] = lt;
			positions[c++] = lt;
			positions[c++] = lb;

			m_interface->drawLines(
				WeakArray<UVec2>(&positions[0], positions.getSize()), Vec4(1.0, 0.0, 0.0, 1.0), m_size);
		}

		return ErrorCode::NONE;
	});
	(void)err;

	// m_interface->
}

void Canvas::setSize(const UVec2& size)
{
	m_size = size;
	m_rootWidget->setRelativePosition(UVec2(0u));
	m_rootWidget->setSize(size);
}

} // end namespace anki
