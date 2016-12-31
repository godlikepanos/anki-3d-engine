// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Common.h"
#include <cstdarg>

#define TERMINAL_COL_INFO "\033[0;32m"
#define TERMINAL_COL_ERROR "\033[1;31m"
#define TERMINAL_COL_WARNING "\033[0;33m"
#define TERMINAL_COL_RESET "\033[0m"

void log(const char* file, int line, unsigned type, const char* fmt, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	switch(type)
	{
	case 1:
		fprintf(stdout, TERMINAL_COL_INFO "[I] %s (%s:%d)\n" TERMINAL_COL_RESET, buffer, file, line);
		break;
	case 2:
		fprintf(stderr, TERMINAL_COL_ERROR "[E] %s (%s:%d)\n" TERMINAL_COL_RESET, buffer, file, line);
		break;
	case 3:
		fprintf(stderr, TERMINAL_COL_WARNING "[W] %s (%s:%d)\n" TERMINAL_COL_RESET, buffer, file, line);
		break;
	};
}

std::string replaceAllString(const std::string& str, const std::string& from, const std::string& to)
{
	if(from.empty())
	{
		return str;
	}

	std::string out = str;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != std::string::npos)
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return out;
}

std::string getFilename(const std::string& path)
{
	std::string out;

	const size_t last = path.find_last_of("/");
	if(std::string::npos != last)
	{
		out.insert(out.end(), path.begin() + last + 1, path.end());
	}
	else
	{
		out = path;
	}

	return out;
}
