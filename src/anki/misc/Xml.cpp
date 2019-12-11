// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/misc/Xml.h>
#include <anki/util/File.h>
#include <anki/util/Logger.h>

namespace anki
{

ANKI_USE_RESULT Error XmlElement::check() const
{
	Error err = Error::NONE;
	if(m_el == nullptr)
	{
		ANKI_MISC_LOGE("Empty element");
		err = Error::USER_DATA;
	}
	return err;
}

Error XmlElement::getText(CString& out) const
{
	Error err = check();
	if(!err && m_el->GetText())
	{
		out = CString(m_el->GetText());
	}
	else
	{
		out = CString();
	}

	return err;
}

Error XmlElement::getMat3(Mat3& out) const
{
	DynamicArrayAuto<F32> arr(m_alloc);
	Error err = getNumbers(arr);

	if(!err && arr.getSize() != 9)
	{
		ANKI_MISC_LOGE("Expecting 9 elements for Mat3");
		err = Error::USER_DATA;
	}

	if(!err)
	{
		for(U32 i = 0; i < 9 && !err; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_MISC_LOGE("Failed to return Mat3. Element: %s", m_el->Value());
	}

	return err;
}

Error XmlElement::getMat4(Mat4& out) const
{
	DynamicArrayAuto<F32> arr(m_alloc);
	Error err = getNumbers(arr);

	if(!err && arr.getSize() != 16)
	{
		ANKI_MISC_LOGE("Expecting 16 elements for Mat4");
		err = Error::USER_DATA;
	}

	if(!err)
	{
		for(U32 i = 0; i < 16 && !err; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_MISC_LOGE("Failed to return Mat4. Element: %s", m_el->Value());
	}

	return err;
}

Error XmlElement::getVec2(Vec2& out) const
{
	DynamicArrayAuto<F32> arr(m_alloc);
	Error err = getNumbers(arr);

	if(!err && arr.getSize() != 2)
	{
		ANKI_MISC_LOGE("Expecting 2 elements for Vec2");
		err = Error::USER_DATA;
	}

	if(!err)
	{
		for(U32 i = 0; i < 2; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_MISC_LOGE("Failed to return Vec2. Element: %s", m_el->Value());
	}

	return err;
}

Error XmlElement::getVec3(Vec3& out) const
{
	DynamicArrayAuto<F32> arr(m_alloc);
	Error err = getNumbers(arr);

	if(!err && arr.getSize() != 3)
	{
		ANKI_MISC_LOGE("Expecting 3 elements for Vec3");
		err = Error::USER_DATA;
	}

	if(!err)
	{
		for(U32 i = 0; i < 3; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_MISC_LOGE("Failed to return Vec3. Element: %s", m_el->Value());
	}

	return err;
}

Error XmlElement::getVec4(Vec4& out) const
{
	DynamicArrayAuto<F32> arr(m_alloc);
	Error err = getNumbers(arr);

	if(!err && arr.getSize() != 4)
	{
		ANKI_MISC_LOGE("Expecting 4 elements for Vec4");
		err = Error::USER_DATA;
	}

	if(!err)
	{
		for(U32 i = 0; i < 4; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_MISC_LOGE("Failed to return Vec4. Element: %s", m_el->Value());
	}

	return err;
}

Error XmlElement::getChildElementOptional(const CString& name, XmlElement& out) const
{
	Error err = check();
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

Error XmlElement::getChildElement(const CString& name, XmlElement& out) const
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
		ANKI_MISC_LOGE("Cannot find tag \"%s\"", &name[0]);
		err = Error::USER_DATA;
	}

	return err;
}

Error XmlElement::getNextSiblingElement(const CString& name, XmlElement& out) const
{
	Error err = check();
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

	I count = -1;
	do
	{
		el = el->NextSiblingElement(m_el->Name());
		++count;
	} while(el);

	out = U32(count);

	return Error::NONE;
}

Error XmlElement::getAttributeTextOptional(const CString& name, CString& out, Bool& attribPresent) const
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

Error XmlDocument::loadFile(const CString& filename, GenericMemoryPoolAllocator<U8> alloc)
{
	File file;
	ANKI_CHECK(file.open(filename, FileOpenFlag::READ));

	StringAuto text(alloc);
	ANKI_CHECK(file.readAllText(text));

	ANKI_CHECK(parse(text.toCString(), alloc));

	return Error::NONE;
}

Error XmlDocument::parse(const CString& xmlText, GenericMemoryPoolAllocator<U8> alloc)
{
	m_alloc = alloc;

	if(m_doc.Parse(&xmlText[0]))
	{
		ANKI_MISC_LOGE(
			"Cannot parse file. Reason: %s", ((m_doc.GetErrorStr1() == nullptr) ? "unknown" : m_doc.GetErrorStr1()));

		return Error::USER_DATA;
	}

	return Error::NONE;
}

ANKI_USE_RESULT Error XmlDocument::getChildElement(const CString& name, XmlElement& out) const
{
	Error err = Error::NONE;
	out = XmlElement(m_doc.FirstChildElement(&name[0]), m_alloc);

	if(!out)
	{
		ANKI_MISC_LOGE("Cannot find tag \"%s\"", &name[0]);
		err = Error::USER_DATA;
	}

	return err;
}

} // end namespace anki
