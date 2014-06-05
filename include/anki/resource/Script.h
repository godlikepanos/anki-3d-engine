// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_SCRIPT_H
#define ANKI_RESOURCE_SCRIPT_H

#include <string>

namespace anki {

/// Python script resource
class Script
{
public:
	void load(const char* filename);

private:
	std::string source;
};

} // end namespace

#endif
