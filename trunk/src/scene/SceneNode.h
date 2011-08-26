#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <memory>
#include <string>
#include "m/Math.h"
#include "core/Object.h"
#include "cln/Obb.h"
#include "util/Accessors.h"


class Material;
class Controller;


/// The backbone of scene. It is also an Object for memory management reasons
class SceneNode: public Object
{
	public:
		typedef cln::Obb VisibilityCollisionShape;

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
		
		enum SceneNodeFlags
		{
			SNF_NONE = 0,
			SNF_VISIBLE = 1,
			SNF_ACTIVE = 2,
			SNF_MOVED = 4
		};

		explicit SceneNode(SceneNodeType type, bool inheritParentTrfFlag,
			SceneNode* parent);
		virtual ~SceneNode();
		virtual void init(const char*) = 0; ///< init using a script file

		/// @name Accessors
		/// @{
		GETTER_SETTER(Transform, localTransform, getLocalTransform,
			setLocalTransform)
		GETTER_SETTER(Transform, worldTransform, getWorldTransform,
			setWorldTransform)
		GETTER_R(Transform, prevWorldTransform, getPrevWorldTransform)

		GETTER_R_BY_VAL(SceneNodeType, type, getSceneNodeType)

		const Object* getParent() const {return getObjParent();}
		Object* getParent() {return getObjParent();}
		const Object::Container& getChildren() const {return getObjChildren();}
		Object::Container& getChildren() {return getObjChildren();}

		GETTER_R(std::string, name, getSceneNodeName)
		/// @}

		/// @name Flag manipulation
		/// @{
		void enableFlag(SceneNodeFlags flag, bool enable = true);
		void disableFlag(SceneNodeFlags flag) {enableFlag(flag, false);}
		bool isFlagEnabled(SceneNodeFlags flag) const;
		ulong getFlags() const {return flags;}
		/// @}

		/// @name Updates
		/// Two separate updates for the main loop. The update happens anyway
		/// and the updateTrf only when the node is being moved
		/// @{

		/// This is called every frame
		virtual void frameUpdate(float /*prevUpdateTime*/,
			float /*crntTime*/) {}

		/// This is called if the node moved
		virtual void moveUpdate() {}
		/// @}

		/// @name Mess with the local transform
		/// @{
		void rotateLocalX(float angDegrees);
		void rotateLocalY(float angDegrees);
		void rotateLocalZ(float angDegrees);
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/// @}

		/// This update happens only when the object gets moved. Called only by
		/// the Scene
		void updateWorldTransform();

		void addChild(SceneNode& node) {Object::addChild(&node);}

	protected:
		std::string name;

	private:
		Transform localTransform; ///< The transformation in local space
		/// The transformation in world space (local combined with parent's
		/// transformation)
		Transform worldTransform;
		Transform prevWorldTransform;

		SceneNodeType type;
		/// This means that the the node will inherit the world transform of
		/// its parent (if there is one) and it will not take into account its
		/// local transform at all
		bool inheritParentTrfFlag;

		static uint uid; ///< Unique identifier

		ulong flags; ///< The state flags
};


inline void SceneNode::rotateLocalX(float angDegrees)
{
	getLocalTransform().getRotation().rotateXAxis(angDegrees);
}


inline void SceneNode::rotateLocalY(float angDegrees)
{
	getLocalTransform().getRotation().rotateYAxis(angDegrees);
}


inline void SceneNode::rotateLocalZ(float angDegrees)
{
	getLocalTransform().getRotation().rotateZAxis(angDegrees);
}


inline void SceneNode::enableFlag(SceneNodeFlags flag, bool enable)
{
	if(enable)
	{
		flags |= flag;
	}
	else
	{
		flags &= ~flag;
	}
}


inline bool SceneNode::isFlagEnabled(SceneNodeFlags flag) const
{
	return flags & flag;
}


#endif
