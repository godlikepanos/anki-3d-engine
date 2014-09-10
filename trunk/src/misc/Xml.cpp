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
I64 XmlElement::getInt() const
{
	check();
	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return int. Element: %s", 
			m_el->Value());
	}
	return CString(txt).toI64();
}

//==============================================================================
F64 XmlElement::getFloat() const
{
	check();
	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return float. Element: %s", 
			m_el->Value());
	}
	return CString(txt).toF64();
}

//==============================================================================V
Vector<F64, StackAllocator<F64>> XmlElement::getFloats() const
{
	check();
	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return floats. Element: %s",
			m_el->Value());
	}

	StringListBase<StackAllocator<char>> list(
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc));

	Vector<F64, StackAllocator<F64>> out;
	out.resize(list.size());

	try
	{
		for(U i = 0; i < out.size(); i++)
		{
			out[i] = list[i].toF64();
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Found non-float element for vec of floats");
	}

	return out;
}

//==============================================================================
Mat4 XmlElement::getMat4() const
{
	check();

	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return Mat4");
	}

	StringListBase<StackAllocator<char>> list = 
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc);

	if(list.size() != 16)
	{
		throw ANKI_EXCEPTION("Expecting 16 elements for Mat4");
	}

	Mat4 out;

	try
	{
		for(U i = 0; i < 16; i++)
		{
			out[i] = list[i].toF64();
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

	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return Vec3");
	}

	StringListBase<StackAllocator<char>> list = 
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc);

	if(list.size() != 3)
	{
		throw ANKI_EXCEPTION("Expecting 3 elements for Vec3");
	}

	Vec3 out;

	try
	{
		for(U i = 0; i < 3; i++)
		{
			out[i] = list[i].toF64();
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

	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to return Vec4");
	}

	StringListBase<StackAllocator<char>> list = 
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc);

	if(list.size() != 4)
	{
		throw ANKI_EXCEPTION("Expecting 3 elements for Vec4");
	}

	Vec4 out;

	try
	{
		for(U i = 0; i < 4; i++)
		{
			out[i] = list[i].toF64();
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Found non-float element for Vec4");
	}

	return out;
}

//==============================================================================
XmlElement XmlElement::getChildElementOptional(const CString& name) const
{
	check();
	XmlElement out;
	out.m_el = m_el->FirstChildElement(&name[0]);
	return out;
}

//==============================================================================
XmlElement XmlElement::getChildElement(const CString& name) const
{
	check();
	const XmlElement out = getChildElementOptional(name);
	if(!out)
	{
		throw ANKI_EXCEPTION("Cannot find tag %s", &name[0]);
	}
	return out;
}

//==============================================================================
XmlElement XmlElement::getNextSiblingElement(const CString& name) const
{
	check();
	XmlElement out;
	out.m_el = m_el->NextSiblingElement(&name[0]);
	return out;
}

//==============================================================================
// XmlDocument                                                                 =
//==============================================================================

//==============================================================================
void XmlDocument::loadFile(const CString& filename, StackAllocator<U8>& alloc)
{
	File file(filename, File::OpenFlag::READ);

	m_alloc = alloc;
	StringBase<StackAllocator<char>> text(m_alloc);
	file.readAllText(text);

	if(m_doc.Parse(&text[0]))
	{
		throw ANKI_EXCEPTION("Cannot parse file. Reason: %s",
			((m_doc.GetErrorStr1() == nullptr)
			? "unknown" : m_doc.GetErrorStr1()));
	}
}

} // end namespace anki
