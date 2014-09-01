// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/misc/Xml.h"
#include "anki/util/StringList.h"
#include "anki/util/File.h"

namespace anki {

//==============================================================================
// XmlElement                                                                  =
//==============================================================================

//==============================================================================
I XmlElement::getInt() const
{
	check();
	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return int. Element: %s", el->Value());
	}
	return std::stoi(txt);
}

//==============================================================================
F64 XmlElement::getFloat() const
{
	check();
	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return float. Element: %s", 
			el->Value());
	}
	return std::stof(txt);
}

//==============================================================================V
Vector<F64> XmlElement::getFloats() const
{
	check();
	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return floats. Element: %s",
			el->Value());
	}

	StringList list = StringList::splitString(txt, ' ');
	Vector<F64> out;
	out.resize(list.size());

	try
	{
		for(U i = 0; i < out.size(); i++)
		{
			out[i] = std::stof(list[i]);
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Found non-float element for Mat4");
	}

	return out;
}

//==============================================================================
Mat4 XmlElement::getMat4() const
{
	check();

	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return Mat4");
	}

	StringList list = StringList::splitString(txt, ' ');
	if(list.size() != 16)
	{
		throw ANKI_EXCEPTION("Expecting 16 elements for Mat4");
	}

	Mat4 out;

	try
	{
		for(U i = 0; i < 16; i++)
		{
			out[i] = std::stof(list[i]);
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Found non-float element for Mat4");
	}

	return out;
}

//==============================================================================
Vec3 XmlElement::getVec3() const
{
	check();

	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return Vec3");
	}

	StringList list = StringList::splitString(txt, ' ');
	if(list.size() != 3)
	{
		throw ANKI_EXCEPTION("Expecting 3 elements for Vec3");
	}

	Vec3 out;

	try
	{
		for(U i = 0; i < 3; i++)
		{
			out[i] = std::stof(list[i]);
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Found non-float element for Vec3");
	}

	return out;
}
//==============================================================================
Vec4 XmlElement::getVec4() const
{
	check();

	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return Vec4");
	}

	StringList list = StringList::splitString(txt, ' ');
	if(list.size() != 4)
	{
		throw ANKI_EXCEPTION("Expecting 3 elements for Vec4");
	}

	Vec4 out;

	try
	{
		for(U i = 0; i < 4; i++)
		{
			out[i] = std::stof(list[i]);
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Found non-float element for Vec4");
	}

	return out;
}

//==============================================================================
XmlElement XmlElement::getChildElementOptional(const char* name) const
{
	check();
	XmlElement out;
	out.el = el->FirstChildElement(name);
	return out;
}

//==============================================================================
XmlElement XmlElement::getChildElement(const char* name) const
{
	check();
	const XmlElement out = getChildElementOptional(name);
	if(!out)
	{
		throw ANKI_EXCEPTION("Cannot find tag %s", name);
	}
	return out;
}

//==============================================================================
XmlElement XmlElement::getNextSiblingElement(const char* name) const
{
	check();
	XmlElement out;
	out.el = el->NextSiblingElement(name);
	return out;
}

//==============================================================================
// XmlDocument                                                                 =
//==============================================================================

//==============================================================================
void XmlDocument::loadFile(const char* filename, StackAllocator<U8>& alloc)
{
	File file(filename, File::OpenFlag::READ);
	std::string text;
	file.readAllText(text);

	if(doc.Parse(text.c_str()))
	{
		throw ANKI_EXCEPTION("Cannot parse file. Reason: %s",
			((doc.GetErrorStr1() == nullptr)
			? "unknown" : doc.GetErrorStr1()));
	}
}

} // end namespace anki
