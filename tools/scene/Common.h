// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_TOOLS_SCENE_MISC_H
#define ANKI_TOOLS_SCENE_MISC_H

#include <string>

/// Write to log. Don't use it directly.
void log(
	const char* file, 
	int line, 
	unsigned type,
	const char* fmt, 
	...);

// Log and errors
#define LOGI(...) log(__FILE__, __LINE__, 1, __VA_ARGS__)

#define ERROR(...) \
	do { \
		log(__FILE__, __LINE__, 2, __VA_ARGS__); \
		abort(); \
	} while(0)

#define LOGW(...) log(__FILE__, __LINE__, 3, __VA_ARGS__)

/// Replace all @a from substrings in @a str to @a to
std::string replaceAllString(
	const std::string& str, 
	const std::string& from, 
	const std::string& to);

/// From a path return only the filename
std::string getFilename(const std::string& path);

#endif
