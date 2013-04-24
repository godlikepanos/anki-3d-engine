#ifndef RESOURCE_RESOURCE_H
#define RESOURCE_RESOURCE_H

#include "anki/resource/ResourceManager.h"
#include "anki/resource/ResourcePointer.h"
#include "anki/util/Singleton.h"

namespace anki {

#define ANKI_RESOURCE_TYPEDEFS(rsrc, name) \
	class rsrc; \
	typedef ResourceManager<rsrc> rsrc ## ResourceManager; \
	typedef Singleton<rsrc ## ResourceManager> \
		rsrc ## ResourceManagerSingleton; \
	typedef ResourcePointer<rsrc, rsrc ## ResourceManagerSingleton> name;

ANKI_RESOURCE_TYPEDEFS(TextureResource, TextureResourcePointer)
ANKI_RESOURCE_TYPEDEFS(ShaderProgramResource, ShaderProgramResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Material, MaterialResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Mesh, MeshResourcePointer)
ANKI_RESOURCE_TYPEDEFS(BucketMesh, BucketMeshResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Skeleton, SkeletonResourcePointer)
ANKI_RESOURCE_TYPEDEFS(SkelAnim, SkelAnimResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Animation, AnimationResourcePointer)
ANKI_RESOURCE_TYPEDEFS(LightRsrc, LightRsrcResourcePointer)
ANKI_RESOURCE_TYPEDEFS(ParticleEmitterResource, ParticleEmitterResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Script, ScriptResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Model, ModelResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Skin, SkinResourcePointer)
ANKI_RESOURCE_TYPEDEFS(DummyRsrc, DummyRsrcResourcePointer)

} // end namespace anki

#endif
