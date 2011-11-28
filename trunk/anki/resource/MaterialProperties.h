#ifndef ANKI_RESOURCE_MATERIAL_PROPERTIES_H
#define ANKI_RESOURCE_MATERIAL_PROPERTIES_H

#include "anki/util/StringList.h"
#include "anki/util/StdTypes.h"
#include <GL/glew.h>


namespace anki {


/// Contains a few properties that other classes may use. For an explanation of
/// the variables refer to Material class documentation
class MaterialProperties
{
public:
	/// Initialize with default values
	MaterialProperties()
	{
		renderingStage = 0;
		levelsOfDetail = 1;
		shadow = false;
		blendingSfactor = GL_ONE;
		blendingDfactor = GL_ZERO;
		depthTesting = true;
		wireframe = false;
	}

	/// @name Accessors
	/// @{
	uint getRenderingStage() const
	{
		return renderingStage;
	}

	const StringList& getPasses() const
	{
		return passes;
	}

	uint getLevelsOfDetail() const
	{
		return levelsOfDetail;
	}

	bool getShadow() const
	{
		return shadow;
	}

	int getBlendingSfactor() const
	{
		return blendingSfactor;
	}

	int getBlendingDfactor() const
	{
		return blendingDfactor;
	}

	bool getDepthTesting() const
	{
		return depthTesting;
	}

	bool getWireframe() const
	{
		return wireframe;
	}
	/// @}

	/// Check if blending is enabled
	bool isBlendingEnabled() const
	{
		return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
	}

protected:
	uint renderingStage;

	StringList passes;

	uint levelsOfDetail;

	bool shadow;

	int blendingSfactor; ///< Default GL_ONE
	int blendingDfactor; ///< Default GL_ZERO

	bool depthTesting;
	
	bool wireframe;
};


} // end namespace


#endif
