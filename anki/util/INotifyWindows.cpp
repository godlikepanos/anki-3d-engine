// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/INotify.h>

namespace anki
{

Error INotify::initInternal()
{
	// TODO
	return Error::NONE;
}

void INotify::destroyInternal()
{
	// TODO
}

Error INotify::pollEvents(Bool& modified)
{
	// TODO
	modified = false;
	return Error::NONE;
}

} // end namespace anki
