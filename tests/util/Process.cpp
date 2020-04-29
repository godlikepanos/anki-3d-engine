// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/Process.h>
#include <anki/util/File.h>
#include <anki/util/HighRezTimer.h>

ANKI_TEST(Util, Process)
{
	// Simple test
	if(1)
	{
		Process proc;
		ANKI_TEST_EXPECT_NO_ERR(proc.start("ls", Array<CString, 1>{{"-lt"}}, {}));
		ANKI_TEST_EXPECT_NO_ERR(proc.wait());
		HighRezTimer::sleep(1.0);

		HeapAllocator<U8> alloc(allocAligned, nullptr);
		StringAuto stdOut(alloc);
		ANKI_TEST_EXPECT_NO_ERR(proc.readFromStdout(stdOut));
		ANKI_TEST_LOGI("%s", stdOut.cstr());
	}

	// Stderr and stdOut
	if(1)
	{
		File file;
		ANKI_TEST_EXPECT_NO_ERR(file.open("process_test.sh", FileOpenFlag::WRITE));
		ANKI_TEST_EXPECT_NO_ERR(file.writeText(R"(#!/bin/bash
x=1
while [ $x -le 10 ]
do
	echo "Stdout message no $x"
	echo "Stderr message no $x" 1>&2
	sleep 1
	x=$(( $x + 1 ))
done
)"));
		file.close();

		Process proc;
		ANKI_TEST_EXPECT_NO_ERR(proc.start("bash", Array<CString, 1>{{"process_test.sh"}}, {}));
		ProcessStatus status;

		while(true)
		{
			ANKI_TEST_EXPECT_NO_ERR(proc.getStatus(status));
			if(status == ProcessStatus::NORMAL_EXIT)
			{
				break;
			}

			HeapAllocator<U8> alloc(allocAligned, nullptr);
			StringAuto stdOut(alloc);
			ANKI_TEST_EXPECT_NO_ERR(proc.readFromStdout(stdOut));
			if(stdOut.getLength())
			{
				ANKI_TEST_LOGI("%s", stdOut.cstr());
			}

			StringAuto stderrStr(alloc);
			ANKI_TEST_EXPECT_NO_ERR(proc.readFromStderr(stderrStr));
			if(stderrStr.getLength())
			{
				ANKI_TEST_LOGI("%s", stderrStr.cstr());
			}
		}

		ANKI_TEST_EXPECT_NO_ERR(proc.wait(0.0, &status));
	}
}
