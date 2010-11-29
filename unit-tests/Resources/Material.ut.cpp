#include <gtest/gtest.h>
#include "Material.h"
#include "RsrcPtr.h"



TEST(MaterialTests, Test)
{
	// Tests
	{
		RsrcPtr<Material> mtl;
		EXPECT_NO_THROW(mtl.loadRsrc("unit-tests/data/custom_sprog.mtl"));
		EXPECT_EQ(mtl->isBlendingEnabled(), false);
	}

	{
		RsrcPtr<Material> mtl;
		EXPECT_NO_THROW(mtl.loadRsrc("unit-tests/data/custom_sprog_skinning.mtl"));
		EXPECT_EQ(mtl->hasHwSkinning(), true);
		EXPECT_EQ(mtl->getBlendingSfactor(), GL_ONE);
		EXPECT_EQ(mtl->getBlendingDfactor(), GL_SRC_ALPHA);
		EXPECT_EQ(mtl->isBlendingEnabled(), true);
	}
}
