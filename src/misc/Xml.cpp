// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/misc/Xml.h"
#include "anki/util/StringList.h"
#include "anki/util/File.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
// XmlElement                                                                  =
//==============================================================================

//==============================================================================
ANKI_USE_RESULT Error XmlElement::check() const
{
	Error err = ErrorCode::NONE;
	if(m_el == nullptr)
	{
		ANKI_LOGE("Empty element");
		err = ErrorCode::USER_DATA;
	}
	return err;
}

//==============================================================================
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

//==============================================================================
Error XmlElement::getI64(I64& out) const
{
	Error err = check();

	if(!err)
	{
		const char* txt = m_el->GetText();
		if(txt != nullptr)
		{
			out = CString(txt).toI64();
		}
		else
		{
			ANKI_LOGE("Failed to return int. Element: %s", m_el->Value());
			err = ErrorCode::USER_DATA;
		}
	}

	return err;
}

//==============================================================================
Error XmlElement::getF64(F64& out) const
{
	Error err = check();
	
	if(!err)
	{
		const char* txt = m_el->GetText();
		if(txt != nullptr)
		{
			out = CString(txt).toF64();
		}
		else
		{
			ANKI_LOGE("Failed to return float. Element: %s", m_el->Value());
			err = ErrorCode::USER_DATA;
		}
	}

	return err;
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
Error XmlElement::getMat4(Mat4& out) const
{
	check();

	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		ANKI_LOGE("Failed to return Mat4");
		return ErrorCode::USER_DATA;
	}

	StringListBase<StackAllocator<char>> list = 
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc);

	if(list.size() != 16)
	{
		ANKI_LOGE("Expecting 16 elements for Mat4");
		return ErrorCode::USER_DATA;
	}

	for(U i = 0; i < 16; i++)
	{
		out[i] = list[i].toF64();
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error XmlElement::getVec3(Vec3& out) const
{
	Error err = check();
	if(err)
	{
		return err;
	}

	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		ANKI_LOGE("Failed to return Vec3");
		return ErrorCode::USER_DATA;
	}

	StringListBase<StackAllocator<char>> list = 
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc);

	if(list.size() != 3)
	{
		ANKI_LOGE("Expecting 3 elements for Vec3");
		return ErrorCode::USER_DATA;
	}

	for(U i = 0; i < 3; i++)
	{
		out[i] = list[i].toF64();
	}
	
	return ErrorCode::NONE;
}
//==============================================================================
Error XmlElement::getVec4(Vec4& out) const
{
	Error err = check();
	if(err)
	{
		return err;
	}

	const char* txt = m_el->GetText();
	if(txt == nullptr)
	{
		ANKI_LOGE("Failed to return Vec4");
		return ErrorCode::USER_DATA;
	}

	StringListBase<StackAllocator<char>> list = 
		StringListBase<StackAllocator<char>>::splitString(
		CString(txt), ' ', m_alloc);

	if(list.size() != 4)
	{
		ANKI_LOGE("Expecting 3 elements for Vec4");
		return ErrorCode::USER_DATA;
	}

	for(U i = 0; i < 4; i++)
	{
		out[i] = list[i].toF64();
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error XmlElement::getChildElementOptional(
	const CString& name, XmlElement& out) const
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

//==============================================================================
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
		ANKI_LOGE("Cannot find tag %s", &name[0]);
		err = ErrorCode::USER_DATA;
	}
	
	return err;
}

//==============================================================================
Error XmlElement::getNextSiblingElement(
	const CString& name, XmlElement& out) const
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

//==============================================================================
// XmlDocument                                                                 =
//==============================================================================

//==============================================================================
void XmlDocument::loadFile(const CString& filename, StackAllocator<U8>& alloc)
{
	File file;
	file.open(filename, File::OpenFlag::READ);

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

//==============================================================================
ANKI_USE_RESULT Error XmlDocument::getChildElement(
	const CString& name, XmlElement& out)
{
	Error err = ErrorCode::NONE;
	out = XmlElement(m_doc.FirstChildElement(&name[0]), m_alloc);

	if(!out)
	{
		ANKI_LOGE("Cannot find tag %s", &name[0]);
		err = ErrorCode::USER_DATA;
	}

	return err;
}

} // end namespace anki
