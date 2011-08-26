#ifndef LINUXMALINFO_H
#define LINUXMALINFO_H


#if defined(PLATFORM_LINUX)

#	include <malloc.h>

	typedef struct mallinfo Mallinfo;

	inline Mallinfo GetMallInfo()
	{
		return mallinfo();
	}

	inline void printMallInfo(const Mallinfo& minfo)
	{
		PRINT("used:" << minfo.uordblks << " free:" << minfo.fordblks << 
			" total:" << minfo.arena);
	}

	inline void printMallInfoDiff(const Mallinfo& prev, const Mallinfo& now)
	{
		Mallinfo diff;
		diff.uordblks = now.uordblks-prev.uordblks;
		diff.fordblks = now.fordblks-prev.fordblks;
		diff.arena = now.arena-prev.arena;
		printMallInfo(diff);
	}

#	define MALLINFO_BEGIN Mallinfo __m__ = GetMallInfo();

#	define MALLINFO_END printMallInfoDiff(__m__, GetMallInfo());

#endif

#endif
