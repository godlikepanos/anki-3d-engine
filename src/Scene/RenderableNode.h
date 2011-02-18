#ifndef RENDERABLE_NODE_H
#define RENDERABLE_NODE_H

#include "SceneNode.h"
#include "Sphere.h"


class Vao;
class Material;


/// Abstract class that acts as an interface for the renderable objects of the scene
class RenderableNode: public SceneNode
{
	public:
		RenderableNode(const Sphere& boundingShapeLSpace, SceneNode* parent);

		virtual const Vao& getCpVao() const = 0; ///< Get color pass VAO
		virtual const Vao& getDpVao() const = 0; ///< Get depth pass VAO
		virtual uint getVertIdsNum() const = 0;  ///< Get vert ids number for rendering
		virtual const Material& getCpMtl() const = 0;  ///< Get color pass material
		virtual const Material& getDpMtl() const = 0;  ///< Get depth pass material
		const Sphere& getBoundingShapeWSpace() const {return boundingShapeWSpace;}

		/// Update the bounding shape
		virtual void moveUpdate();
		void frameUpdate() {}

	private:
		Sphere boundingShapeLSpace;
		Sphere boundingShapeWSpace;
};


#endif
