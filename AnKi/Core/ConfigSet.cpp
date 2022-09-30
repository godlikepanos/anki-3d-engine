// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/File.h>

// Because some cvars set their default values
#include <AnKi/Util/System.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

ConfigSet::~ConfigSet()
{
#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) \
	if(m_##name.m_value) \
	{ \
		m_pool.free(m_##name.m_value); \
	}
#include <AnKi/Core/AllConfigVars.defs.h>

#if ANKI_EXTRA_CHECKS
	for(const Var& var : m_vars)
	{
		if(var.m_timesAccessed.load() == 0)
		{
			ANKI_CORE_LOGW("Config var doesn't appear to have been accessed. Maybe unused: %s", var.m_name.cstr());
		}
	}
#endif
}

void ConfigSet::init(AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ANKI_ASSERT(!isInitialized());
	m_pool.init(allocCb, allocCbUserData);

#define ANKI_CONFIG_VAR_NUMERIC(name, defaultValue, minValue, maxValue, description) \
	ANKI_ASSERT(minValue <= maxValue && defaultValue >= minValue && defaultValue <= maxValue); \
	m_##name.m_name = #name; \
	m_##name.m_description = description; \
	m_##name.m_value = defaultValue; \
	m_##name.m_min = minValue; \
	m_##name.m_max = maxValue; \
	m_vars.pushBack(&m_##name);

#define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_NUMERIC(name, defaultValue, minValue, maxValue, description)

#define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_NUMERIC(name, defaultValue, minValue, maxValue, description)

#define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_NUMERIC(name, defaultValue, minValue, maxValue, description)

#define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_NUMERIC(name, defaultValue, minValue, maxValue, description)

#define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description) \
	m_##name.m_name = #name; \
	m_##name.m_description = description; \
	m_##name.m_value = defaultValue; \
	m_vars.pushBack(&m_##name);

#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) \
	m_##name.m_name = #name; \
	m_##name.m_description = description; \
	setStringVar(defaultValue, m_##name); \
	m_vars.pushBack(&m_##name);

#include <AnKi/Core/AllConfigVars.defs.h>
}

ConfigSet::Var* ConfigSet::tryFind(CString optionName)
{
	for(auto it = m_vars.getBegin(); it != m_vars.getEnd(); ++it)
	{
		if((*it).m_name == optionName)
		{
			return &(*it);
		}
	}

	return nullptr;
}

const ConfigSet::Var* ConfigSet::tryFind(CString optionName) const
{
	for(auto it = m_vars.getBegin(); it != m_vars.getEnd(); ++it)
	{
		if((*it).m_name == optionName)
		{
			return &(*it);
		}
	}

	return nullptr;
}

Error ConfigSet::loadFromFile(CString filename)
{
	ANKI_ASSERT(isInitialized());

	ANKI_CORE_LOGI("Loading config file %s", filename.cstr());
	XmlDocument xml(&m_pool);
	ANKI_CHECK(xml.loadFile(filename));

	XmlElement rootel;
	ANKI_CHECK(xml.getChildElement("config", rootel));

	XmlElement el;

#define ANKI_NUMERIC(name) \
	ANKI_CHECK(rootel.getChildElementOptional(m_##name.m_name, el)); \
	if(el) \
	{ \
		ANKI_CHECK(el.getNumber(m_##name.m_value)); \
	}

#define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(name)
#define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(name)
#define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(name)
#define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(name)

#define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description) \
	ANKI_CHECK(rootel.getChildElementOptional(m_##name.m_name, el)); \
	if(el) \
	{ \
		CString txt; \
		ANKI_CHECK(el.getText(txt)); \
		if(txt == "true") \
		{ \
			m_##name.m_value = true; \
		} \
		else if(txt == "false") \
		{ \
			m_##name.m_value = false; \
		} \
		else \
		{ \
			ANKI_CORE_LOGE("Wrong value for %s", m_##name.m_name.cstr()); \
			return Error::kUserData; \
		} \
	}

#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) \
	ANKI_CHECK(rootel.getChildElementOptional(m_##name.m_name, el)); \
	if(el) \
	{ \
		CString txt; \
		ANKI_CHECK(el.getText(txt)); \
		setStringVar(txt, m_##name); \
	}

#include <AnKi/Core/AllConfigVars.defs.h>
#undef ANKI_NUMERIC

	return Error::kNone;
}

Error ConfigSet::saveToFile(CString filename) const
{
	ANKI_ASSERT(isInitialized());
	ANKI_CORE_LOGI("Saving config file %s", &filename[0]);

	File file;
	ANKI_CHECK(file.open(filename, FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeTextf("%s\n<config>\n", XmlDocument::kXmlHeader.cstr()));

#define ANKI_NUMERIC_UINT(name) \
	ANKI_CHECK(file.writeTextf("\t<!-- %s -->\n", m_##name.m_description.cstr())); \
	ANKI_CHECK(file.writeTextf("\t<%s>%" PRIu64 "</%s>\n", m_##name.m_name.cstr(), U64(m_##name.m_value), \
							   m_##name.m_name.cstr()));

#define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC_UINT(name)
#define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC_UINT(name)
#define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC_UINT(name)
#define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC_UINT(name)
#define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description) \
	ANKI_CHECK(file.writeTextf("\t<!-- %s -->\n", m_##name.m_description.cstr())); \
	ANKI_CHECK(file.writeTextf("\t<%s>%s</%s>\n", m_##name.m_name.cstr(), (m_##name.m_value) ? "true" : "false", \
							   m_##name.m_name.cstr()));
#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) \
	ANKI_CHECK(file.writeTextf("\t<!-- %s -->\n", m_##name.m_description.cstr())); \
	ANKI_CHECK(file.writeTextf("\t<%s>%s</%s>\n", m_##name.m_name.cstr(), m_##name.m_value, m_##name.m_name.cstr()));
#include <AnKi/Core/AllConfigVars.defs.h>
#undef ANKI_NUMERIC_UINT

	ANKI_CHECK(file.writeText("</config>\n"));
	return Error::kNone;
}

Error ConfigSet::setFromCommandLineArguments(U32 cmdLineArgsCount, char* cmdLineArgs[])
{
	ANKI_ASSERT(isInitialized());

	for(U i = 0; i < cmdLineArgsCount; ++i)
	{
		ANKI_ASSERT(cmdLineArgs[i]);
		const CString varName = cmdLineArgs[i];

		// Set the value
		++i;
		if(i >= cmdLineArgsCount)
		{
			ANKI_CORE_LOGE("Expecting a command line argument after %s", varName.cstr());
			return Error::kUserData;
		}

		ANKI_ASSERT(cmdLineArgs[i]);
		const CString value = cmdLineArgs[i];

		if(false)
		{
		}

#define ANKI_NUMERIC(type, name) \
	else if(varName == m_##name.m_name) \
	{ \
		type v; \
		ANKI_CHECK(value.toNumber(v)); \
		set##name(v); \
	}

#define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(U8, name)
#define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(U32, name)
#define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(PtrSize, name)
#define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description) ANKI_NUMERIC(F32, name)

#define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description) \
	else if(varName == m_##name.m_name) \
	{ \
		U32 v; \
		ANKI_CHECK(value.toNumber(v)); \
		m_##name.m_value = v != 0; \
	}

#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) \
	else if(varName == m_##name.m_name) \
	{ \
		setStringVar(value, m_##name); \
	}

#include <AnKi/Core/AllConfigVars.defs.h>
#undef ANKI_NUMERIC
		else
		{
			ANKI_CORE_LOGW("Can't recognize command line argument: %s. Skipping", varName.cstr());
		}
	}

	return Error::kNone;
}

} // end namespace anki
