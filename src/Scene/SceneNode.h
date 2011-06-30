#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <memory>
#include <string>
#include "Math/Math.h"
#include "Core/Object.h"
#include "Collision/Obb.h"
#include "Util/Accessors.h"


class Material;
class Controller;


/// The backbone of scene. It is also an Object for memory management reasons
class SceneNode: public Object
{
	public:
		typedef Obb VisibilityCollisionShape;

		enum SceneNodeType
		{
			SNT_GHOST,
			SNT_LIGHT,
			SNT_CAMERA,
			SNT_PARTICLE_EMITTER,
			SNT_MODEL,
			SNT_SKIN,
			SNT_RENDERABLE
		};
		
		explicit SceneNode(SceneNodeType type_, bool compoundFlag, SceneNode* parent);
		virtual ~SceneNode();
		virtual void init(const char*) = 0; ///< init using a script file

		/// @name Accessors
		/// @{
		GETTER_SETTER(Transform, localTransform, getLocalTransform, setLocalTransform)
		GETTER_SETTER(Transform, worldTransform, getWorldTransform, setWorldTransform)
		GETTER_R(Transform, prevWorldTransform, getPrevWorldTransform)

		GETTER_R_BY_VAL(SceneNodeType, type, getSceneNodeType)

		const Object* getParent() const {return getObjParent();}
		Object* getParent() {return getObjParent();}
		const Object::Container& getChildren() const {return getObjChildren();}
		Object::Container& getChildren() {return getObjChildren();}

		GETTER_SETTER_BY_VAL(bool, visible, isVisible, setVisible)

		GETTER_R(std::string, name, getSceneNodeName)
		/// @}

		/// @name Updates
		/// Two separate updates for the main loop. The update happens anyway and the updateTrf only when the node is
		/// being moved
		/// @{

		/// This is called every frame
		virtual void frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/) = 0;

		/// This is called if the node moved
		virtual void moveUpdate() = 0;
		/// @}

		/// @name Mess with the local transform
		/// @{
		void rotateLocalX(float angDegrees) {getLocalTransform().getRotation().rotateXAxis(angDegrees);}
		void rotateLocalY(float angDegrees) {getLocalTransform().getRotation().rotateYAxis(angDegrees);}
		void rotateLocalZ(float angDegrees) {getLocalTransform().getRotation().rotateZAxis(angDegrees);}
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/// @}

		/// This update happens only when the object gets moved. Called only by the Scene
		void updateWorldTransform();

		void addChild(SceneNode& node) {Object::addChild(&node);}

	protected:
		std::string name;

	private:
		Transform localTransform; ///< The transformation in local space
		Transform worldTransform; ///< The transformation in world space (local combined with parent's transformation)
		Transform prevWorldTransform;

		SceneNodeType type;
		bool compoundFlag; ///< This means that the children will inherit the world transform of this node

		static uint uid; ///< Unique identifier

		/// @name Runtime info
		/// @{
		bool visible; ///< Visible by any camera
		bool moved;
		/// @}
};


#endif
