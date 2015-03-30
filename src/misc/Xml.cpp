// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
			err = CString(txt).toI64(out);
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
			err = CString(txt).toF64(out);
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
Error XmlElement::getFloats(DArrayAuto<F64>& out) const
{
	Error err = check();
	
	const char* txt;
	if(!err)
	{
		txt = m_el->GetText();
		if(txt == nullptr)
		{
			err = ErrorCode::USER_DATA;
		}
	}

	StringList list;
	if(!err)
	{
		err = list.splitString(m_alloc, txt, ' ');
	}

	out = std::move(DArrayAuto<F64>(m_alloc));

	if(!err)
	{
		err = out.create(list.getSize());
	}

	auto it = list.getBegin();
	for(U i = 0; i < out.getSize() && !err; i++)
	{
		err = it->toF64(out[i]);
		++it;
	}

	if(err)
	{
		ANKI_LOGE("Failed to return floats. Element: %s", m_el->Value());
	}

	list.destroy(m_alloc);

	return err;
}

//==============================================================================
Error XmlElement::getMat4(Mat4& out) const
{
	DArrayAuto<F64> arr(m_alloc);
	Error err = getFloats(arr);	

	if(!err && arr.getSize() != 16)
	{
		ANKI_LOGE("Expecting 16 elements for Mat4");
		err = ErrorCode::USER_DATA;
	}

	if(!err)
	{
		for(U i = 0; i < 16 && !err; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_LOGE("Failed to return Mat4. Element: %s", m_el->Value());
	}

	return err;
}

//==============================================================================
Error XmlElement::getVec3(Vec3& out) const
{
	DArrayAuto<F64> arr(m_alloc);
	Error err = getFloats(arr);
	
	if(!err && arr.getSize() != 3)
	{
		ANKI_LOGE("Expecting 3 elements for Vec3");
		err = ErrorCode::USER_DATA;
	}

	if(!err)
	{
		for(U i = 0; i < 3; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_LOGE("Failed to return Vec3. Element: %s", m_el->Value());
	}
	
	return err;
}
//==============================================================================
Error XmlElement::getVec4(Vec4& out) const
{
	DArrayAuto<F64> arr(m_alloc);
	Error err = getFloats(arr);
	
	if(!err && arr.getSize() != 4)
	{
		ANKI_LOGE("Expecting 4 elements for Vec3");
		err = ErrorCode::USER_DATA;
	}

	if(!err)
	{
		for(U i = 0; i < 4; i++)
		{
			out[i] = arr[i];
		}
	}

	if(err)
	{
		ANKI_LOGE("Failed to return Vec4. Element: %s", m_el->Value());
	}
	
	return err;
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
Error XmlElement::getSiblingElementsCount(U32& out) const
{
	Error err = check();
	if(!err)
	{
		tinyxml2::XMLElement* el = m_el;
		
		I count = -1;
		do
		{
			el = el->NextSiblingElement(m_el->Name());
			++count;
		} while(el);

		out = count;
	}
	else
	{
		out = 0;
	}

	return err;
}

//==============================================================================
// XmlDocument                                                                 =
//==============================================================================

//==============================================================================
Error XmlDocument::loadFile(const CString& filename, 
	GenericMemoryPoolAllocator<U8> alloc)
{
	Error err = ErrorCode::NONE;

	File file;
	err = file.open(filename, File::OpenFlag::READ);
	if(err)
	{
		return err;
	}

	m_alloc = alloc;
	String text;
	
	err = file.readAllText(m_alloc, text);
	if(err)
	{
		return err;
	}

	if(m_doc.Parse(&text[0]))
	{
		ANKI_LOGE("Cannot parse file. Reason: %s",
			((m_doc.GetErrorStr1() == nullptr)
			? "unknown" : m_doc.GetErrorStr1()));
	}

	text.destroy(m_alloc);

	return err;
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
