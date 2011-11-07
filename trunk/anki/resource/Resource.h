#ifndef RESOURCE_RESOURCE_H
#define RESOURCE_RESOURCE_H

#include "anki/resource/ResourceManager.h"
#include "anki/resource/ResourcePointer.h"
#include "anki/util/Singleton.h"


namespace anki {


#define ANKI_RESOURCE_TYPEDEFS(x) \
	class x; \
	typedef ResourceManager<x> x ## ResourceManager; \
	typedef Singleton<x ## ResourceManager> x ## ResourceManagerSingleton; \
	typedef ResourcePointer<x, x ## ResourceManagerSingleton> \
		x ## ResourcePointer;


ANKI_RESOURCE_TYPEDEFS(Texture)
ANKI_RESOURCE_TYPEDEFS(ShaderProgram)
ANKI_RESOURCE_TYPEDEFS(Material)
ANKI_RESOURCE_TYPEDEFS(Mesh)
ANKI_RESOURCE_TYPEDEFS(Skeleton)
ANKI_RESOURCE_TYPEDEFS(SkelAnim)
ANKI_RESOURCE_TYPEDEFS(LightRsrc)
ANKI_RESOURCE_TYPEDEFS(ParticleEmitterRsrc)
ANKI_RESOURCE_TYPEDEFS(Script)
ANKI_RESOURCE_TYPEDEFS(Model)
ANKI_RESOURCE_TYPEDEFS(Skin)
ANKI_RESOURCE_TYPEDEFS(DummyRsrc)


} // end namespace


#endif
