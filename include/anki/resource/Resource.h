// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef RESOURCE_RESOURCE_H
#define RESOURCE_RESOURCE_H

#include "anki/resource/ResourceManager.h"
#include "anki/resource/ResourcePointer.h"

namespace anki {

#define ANKI_RESOURCE_TYPEDEFS(rsrc_, name_) \
	class rsrc_; \
	using name_ = ResourcePointer<rsrc_, ResourceManager>;

ANKI_RESOURCE_TYPEDEFS(Animation, AnimationResourcePointer)
ANKI_RESOURCE_TYPEDEFS(TextureResource, TextureResourcePointer)
ANKI_RESOURCE_TYPEDEFS(ProgramResource, ProgramResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Material, MaterialResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Mesh, MeshResourcePointer)
ANKI_RESOURCE_TYPEDEFS(BucketMesh, BucketMeshResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Skeleton, SkeletonResourcePointer)
ANKI_RESOURCE_TYPEDEFS(SkelAnim, SkelAnimResourcePointer)
ANKI_RESOURCE_TYPEDEFS(LightRsrc, LightRsrcResourcePointer)
ANKI_RESOURCE_TYPEDEFS(ParticleEmitterResource, ParticleEmitterResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Script, ScriptResourcePointer)
ANKI_RESOURCE_TYPEDEFS(Model, ModelResourcePointer)

#undef ANKI_RESOURCE_TYPEDEFS

} // end namespace anki

#endif
