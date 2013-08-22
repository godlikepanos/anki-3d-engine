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
		throw ANKI_EXCEPTION(std::string("Failed to return int. Element: ")
			+ el->Value());
	}
	return std::stoi(txt);
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

//==============================================================================
// XmlDocument                                                                 =
//==============================================================================

//==============================================================================
void XmlDocument::loadFile(const char* filename)
{
	File file(filename, File::OF_READ);
	std::string text;
	file.readAllText(text);

	if(doc.Parse(text.c_str()))
	{
		throw ANKI_EXCEPTION("Cannot parse file. Reason: "
			+ ((doc.GetErrorStr1() == nullptr)
			? "unknown" : doc.GetErrorStr1()));
	}
}

} // end namespace anki
