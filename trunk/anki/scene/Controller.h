#ifndef CONTROLLER_H
#define CONTROLLER_H


class SceneNode;


/// Scenegraph node controller (A)
class Controller
{
	public:
		enum ControllerType
		{ 
			CT_SKEL_ANIM_SKIN_NODE,
			CT_NUM
		};
	
		Controller(ControllerType type, SceneNode& node);
		virtual ~Controller();

		/// @name Accessors
		/// @{
		ControllerType getType() const {return type;}
		/// @}

		virtual void update(float time) = 0;

	protected:
		SceneNode& controlledNode;

	private:
		ControllerType type; ///< Once the type is set nothing can change it
};


#endif
