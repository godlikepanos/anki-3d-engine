#ifndef SCENE_RENDERABLE_H
#define SCENE_RENDERABLE_H

#include "Math.h"


class Vao;
class Material;


/// Abstract class that acts as an interface for the renderable objects of the scene
class SceneRenderable
{
	public:
		virtual const Vao& getCpVao() const = 0; ///< Get color pass VAO
		virtual const Vao& getDpVao() const = 0; ///< Get depth pass VAO
		virtual uint getVertIdsNum() const = 0;  ///< Get vert ids number for rendering
		virtual const Material& getCpMtl() const = 0;  ///< Get color pass material
		virtual const Material& getDpMtl() const = 0;  ///< Get depth pass material
		virtual const Transform& getWorldTransform() const = 0; ///< Get the world transformation
};


#endif
