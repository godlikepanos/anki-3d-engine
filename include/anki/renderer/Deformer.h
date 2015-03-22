// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_DEFORMER_H
#define ANKI_RENDERER_DEFORMER_H

#include "anki/resource/Resource.h"

namespace anki {

class ShaderResource;
class SkinPatchNode;
class SkinNode;

/// @addtogroup renderer
/// @{

/// SkinPatchNode deformer
///
/// It gets a SkinPatchNode and using transform feedback it deforms its vertex 
/// attributes
class Deformer
{
public:
	Deformer()
	{
		init();
	}
	~Deformer();

	void deform(SkinNode& node, SkinPatchNode& subNode) const;

private:
	/// Shader program that deforms all attribs
	ShaderResourcePointer tfHwSkinningAllSProg;

	/// Shader program that deforms only the position attribute
	ShaderResourcePointer tfHwSkinningPosSProg;

	void init();
};

/// @}

} // end namespace

#endif
