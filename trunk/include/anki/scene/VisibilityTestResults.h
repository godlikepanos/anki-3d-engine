#ifndef ANKI_SCENE_VISIBILITY_TEST_RESULTS_H
#define ANKI_SCENE_VISIBILITY_TEST_RESULTS_H

#include "anki/scene/Common.h"

namespace anki {

// Forward
class SceneNode;

/// @addtogroup Scene
/// @{

/// Its actually a container for visible entities. It should be per frame
struct VisibilityTestResults
{
	typedef SceneVector<SceneNode*> Renderables;
	typedef SceneVector<SceneNode*> Lights;

	Renderables renderables;
	Lights lights;

	VisibilityTestResults(const SceneVector<U8>& frameAlloc)
		: renderables(frameAlloc), lights(frameAlloc)
	{}
};
/// @}

} // end namespace anki

#endif
