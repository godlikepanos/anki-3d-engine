// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Xml.h>
#include <anki/util/StringList.h>

namespace anki
{

template<typename T>
Error XmlElement::getNumber(T& out) const
{
	ANKI_CHECK(check());

	const char* txt = m_el->GetText();
	if(txt != nullptr)
	{
		ANKI_CHECK(CString(txt).toNumber(out));
	}
	else
	{
		ANKI_UTIL_LOGE("Failed to return number. Element: %s", m_el->Value());
		return Error::USER_DATA;
	}

	return Error::NONE;
}

template<typename T>
Error XmlElement::getNumbers(DynamicArrayAuto<T>& out) const
{
	CString txt;
	ANKI_CHECK(getText(txt));

	if(txt)
	{
		return parseNumbers(txt, out);
	}
	else
	{
		out.destroy();
		return Error::NONE;
	}
}

template<typename TArray>
Error XmlElement::getNumbers(TArray& out) const
{
	CString txt;
	ANKI_CHECK(getText(txt));
	return parseNumbers(txt, out);
}

template<typename T>
Error XmlElement::getAttributeNumbersOptional(CString name, DynamicArrayAuto<T>& out, Bool& attribPresent) const
{
	CString txtVal;
	ANKI_CHECK(getAttributeTextOptional(name, txtVal, attribPresent));

	if(txtVal && attribPresent)
	{
		return parseNumbers(txtVal, out);
	}
	else
	{
		return Error::NONE;
	}
}

template<typename TArray>
Error XmlElement::getAttributeNumbersOptional(CString name, TArray& out, Bool& attribPresent) const
{
	CString txtVal;
	ANKI_CHECK(getAttributeTextOptional(name, txtVal, attribPresent));

	if(txtVal && attribPresent)
	{
		return parseNumbers(txtVal, out);
	}
	else
	{
		return Error::NONE;
	}
}

template<typename T>
Error XmlElement::getAttributeNumberOptional(CString name, T& out, Bool& attribPresent) const
{
	DynamicArrayAuto<T> arr(m_alloc);
	ANKI_CHECK(getAttributeNumbersOptional(name, arr, attribPresent));

	if(attribPresent)
	{
		if(arr.getSize() != 1)
		{
			ANKI_UTIL_LOGE("Expecting one element for attrib: %s", &name[0]);
			return Error::USER_DATA;
		}

		out = arr[0];
	}

	return Error::NONE;
}

template<typename T>
Error XmlElement::parseNumbers(CString txt, DynamicArrayAuto<T>& out) const
{
	ANKI_ASSERT(txt);
	ANKI_ASSERT(m_el);

	StringListAuto list(m_alloc);
	list.splitString(txt, ' ');

	out.destroy();
	out.create(U32(list.getSize()));

	Error err = Error::NONE;
	auto it = list.getBegin();
	auto end = list.getEnd();
	U32 i = 0;
	while(it != end && !err)
	{
		err = it->toNumber(out[i++]);
		++it;
	}

	if(err)
	{
		ANKI_UTIL_LOGE("Failed to covert to numbers the element: %s", m_el->Value());
	}

	return err;
}

template<typename TArray>
Error XmlElement::parseNumbers(CString txt, TArray& out) const
{
	ANKI_ASSERT(!txt.isEmpty());
	ANKI_ASSERT(m_el);

	StringListAuto list(m_alloc);
	list.splitString(txt, ' ');
	const PtrSize listSize = list.getSize();

	if(listSize != out.getSize())
	{
		ANKI_UTIL_LOGE("Wrong number of elements for element: %s", m_el->Value());
		return Error::USER_DATA;
	}

	Error err = Error::NONE;
	auto it = list.getBegin();
	auto end = list.getEnd();
	U32 i = 0;
	while(it != end && !err)
	{
		err = it->toNumber(out[i++]);
		++it;
	}

	if(err)
	{
		ANKI_UTIL_LOGE("Failed to covert to numbers the element: %s", m_el->Value());
	}

	return err;
}

} // end namespace anki
