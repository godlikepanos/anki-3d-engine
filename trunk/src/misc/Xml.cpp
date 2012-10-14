#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
int XmlElement::getInt() const
{
	check();
	const char* txt = getText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION(std::string("Failed to return int. Element: ")
			+ el->Value());
	}
	return std::stoi(txt);
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
		throw ANKI_EXCEPTION("Cannot find <" + name + ">");
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

} // end namespace anki
