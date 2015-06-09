// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/misc/ConfigSet.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
ConfigSet::ConfigSet()
{
	m_alloc = HeapAllocator<U8>(allocAligned, nullptr);
}

//==============================================================================
ConfigSet::~ConfigSet()
{
	for(Option& o : m_options)
	{
		o.m_name.destroy(m_alloc);
		o.m_strVal.destroy(m_alloc);
	}

	m_options.destroy(m_alloc);
}

//==============================================================================
ConfigSet& ConfigSet::operator=(const ConfigSet& b)
{
	m_alloc = b.m_alloc; // Not a copy but we are fine

	for(const Option& o : b.m_options)
	{
		Option newO;
		newO.m_name.create(m_alloc, o.m_name.toCString());
		if(o.m_type == 0)
		{
			newO.m_strVal.create(m_alloc, o.m_strVal.toCString());
		}
		newO.m_fVal = o.m_fVal;
		newO.m_type = o.m_type;

		m_options.emplaceBack(m_alloc, std::move(newO));
	}

	return *this;
}

//==============================================================================
ConfigSet::Option* ConfigSet::tryFind(const CString& name)
{
	List<Option>::Iterator it = m_options.getBegin();
	for(; it != m_options.getEnd(); ++it)
	{
		if((*it).m_name == name)
		{
			return &(*it);
		}
	}

	return nullptr;
}

//==============================================================================
const ConfigSet::Option* ConfigSet::tryFind(const CString& name) const
{
	List<Option>::ConstIterator it = m_options.getBegin();
	for(; it != m_options.getEnd(); ++it)
	{
		if((*it).m_name == name)
		{
			return &(*it);
		}
	}

	return nullptr;
}

//==============================================================================
void ConfigSet::newOption(const CString& name, const CString& value)
{
	ANKI_ASSERT(!tryFind(name));

	Option o;
	o.m_name.create(m_alloc, name);
	o.m_strVal.create(m_alloc, value);
	o.m_type = 0;

	m_options.emplaceBack(m_alloc, std::move(o));
}

//==============================================================================
void ConfigSet::newOption(const CString& name, F64 value)
{
	ANKI_ASSERT(!tryFind(name));

	Option o;
	o.m_name.create(m_alloc, name);
	o.m_fVal = value;
	o.m_type = 1;

	m_options.emplaceBack(m_alloc, std::move(o));
}

//==============================================================================
void ConfigSet::set(const CString& name, F64 value)
{
	Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 1);
	o->m_fVal = value;
}

//==============================================================================
void ConfigSet::set(const CString& name, const CString& value)
{
	Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 0);
	o->m_strVal.destroy(m_alloc);
	o->m_strVal.create(m_alloc, value);
}

//==============================================================================
F64 ConfigSet::getNumber(const CString& name) const
{
	const Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 1);
	return o->m_fVal;
}

//==============================================================================
CString ConfigSet::getString(const CString& name) const
{
	const Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 0);
	return o->m_strVal.toCString();
}

} // end namespace anki
