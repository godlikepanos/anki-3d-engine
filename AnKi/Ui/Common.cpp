// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/Common.h>
#include <AnKi/Ui/UiObject.h>

namespace anki {

void UiObjectDeleter::operator()(UiObject* x)
{
	x->~UiObject();
	UiMemoryPool::getSingleton().free(x);
}

} // end namespace anki
