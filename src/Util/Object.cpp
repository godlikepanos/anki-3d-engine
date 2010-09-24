#include <algorithm>
#include "Object.h"


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
	DEBUG_ERR(child == NULL);
	DEBUG_ERR(child->objParent != NULL); // Child already has parent

	child->objParent = this;
	objChilds.push_back(child);
}


//======================================================================================================================
// removeChild                                                                                                         =
//======================================================================================================================
void Object::removeChild(Object* child)
{
	DEBUG_ERR(child == NULL);

	Vec<Object*>::iterator it = std::find(objChilds.begin(), objChilds.end(), child);

	if(it == objChilds.end())
	{
		ERROR("Internal error");
		return;
	}

	objChilds.erase(it);
	child->objParent = NULL;
}
