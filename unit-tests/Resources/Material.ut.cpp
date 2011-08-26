#include <gtest/gtest.h>
#include "rsrc/Material.h"aterial.h"
#include "rsrc/RsrcPtr.h"


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
		EXPECT_EQ(mtl->getBlendingSfactor(), GL_ONE);
		EXPECT_EQ(mtl->getBlendingDfactor(), GL_SRC_ALPHA);
		EXPECT_EQ(mtl->isBlendingEnabled(), true);
	}
	
	{
		RsrcPtr<Material> mtl;
		EXPECT_ANY_THROW(mtl.loadRsrc("unit-tests/data/bool_err.mtl"));
	}

	{
		RsrcPtr<Material> mtl;
		EXPECT_NO_THROW(mtl.loadRsrc("unit-tests/data/complex.mtl"));
		EXPECT_EQ(mtl->getUserDefinedVars().size(), 6);
		Vec3 tmp = mtl->getUserDefinedVars()[3].get<Vec3>();
		EXPECT_EQ(tmp, Vec3(1.0, 2.0, -0.8));
	}
}
