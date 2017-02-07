// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/misc/ConfigSet.h>
#include <anki/misc/Xml.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>

namespace anki
{

ConfigSet::ConfigSet()
{
	m_alloc = HeapAllocator<U8>(allocAligned, nullptr);
}

ConfigSet::~ConfigSet()
{
	for(Option& o : m_options)
	{
		o.m_name.destroy(m_alloc);
		o.m_strVal.destroy(m_alloc);
	}

	m_options.destroy(m_alloc);
}

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

void ConfigSet::newOption(const CString& name, const CString& value)
{
	ANKI_ASSERT(!tryFind(name));

	Option o;
	o.m_name.create(m_alloc, name);
	o.m_strVal.create(m_alloc, value);
	o.m_type = 0;

	m_options.emplaceBack(m_alloc, std::move(o));
}

void ConfigSet::newOption(const CString& name, F64 value)
{
	ANKI_ASSERT(!tryFind(name));

	Option o;
	o.m_name.create(m_alloc, name);
	o.m_fVal = value;
	o.m_type = 1;

	m_options.emplaceBack(m_alloc, std::move(o));
}

void ConfigSet::set(const CString& name, F64 value)
{
	Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 1);
	o->m_fVal = value;
}

void ConfigSet::set(const CString& name, const CString& value)
{
	Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 0);
	o->m_strVal.destroy(m_alloc);
	o->m_strVal.create(m_alloc, value);
}

F64 ConfigSet::getNumber(const CString& name) const
{
	const Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 1);
	return o->m_fVal;
}

CString ConfigSet::getString(const CString& name) const
{
	const Option* o = tryFind(name);
	ANKI_ASSERT(o);
	ANKI_ASSERT(o->m_type == 0);
	return o->m_strVal.toCString();
}

Error ConfigSet::loadFromFile(CString filename)
{
	ANKI_MISC_LOGI("Loading config file %s", &filename[0]);
	XmlDocument xml;
	ANKI_CHECK(xml.loadFile(filename, m_alloc));

	XmlElement rootel;
	ANKI_CHECK(xml.getChildElement("config", rootel));

	for(Option& option : m_options)
	{
		XmlElement el;
		ANKI_CHECK(rootel.getChildElementOptional(option.m_name.toCString(), el));

		if(el)
		{
			if(option.m_type == 1)
			{
				ANKI_CHECK(el.getF64(option.m_fVal));
			}
			else
			{
				CString txt;
				ANKI_CHECK(el.getText(txt));
				option.m_strVal.destroy(m_alloc);
				option.m_strVal.create(m_alloc, txt);
			}
		}
		else
		{
			if(option.m_type == 0)
			{
				ANKI_MISC_LOGW("Missing option for \"%s\". Will use the default value: %s",
					&option.m_name[0],
					&option.m_strVal[0]);
			}
			else
			{
				ANKI_MISC_LOGW(
					"Missing option for \"%s\". Will use the default value: %f", &option.m_name[0], option.m_fVal);
			}
		}
	}

	return ErrorCode::NONE;
}

Error ConfigSet::saveToFile(CString filename) const
{
	ANKI_MISC_LOGI("Saving config file %s", &filename[0]);

	File file;
	ANKI_CHECK(file.open(filename, FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("%s\n<config>\n", &XmlDocument::XML_HEADER[0]));

	for(const Option& option : m_options)
	{
		if(option.m_type == 1)
		{
			// Some of the options are integers so try not to make them appear
			// as floats in the file
			if(option.m_fVal == round(option.m_fVal) && option.m_fVal >= 0.0)
			{
				ANKI_CHECK(file.writeText("\t<%s>%u</%s>\n", &option.m_name[0], U(option.m_fVal), &option.m_name[0]));
			}
			else
			{
				ANKI_CHECK(file.writeText("\t<%s>%f</%s>\n", &option.m_name[0], option.m_fVal, &option.m_name[0]));
			}
		}
		else
		{
			ANKI_CHECK(file.writeText("\t<%s>%s</%s>\n", &option.m_name[0], &option.m_strVal[0], &option.m_name[0]));
		}
	}

	ANKI_CHECK(file.writeText("</config>\n"));
	return ErrorCode::NONE;
}

Error ConfigSet::setFromCommandLineArguments(U cmdLineArgsCount, char* cmdLineArgs[])
{
	for(U i = 0; i < cmdLineArgsCount; ++i)
	{
		const char* arg = cmdLineArgs[i];
		ANKI_ASSERT(arg);
		if(CString(arg) == "-cfg")
		{
			if(i + 2 >= cmdLineArgsCount)
			{
				ANKI_MISC_LOGE("Wrong number of arguments after -cfg");
				return ErrorCode::USER_DATA;
			}

			// Get the option
			++i;
			arg = cmdLineArgs[i];
			ANKI_ASSERT(arg);
			Option* option = tryFind(arg);
			if(option == nullptr)
			{
				ANKI_MISC_LOGE("Option name following -cfg not found: %s", arg);
				return ErrorCode::USER_DATA;
			}

			// Set the value
			++i;
			arg = cmdLineArgs[i];
			ANKI_ASSERT(arg);
			if(option->m_type == 0)
			{
				option->m_strVal.destroy(m_alloc);
				option->m_strVal.create(m_alloc, arg);
			}
			else
			{
				CString val(arg);
				ANKI_CHECK(val.toF64(option->m_fVal));
			}
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
