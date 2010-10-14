#include <algorithm>
#include "Object.h"
#include "Exception.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Object::Object(Object* parent):
	objParent(NULL)
{
	if(parent != NULL)
	{
		parent->addChild(this);
	}
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
Object::~Object()
{
	if(objParent != NULL)
	{
		objParent->removeChild(this);
	}

	// delete all children
	for(Vec<Object*>::reverse_iterator it=objChilds.rbegin(); it!=objChilds.rend(); it++)
	{
		delete *it;
	}
}


//======================================================================================================================
// addChild                                                                                                            =
//======================================================================================================================
void Object::addChild(Object* child)
{
	RASSERT_THROW_EXCEPTION(child == NULL);
	RASSERT_THROW_EXCEPTION(child->objParent != NULL); // Child already has parent

	child->objParent = this;
	objChilds.push_back(child);
}


//======================================================================================================================
// removeChild                                                                                                         =
//======================================================================================================================
void Object::removeChild(Object* child)
{
	RASSERT_THROW_EXCEPTION(child == NULL);

	Vec<Object*>::iterator it = std::find(objChilds.begin(), objChilds.end(), child);

	if(it == objChilds.end())
	{
		throw EXCEPTION("Internal error");
	}

	objChilds.erase(it);
	child->objParent = NULL;
}
