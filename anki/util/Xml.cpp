// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Xml.h>
#include <anki/util/File.h>
#include <anki/util/Logger.h>

namespace anki
{

Error XmlElement::check() const
{
	Error err = Error::NONE;
	if(m_el == nullptr)
	{
		ANKI_UTIL_LOGE("Empty element");
		err = Error::USER_DATA;
	}

	return err;
}

Error XmlElement::getText(CString& out) const
{
	ANKI_CHECK(check());
	out = (m_el->GetText()) ? CString(m_el->GetText()) : CString();
	return Error::NONE;
}

Error XmlElement::getChildElementOptional(CString name, XmlElement& out) const
{
	const Error err = check();
	if(!err)
	{
		out = XmlElement(m_el->FirstChildElement(&name[0]), m_alloc);
	}
	else
	{
		out = XmlElement();
	}

	return err;
}

Error XmlElement::getChildElement(CString name, XmlElement& out) const
{
	Error err = check();
	if(err)
	{
		out = XmlElement();
		return err;
	}

	err = getChildElementOptional(name, out);
	if(err)
	{
		return err;
	}

	if(!out)
	{
		ANKI_UTIL_LOGE("Cannot find tag: %s", &name[0]);
		err = Error::USER_DATA;
	}

	return err;
}

Error XmlElement::getNextSiblingElement(CString name, XmlElement& out) const
{
	const Error err = check();
	if(!err)
	{
		out = XmlElement(m_el->NextSiblingElement(&name[0]), m_alloc);
	}
	else
	{
		out = XmlElement();
	}

	return err;
}

Error XmlElement::getSiblingElementsCount(U32& out) const
{
	ANKI_CHECK(check());
	const tinyxml2::XMLElement* el = m_el;

	out = 0;
	do
	{
		el = el->NextSiblingElement(m_el->Name());
		++out;
	} while(el);

	out -= 1;

	return Error::NONE;
}

Error XmlElement::getAttributeTextOptional(CString name, CString& out, Bool& attribPresent) const
{
	ANKI_CHECK(check());

	const tinyxml2::XMLAttribute* attrib = m_el->FindAttribute(&name[0]);
	if(!attrib)
	{
		attribPresent = false;
		return Error::NONE;
	}

	attribPresent = true;

	const char* value = attrib->Value();
	if(value)
	{
		out = value;
	}
	else
	{
		out = CString();
	}

	return Error::NONE;
}

CString XmlDocument::XML_HEADER = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

Error XmlDocument::loadFile(CString filename, GenericMemoryPoolAllocator<U8> alloc)
{
	File file;
	ANKI_CHECK(file.open(filename, FileOpenFlag::READ));

	StringAuto text(alloc);
	ANKI_CHECK(file.readAllText(text));

	ANKI_CHECK(parse(text.toCString(), alloc));

	return Error::NONE;
}

Error XmlDocument::parse(CString xmlText, GenericMemoryPoolAllocator<U8> alloc)
{
	m_alloc = alloc;

	if(m_doc.Parse(&xmlText[0]))
	{
		ANKI_UTIL_LOGE("Cannot parse file. Reason: %s",
					   ((m_doc.GetErrorStr1() == nullptr) ? "unknown" : m_doc.GetErrorStr1()));

		return Error::USER_DATA;
	}

	return Error::NONE;
}

ANKI_USE_RESULT Error XmlDocument::getChildElementOptional(CString name, XmlElement& out) const
{
	out = XmlElement(m_doc.FirstChildElement(&name[0]), m_alloc);
	return Error::NONE;
}

ANKI_USE_RESULT Error XmlDocument::getChildElement(CString name, XmlElement& out) const
{
	ANKI_CHECK(getChildElementOptional(name, out));

	if(!out)
	{
		ANKI_UTIL_LOGE("Cannot find tag \"%s\"", &name[0]);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

} // end namespace anki
