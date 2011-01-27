#ifndef VISIBILITY_TESTER_H
#define VISIBILITY_TESTER_H

#include <deque>
#include "Properties.h"


class Camera;
class Scene;
class SceneRenderable;


class VisibilityTester
{
	public:
		/// Types
		template<typename Type>
		class Types
		{
			public:
				typedef std::deque<Type*> Container;
				typedef typename Container::iterator Iterator;
				typedef typename Container::const_iterator ConstIterator;
		};

		VisibilityTester(const Scene& scene);

		/// @name Accessors
		/// @{
		GETTER_R(Types<SceneRenderable>::Container, renderables, getRenderables)
		GETTER_R(Types<SceneRenderable>::Container, lights, getLights)
		/// @}

		void test(const Camera& cam);

	private:
		const Scene& scene; ///< Know your father

		Types<SceneRenderable>::Container renderables;
		Types<SceneRenderable>::Container lights;
};


#endif
