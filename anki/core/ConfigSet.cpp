// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/ConfigSet.h>
#include <anki/util/Xml.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>

// Used by the config options
#include <anki/util/System.h>
#include <anki/renderer/ClusterBin.h>

namespace anki
{

class ConfigSet::Option : public NonCopyable
{
public:
	enum Type
	{
		NONE,
		FLOAT,
		UNSIGNED,
		STRING
	};

	String m_name;
	String m_helpMsg;

	String m_str;
	F64 m_float = 0.0;
	F64 m_minFloat = 0.0;
	F64 m_maxFloat = 0.0;
	U64 m_unsigned = 0;
	U64 m_minUnsigned = 0;
	U64 m_maxUnsigned = 0;

	Type m_type = NONE;

	Option() = default;

	Option(Option&& b)
		: m_name(std::move(b.m_name))
		, m_helpMsg(std::move(b.m_helpMsg))
		, m_str(std::move(b.m_str))
		, m_float(b.m_float)
		, m_minFloat(b.m_minFloat)
		, m_maxFloat(b.m_maxFloat)
		, m_unsigned(b.m_unsigned)
		, m_minUnsigned(b.m_minUnsigned)
		, m_maxUnsigned(b.m_maxUnsigned)
		, m_type(b.m_type)
	{
	}

	~Option() = default;

	Option& operator=(Option&& b) = delete;
};

ConfigSet::ConfigSet()
{
	m_alloc = HeapAllocator<U8>(allocAligned, nullptr);

#define ANKI_CONFIG_OPTION(name, ...) newOption(ANKI_STRINGIZE(name), __VA_ARGS__);
#include <anki/core/ConfigDefs.h>
#include <anki/resource/ConfigDefs.h>
#include <anki/renderer/ConfigDefs.h>
#include <anki/scene/ConfigDefs.h>
#include <anki/gr/ConfigDefs.h>
#undef ANKI_CONFIG_OPTION
}

ConfigSet::~ConfigSet()
{
	for(Option& o : m_options)
	{
		o.m_name.destroy(m_alloc);
		o.m_str.destroy(m_alloc);
		o.m_helpMsg.destroy(m_alloc);
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
		if(o.m_type == Option::STRING)
		{
			newO.m_str.create(m_alloc, o.m_str.toCString());
		}
		newO.m_float = o.m_float;
		newO.m_minFloat = o.m_minFloat;
		newO.m_maxFloat = o.m_maxFloat;
		newO.m_unsigned = o.m_unsigned;
		newO.m_minUnsigned = o.m_minUnsigned;
		newO.m_maxUnsigned = o.m_maxUnsigned;
		newO.m_type = o.m_type;

		m_options.emplaceBack(m_alloc, std::move(newO));
	}

	return *this;
}

ConfigSet::Option* ConfigSet::tryFind(CString optionName)
{
	for(List<Option>::Iterator it = m_options.getBegin(); it != m_options.getEnd(); ++it)
	{
		if((*it).m_name == optionName)
		{
			return &(*it);
		}
	}

	return nullptr;
}

const ConfigSet::Option* ConfigSet::tryFind(CString optionName) const
{
	for(List<Option>::ConstIterator it = m_options.getBegin(); it != m_options.getEnd(); ++it)
	{
		if((*it).m_name == optionName)
		{
			return &(*it);
		}
	}

	return nullptr;
}

void ConfigSet::newOption(CString optionName, CString value, CString helpMsg)
{
	ANKI_ASSERT(!tryFind(optionName));

	Option o;
	o.m_name.create(m_alloc, optionName);
	o.m_str.create(m_alloc, value);
	o.m_type = Option::STRING;
	if(!helpMsg.isEmpty())
	{
		o.m_helpMsg.create(m_alloc, helpMsg);
	}

	m_options.emplaceBack(m_alloc, std::move(o));
}

void ConfigSet::newOptionInternal(CString optionName, F64 value, F64 minValue, F64 maxValue, CString helpMsg)
{
	ANKI_ASSERT(!tryFind(optionName));
	ANKI_ASSERT(value >= minValue && value <= maxValue && minValue <= maxValue);

	Option o;
	o.m_name.create(m_alloc, optionName);
	o.m_float = value;
	o.m_minFloat = minValue;
	o.m_maxFloat = maxValue;
	o.m_type = Option::FLOAT;
	if(!helpMsg.isEmpty())
	{
		o.m_helpMsg.create(m_alloc, helpMsg);
	}

	m_options.emplaceBack(m_alloc, std::move(o));
}

void ConfigSet::newOptionInternal(CString optionName, U64 value, U64 minValue, U64 maxValue, CString helpMsg)
{
	ANKI_ASSERT(!tryFind(optionName));
	ANKI_ASSERT(value >= minValue && value <= maxValue && minValue <= maxValue);

	Option o;
	o.m_name.create(m_alloc, optionName);
	o.m_unsigned = value;
	o.m_minUnsigned = minValue;
	o.m_maxUnsigned = maxValue;
	o.m_type = Option::UNSIGNED;
	if(!helpMsg.isEmpty())
	{
		o.m_helpMsg.create(m_alloc, helpMsg);
	}

	m_options.emplaceBack(m_alloc, std::move(o));
}

void ConfigSet::set(CString optionName, CString value)
{
	Option& o = find(optionName);
	ANKI_ASSERT(o.m_type == Option::STRING);
	o.m_str.destroy(m_alloc);
	o.m_str.create(m_alloc, value);
}

void ConfigSet::setInternal(CString optionName, F64 value)
{
	Option& o = find(optionName);
	ANKI_ASSERT(o.m_type == Option::FLOAT);
	ANKI_ASSERT(value >= o.m_minFloat);
	ANKI_ASSERT(value <= o.m_maxFloat);
	o.m_float = value;
}

void ConfigSet::setInternal(CString optionName, U64 value)
{
	Option& o = find(optionName);
	ANKI_ASSERT(o.m_type == Option::UNSIGNED);
	ANKI_ASSERT(value >= o.m_minUnsigned);
	ANKI_ASSERT(value <= o.m_maxUnsigned);
	o.m_unsigned = value;
}

F64 ConfigSet::getNumberF64(CString optionName) const
{
	const Option& option = find(optionName);
	ANKI_ASSERT(option.m_type == Option::FLOAT);
	return option.m_float;
}

F32 ConfigSet::getNumberF32(CString optionName) const
{
	return F32(getNumberF64(optionName));
}

U64 ConfigSet::getNumberU64(CString optionName) const
{
	const Option& option = find(optionName);
	ANKI_ASSERT(option.m_type == Option::UNSIGNED);
	return option.m_unsigned;
}

U32 ConfigSet::getNumberU32(CString optionName) const
{
	const U64 out = getNumberU64(optionName);
	if(out > MAX_U32)
	{
		ANKI_CORE_LOGW("Option is out of U32 range: %s", optionName.cstr());
	}
	return U32(out);
}

U16 ConfigSet::getNumberU16(CString optionName) const
{
	const U64 out = getNumberU64(optionName);
	if(out > MAX_U16)
	{
		ANKI_CORE_LOGW("Option is out of U16 range: %s", optionName.cstr());
	}
	return U16(out);
}

U8 ConfigSet::getNumberU8(CString optionName) const
{
	const U64 out = getNumberU64(optionName);
	if(out > MAX_U8)
	{
		ANKI_CORE_LOGW("Option is out of U8 range: %s", optionName.cstr());
	}
	return U8(out);
}

Bool ConfigSet::getBool(CString optionName) const
{
	const U64 val = getNumberU64(optionName);
	if((val & ~U64(1)) != 0)
	{
		ANKI_CORE_LOGW("Expecting 0 or 1 for the config option \"%s\". Will mask out extra bits", optionName.cstr());
	}
	return val & 1;
}

CString ConfigSet::getString(CString optionName) const
{
	const Option& o = find(optionName);
	ANKI_ASSERT(o.m_type == Option::STRING);
	return o.m_str.toCString();
}

Error ConfigSet::loadFromFile(CString filename)
{
	ANKI_CORE_LOGI("Loading config file %s", filename.cstr());
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
			if(option.m_type == Option::FLOAT)
			{
				ANKI_CHECK(el.getNumber(option.m_float));
			}
			else if(option.m_type == Option::UNSIGNED)
			{
				ANKI_CHECK(el.getNumber(option.m_unsigned));
			}
			else
			{
				ANKI_ASSERT(option.m_type == Option::STRING);
				CString txt;
				ANKI_CHECK(el.getText(txt));
				option.m_str.destroy(m_alloc);
				option.m_str.create(m_alloc, txt);
			}
		}
		else
		{
			if(option.m_type == Option::FLOAT)
			{
				ANKI_CORE_LOGW("Missing option for \"%s\". Will use the default value: %f", &option.m_name[0],
							   option.m_float);
			}
			else if(option.m_type == Option::UNSIGNED)
			{
				ANKI_CORE_LOGW("Missing option for \"%s\". Will use the default value: %" PRIu64, &option.m_name[0],
							   option.m_unsigned);
			}
			else
			{
				ANKI_ASSERT(option.m_type == Option::STRING);
				ANKI_CORE_LOGW("Missing option for \"%s\". Will use the default value: %s", option.m_name.cstr(),
							   option.m_str.cstr());
			}
		}
	}

	return Error::NONE;
}

Error ConfigSet::saveToFile(CString filename) const
{
	ANKI_CORE_LOGI("Saving config file %s", &filename[0]);

	File file;
	ANKI_CHECK(file.open(filename, FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("%s\n<config>\n", XmlDocument::XML_HEADER.cstr()));

	for(const Option& option : m_options)
	{
		if(!option.m_helpMsg.isEmpty())
		{
			ANKI_CHECK(file.writeText("\t<!-- %s -->\n", option.m_helpMsg.cstr()));
		}

		if(option.m_type == Option::FLOAT)
		{
			ANKI_CHECK(file.writeText("\t<%s>%f</%s>\n", option.m_name.cstr(), option.m_float, option.m_name.cstr()));
		}
		else if(option.m_type == Option::UNSIGNED)
		{
			ANKI_CHECK(file.writeText("\t<%s>%" PRIu64 "</%s>\n", option.m_name.cstr(), option.m_unsigned,
									  option.m_name.cstr()));
		}
		else
		{
			ANKI_ASSERT(option.m_type == Option::STRING);
			ANKI_CHECK(file.writeText("\t<%s><![CDATA[%s]]></%s>\n", option.m_name.cstr(), option.m_str.cstr(),
									  option.m_name.cstr()));
		}
	}

	ANKI_CHECK(file.writeText("</config>\n"));
	return Error::NONE;
}

Error ConfigSet::setFromCommandLineArguments(U32 cmdLineArgsCount, char* cmdLineArgs[])
{
	for(U i = 0; i < cmdLineArgsCount; ++i)
	{
		const char* arg = cmdLineArgs[i];
		ANKI_ASSERT(arg);

		Option* option = tryFind(arg);
		if(option == nullptr)
		{
			ANKI_CORE_LOGW("Can't recognize command line argument: %s", arg);
			continue;
		}

		// Set the value
		++i;
		if(i >= cmdLineArgsCount)
		{
			ANKI_CORE_LOGE("Expecting a command line argument after %s", arg);
			return Error::USER_DATA;
		}

		arg = cmdLineArgs[i];
		ANKI_ASSERT(arg);
		if(option->m_type == Option::STRING)
		{
			option->m_str.destroy(m_alloc);
			option->m_str.create(m_alloc, arg);
		}
		else if(option->m_type == Option::FLOAT)
		{
			CString val(arg);
			ANKI_CHECK(val.toNumber(option->m_float));
		}
		else
		{
			ANKI_ASSERT(option->m_type == Option::UNSIGNED);
			CString val(arg);
			ANKI_CHECK(val.toNumber(option->m_unsigned));
		}
	}

	return Error::NONE;
}

} // end namespace anki
