#include <gtest/gtest.h>
#include <malloc.h>
#include "UiFtFontLoader.h"


TEST(UiTests, UiFtFontLoader)
{
	struct mallinfo pre = mallinfo();
	
	{
		FT_Vector ftSize = {20, 20};
		Ui::FtFontLoader ft("engine-rsrc/ModernAntiqua.ttf", ftSize);
	}
	
	struct mallinfo post = mallinfo();
	
	EXPECT_EQ(post.uordblks - pre.uordblks, 0);
}
