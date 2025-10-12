// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Script/LuaBinder.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Util/CVarSet.h>

namespace anki {

static CVar* findCVar(CString name)
{
	CVar* foundCVar = nullptr;

	CString prefix = "cvar";
	const PtrSize pos = name.find(prefix);
	if(name.getLength() > prefix.getLength() && pos == 0)
	{
		// Possibly a CVAR

		CString cvarName = name.getBegin() + prefix.getLength();
		CVarSet::getSingleton().iterateCVars([&](CVar& cvar) {
			ScriptString cvarName2 = cvar.getName();
			cvarName2.replaceAll(".", "");

			if(cvarName == cvarName2)
			{
				foundCVar = &cvar;
				return FunctorContinue::kStop;
			}

			return FunctorContinue::kContinue;
		});
	}

	return foundCVar;
}

static inline int pwrap__newindex(lua_State* l)
{
	if(LuaBinder::checkArgsCount(l, 3))
	{
		return -1;
	}

	const Char* varName_;
	if(LuaBinder::checkString(l, 2, varName_))
	{
		return -1;
	}
	CString varName(varName_);

	CVar* cvar = findCVar(varName);
	if(cvar)
	{
		// Found

#define ANKI_CVAR_NUMERIC_TYPE(type) \
	case CVarValueType::kNumeric##type: \
	{ \
		type val; \
		if(LuaBinder::checkNumber(l, 3, val)) \
		{ \
			return -1; \
		} \
		static_cast<NumericCVar<type>&>(*cvar) = val; \
	} \
	break;

		switch(cvar->getValueType())
		{
		case CVarValueType::kString:
		{
			const Char* val;
			if(LuaBinder::checkString(l, 3, val))
			{
				return -1;
			}
			static_cast<StringCVar&>(*cvar) = val;
		}
		break;
		case CVarValueType::kBool:
		{
			I32 val;
			if(LuaBinder::checkNumber(l, 3, val))
			{
				return -1;
			}
			static_cast<BoolCVar&>(*cvar) = val;
		}
		break;
			ANKI_CVAR_NUMERIC_TYPE(U8)
			ANKI_CVAR_NUMERIC_TYPE(U16)
			ANKI_CVAR_NUMERIC_TYPE(U32)
			ANKI_CVAR_NUMERIC_TYPE(PtrSize)
			ANKI_CVAR_NUMERIC_TYPE(F32)
			ANKI_CVAR_NUMERIC_TYPE(F64)
		default:
			ANKI_ASSERT(0);
		}

#undef ANKI_CVAR_NUMERIC_TYPE
	}
	else
	{
		ANKI_SCRIPT_LOGE("Global variable not found: %s", varName.cstr());
		return -1;
	}

	return 0;
}

static int wrap__newindex(lua_State* l)
{
	const int res = pwrap__newindex(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

static inline int pwrap__index(lua_State* l)
{
	if(LuaBinder::checkArgsCount(l, 2))
	{
		return -1;
	}

	const Char* varName_;
	if(LuaBinder::checkString(l, 2, varName_))
	{
		return -1;
	}
	CString varName(varName_);

	CVar* cvar = findCVar(varName);
	if(cvar)
	{
		// Found

#define ANKI_CVAR_NUMERIC_TYPE(type) \
	case CVarValueType::kNumeric##type: \
	{ \
		lua_pushnumber(l, lua_Number(static_cast<NumericCVar<type>&>(*cvar))); \
	} \
	break;

		switch(cvar->getValueType())
		{
		case CVarValueType::kString:
		{
			lua_pushstring(l, CString(static_cast<StringCVar&>(*cvar)).cstr());
		}
		break;
		case CVarValueType::kBool:
		{
			lua_pushnumber(l, static_cast<BoolCVar&>(*cvar));
		}
		break;
			ANKI_CVAR_NUMERIC_TYPE(U8)
			ANKI_CVAR_NUMERIC_TYPE(U16)
			ANKI_CVAR_NUMERIC_TYPE(U32)
			ANKI_CVAR_NUMERIC_TYPE(PtrSize)
			ANKI_CVAR_NUMERIC_TYPE(F32)
			ANKI_CVAR_NUMERIC_TYPE(F64)
		default:
			ANKI_ASSERT(0);
		}

#undef ANKI_CVAR_NUMERIC_TYPE
	}
	else
	{
		ANKI_SCRIPT_LOGE("Global variable not found: %s", varName.cstr());
		return -1;
	}

	return 1;
}

static int wrap__index(lua_State* l)
{
	const int res = pwrap__index(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

void wrapModuleGlobals(lua_State* l)
{
	lua_newtable(l); // push table

	lua_newtable(l); // push meta_table

	lua_pushcfunction(l, wrap__index); // push func
	lua_setfield(l, -2, "__index"); // meta_table[__index] = stack[-2] and pop func

	lua_pushcfunction(l, wrap__newindex); // push func
	lua_setfield(l, -2, "__newindex"); // meta_table[__newindex] = stack[-2] and pop func

	lua_setmetatable(l, -2); // setmetatable(table, meta_table) and pop meta_table

	lua_setglobal(l, "g"); // setglobal(table, "g") and pop table

	lua_settop(l, 0);
}

} // namespace anki
