// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Thread.h>

namespace anki {

#if ANKI_ASSERTIONS_ENABLED
void CVar::validateSetValue() const
{
	ANKI_ASSERT(Thread::getCurrentThreadId() == CVarSet::getSingleton().m_mainThreadHandle && "CVars can only be set by the main thread");
}
#endif

void CVarSet::registerCVar(CVar* cvar)
{
#if ANKI_ASSERTIONS_ENABLED
	m_mainThreadHandle = Thread::getCurrentThreadId();
#endif

	for([[maybe_unused]] CVar& it : m_cvars)
	{
		ANKI_ASSERT(it.m_name != cvar->m_name);
	}

	m_cvars.pushBack(cvar);
}

Error CVarSet::setMultiple(ConstWeakArray<const Char*> arr)
{
	for(U32 i = 0; i < arr.getSize(); ++i)
	{
		ANKI_ASSERT(arr[i]);
		const CString varName = arr[i];

		// Get the value string
		++i;
		if(i >= arr.getSize())
		{
			ANKI_UTIL_LOGE("Expecting a value after %s", varName.cstr());
			return Error::kUserData;
		}
		ANKI_ASSERT(arr[i]);
		const CString value = arr[i];

		// Find the CVar
		CVar* foundCVar = nullptr;
		for(CVar& it : m_cvars)
		{
			if(it.m_name == varName)
			{
				foundCVar = &it;
			}
		}

		if(foundCVar)
		{
#define ANKI_CVAR_NUMERIC_SET(type) \
	case CVarValueType::kNumeric##type: \
	{ \
		type v; \
		err = value.toNumber(v); \
		if(!err) \
		{ \
			static_cast<NumericCVar<type>&>(*foundCVar) = v; \
		} \
		break; \
	}

			Error err = Error::kNone;
			switch(foundCVar->m_type)
			{
			case CVarValueType::kString:
				static_cast<StringCVar&>(*foundCVar) = value;
				break;
			case CVarValueType::kBool:
			{
				U32 v;
				err = value.toNumber(v);
				if(!err)
				{
					static_cast<BoolCVar&>(*foundCVar) = (v != 0);
				}
				break;
			}
				ANKI_CVAR_NUMERIC_SET(U8)
				ANKI_CVAR_NUMERIC_SET(U16)
				ANKI_CVAR_NUMERIC_SET(U32)
				ANKI_CVAR_NUMERIC_SET(PtrSize)
				ANKI_CVAR_NUMERIC_SET(F32)
				ANKI_CVAR_NUMERIC_SET(F64)
			default:
				ANKI_ASSERT(0);
			}

			if(err)
			{
				ANKI_UTIL_LOGE("Wrong value for %s. Value will not be set", foundCVar->m_name.cstr());
			}
		}
		else
		{
			ANKI_UTIL_LOGE("Can't recognize command line argument: %s. Skipping", varName.cstr());
		}
	}

	return Error::kNone;
}

} // end namespace anki
