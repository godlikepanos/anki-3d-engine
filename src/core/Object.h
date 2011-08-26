#ifndef OBJECT_H
#define OBJECT_H

#include "util/Vec.h"


/// A class for automatic garbage collection. Cause we -the programmers- get
/// bored when it comes to deallocation. Dont even think to put as a parent an
/// object that has not created dynamically
class Object
{
	public:
		typedef Vec<Object*> Container;

		/// Calls addChild if parent is not NULL
		/// @exception Exception
		Object(Object* parent);

		/// Delete childs from the last entered to the first and update parent
		virtual ~Object();

	protected:
		/// @name Accessors
		/// @{
		const Object* getObjParent() const {return objParent;}
		Object* getObjParent() {return objParent;}
		const Container& getObjChildren() const {return objChilds;}
		Container& getObjChildren() {return objChilds;}
		/// @}

		void addChild(Object* child);
		void removeChild(Object* child);

	private:
		Object* objParent; ///< May be nullptr
		Container objChilds;
};


#endif
