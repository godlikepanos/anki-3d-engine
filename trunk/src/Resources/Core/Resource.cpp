#include "Resource.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightProps.h"
#include "RsrcMngr.h"


//======================================================================================================================
// tryToUnoadMe                                                                                                        =
//======================================================================================================================
void Resource::tryToUnoadMe()
{
	switch(getRsrcType())
	{
		case RT_TEXTURE:
			RsrcMngr::textures.unload(static_cast<Texture*>(this));
			break;

		case RT_SHADER_PROG:
			RsrcMngr::shaders.unload(static_cast<ShaderProg*>(this));
			break;

		case RT_MATERIAL:
			RsrcMngr::materials.unload(static_cast<Material*>(this));
			break;

		case RT_MESH:
			RsrcMngr::meshes.unload(static_cast<Mesh*>(this));
			break;

		case RT_SKELETON:
			RsrcMngr::skeletons.unload(static_cast<Skeleton*>(this));
			break;

		case RT_SKEL_ANIM:
			RsrcMngr::skelAnims.unload(static_cast<SkelAnim*>(this));
			break;

		case RT_LIGHT_PROPS:
			RsrcMngr::lightProps.unload(static_cast<LightProps*>(this));
			break;

		case RT_EXTENSION:
			break;
	}
}
