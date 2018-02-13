// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Assert.h>
#include <anki/util/System.h>
#include <cstdlib>
#include <cstdio>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki
{

#if ANKI_EXTRA_CHECKS

void akassert(const char* exprTxt, const char* file, int line, const char* func)
{
#	if ANKI_OS == ANKI_OS_ANDROID
	__android_log_print(ANDROID_LOG_ERROR, "AnKi", "(%s:%d %s) Assertion failed: %s", file, line, func, exprTxt);
#	else
#		if ANKI_OS == ANKI_OS_LINUX
	if(runningFromATerminal())
		fprintf(stderr, "\033[1;31m(%s:%d %s) Assertion failed: %s\033[0m\n", file, line, func, exprTxt);
	else
#		endif
		fprintf(stderr, "(%s:%d %s) Assertion failed: %s\n", file, line, func, exprTxt);
#	endif

	class BW : public BackTraceWalker
	{
	public:
		BW()
			: BackTraceWalker(10)
		{
		}

		U m_c = 0;

		void operator()(const char* symbol)
		{
			printf("%.2u: %s\n", unsigned(m_c++), symbol);
		}
	};

	BW bw;
	printf("Backtrace:\n");
	bw.exec();

	abort();
}

#endif

} // end namespace anki
