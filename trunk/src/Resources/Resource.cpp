#include "Resource.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightProps.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================

ResourceContainer<Texture>    Resource::textures;
ResourceContainer<ShaderProg> Resource::shaders;
ResourceContainer<Material>   Resource::materials;
ResourceContainer<Mesh>       Resource::meshes;
ResourceContainer<Skeleton>   Resource::skeletons;
ResourceContainer<SkelAnim>   Resource::skelAnims;
ResourceContainer<LightProps> Resource::lightProps;


//======================================================================================================================
// tryToUnoadMe                                                                                                        =
//======================================================================================================================
void Resource::tryToUnoadMe()
{
	switch(getRsrcType())
	{
		case RT_TEXTURE:
			textures.unload(static_cast<Texture*>(this));
			break;

		case RT_SHADER_PROG:
			shaders.unload(static_cast<ShaderProg*>(this));
			break;

		case RT_MATERIAL:
			materials.unload(static_cast<Material*>(this));
			break;

		case RT_MESH:
			meshes.unload(static_cast<Mesh*>(this));
			break;

		case RT_SKELETON:
			skeletons.unload(static_cast<Skeleton*>(this));
			break;

		case RT_SKEL_ANIM:
			skelAnims.unload(static_cast<SkelAnim*>(this));
			break;

		case RT_LIGHT_PROPS:
			lightProps.unload(static_cast<LightProps*>(this));
			break;

		case RT_EXTENSION:
			break;
	}
}
