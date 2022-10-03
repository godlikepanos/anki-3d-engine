// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

static void createBashScript(CString code)
{
	File file;

	ANKI_TEST_EXPECT_NO_ERR(file.open("process_test.sh", FileOpenFlag::kWrite));
	ANKI_TEST_EXPECT_NO_ERR(file.writeTextf("#!/bin/bash\n%s\n", code.cstr()));
}

} // end namespace anki

ANKI_TEST(Util, Process)
{
	// Simple test
	if(1)
	{
		createBashScript(R"(
echo Hello from script
exit 6
)");

		Process proc;
		ANKI_TEST_EXPECT_NO_ERR(proc.start("bash", Array<CString, 1>{{"process_test.sh"}}, {}));
		ProcessStatus status;
		I32 exitCode;
		ANKI_TEST_EXPECT_NO_ERR(proc.wait(-1.0, &status, &exitCode));

		ANKI_TEST_EXPECT_EQ(status, ProcessStatus::kNotRunning);
		ANKI_TEST_EXPECT_EQ(exitCode, 6);

		// Get stuff again, don't wait this time
		exitCode = 0;
		ANKI_TEST_EXPECT_NO_ERR(proc.wait(0.0, &status, &exitCode));
		ANKI_TEST_EXPECT_EQ(status, ProcessStatus::kNotRunning);
		ANKI_TEST_EXPECT_EQ(exitCode, 6);

		HeapMemoryPool pool(allocAligned, nullptr);
		StringRaii stdOut(&pool);
		ANKI_TEST_EXPECT_NO_ERR(proc.readFromStdout(stdOut));
		ANKI_TEST_EXPECT_EQ(stdOut, "Hello from script\n");
	}

	// Stderr and stdOut
	if(1)
	{
		createBashScript(R"(
x=1
while [ $x -le 10 ]
do
	echo "Stdout message no $x"
	echo "Stderr message no $x" 1>&2
	sleep 1
	x=$(( $x + 1 ))
done
)");

		Process proc;
		ANKI_TEST_EXPECT_NO_ERR(proc.start("bash", Array<CString, 1>{{"process_test.sh"}}, {}));
		ProcessStatus status;

		ANKI_TEST_EXPECT_NO_ERR(proc.getStatus(status));
		ANKI_TEST_EXPECT_EQ(status, ProcessStatus::kRunning);

		while(true)
		{
			ANKI_TEST_EXPECT_NO_ERR(proc.getStatus(status));
			if(status == ProcessStatus::kNotRunning)
			{
				break;
			}

			HeapMemoryPool pool(allocAligned, nullptr);
			StringRaii stdOut(&pool);
			ANKI_TEST_EXPECT_NO_ERR(proc.readFromStdout(stdOut));
			if(stdOut.getLength())
			{
				ANKI_TEST_LOGI("%s", stdOut.cstr());
			}

			StringRaii stderrStr(&pool);
			ANKI_TEST_EXPECT_NO_ERR(proc.readFromStderr(stderrStr));
			if(stderrStr.getLength())
			{
				ANKI_TEST_LOGI("%s", stderrStr.cstr());
			}
		}

		ANKI_TEST_EXPECT_NO_ERR(proc.wait(0.0, &status));
	}

	// Read after complete wait & env
	if(1)
	{
		createBashScript(R"(
echo $ENV_VAR
sleep 1
)");

		Process proc;
		ANKI_TEST_EXPECT_NO_ERR(
			proc.start("bash", Array<CString, 1>{{"process_test.sh"}}, Array<CString, 1>{{"ENV_VAR=Lala"}}));

		ANKI_TEST_EXPECT_NO_ERR(proc.wait());

		HighRezTimer::sleep(0.5_sec); // Wait a bit more for good measure

		HeapMemoryPool pool(allocAligned, nullptr);
		StringRaii stdOut(&pool);
		ANKI_TEST_EXPECT_NO_ERR(proc.readFromStdout(stdOut));
		ANKI_TEST_EXPECT_EQ(stdOut, "Lala\n");
	}

	ANKI_TEST_EXPECT_NO_ERR(removeFile("process_test.sh"));
}
