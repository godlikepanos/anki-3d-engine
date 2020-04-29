// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/LuaBinder.h>
#include <anki/util/Logger.h>
#include <anki/util/Tracer.h>

namespace anki
{

// Forward
#define ANKI_SCRIPT_CALL_WRAP(x_) void wrapModule##x_(lua_State*)
ANKI_SCRIPT_CALL_WRAP(Logger);
ANKI_SCRIPT_CALL_WRAP(Math);
ANKI_SCRIPT_CALL_WRAP(Renderer);
ANKI_SCRIPT_CALL_WRAP(Scene);
#undef ANKI_SCRIPT_CALL_WRAP

static void wrapModules(lua_State* l)
{
#define ANKI_SCRIPT_CALL_WRAP(x_) wrapModule##x_(l)
	ANKI_SCRIPT_CALL_WRAP(Logger);
	ANKI_SCRIPT_CALL_WRAP(Math);
	ANKI_SCRIPT_CALL_WRAP(Renderer);
	ANKI_SCRIPT_CALL_WRAP(Scene);
#undef ANKI_SCRIPT_CALL_WRAP
}

static int luaPanic(lua_State* l)
{
	ANKI_SCRIPT_LOGE("Lua panic attack: %s", lua_tostring(l, -1));
	abort();
}

LuaBinder::LuaBinder()
{
}

LuaBinder::~LuaBinder()
{
	if(m_l)
	{
		lua_close(m_l);
	}
	m_userDataSigToDataInfo.destroy(m_alloc);
}

Error LuaBinder::init(ScriptAllocator alloc, LuaBinderOtherSystems* otherSystems)
{
	ANKI_ASSERT(otherSystems);
	m_otherSystems = otherSystems;
	m_alloc = alloc;

	m_l = lua_newstate(luaAllocCallback, this);
	luaL_openlibs(m_l);
	lua_atpanic(m_l, &luaPanic);

	wrapModules(m_l);

	return Error::NONE;
}

void* LuaBinder::luaAllocCallback(void* userData, void* ptr, PtrSize osize, PtrSize nsize)
{
	ANKI_ASSERT(userData);

#if 1
	LuaBinder& binder = *reinterpret_cast<LuaBinder*>(userData);
	void* out = nullptr;

	if(nsize == 0)
	{
		if(ptr != nullptr)
		{
			binder.m_alloc.getMemoryPool().free(ptr);
		}
	}
	else
	{
		// Should be doing something like realloc

		if(ptr == nullptr)
		{
			out = binder.m_alloc.getMemoryPool().allocate(nsize, 16);
		}
		else if(nsize <= osize)
		{
			out = ptr;
		}
		else
		{
			// realloc

			out = binder.m_alloc.getMemoryPool().allocate(nsize, 16);
			memcpy(out, ptr, osize);
			binder.m_alloc.getMemoryPool().free(ptr);
		}
	}
#else
	void* out = nullptr;
	if(nsize == 0)
	{
		free(ptr);
	}
	else
	{
		out = realloc(ptr, nsize);
	}
#endif

	return out;
}

Error LuaBinder::evalString(lua_State* state, const CString& str)
{
	ANKI_TRACE_SCOPED_EVENT(LUA_EXEC);

	Error err = Error::NONE;
	int e = luaL_dostring(state, &str[0]);
	if(e)
	{
		ANKI_SCRIPT_LOGE("%s", lua_tostring(state, -1));
		lua_pop(state, 1);
		err = Error::USER_DATA;
	}

	garbageCollect(state);
	return err;
}

void LuaBinder::createClass(lua_State* l, const LuaUserDataTypeInfo* typeInfo)
{
	ANKI_ASSERT(typeInfo);
	lua_newtable(l); // push new table
	lua_setglobal(l, typeInfo->m_typeName); // pop and make global
	luaL_newmetatable(l, typeInfo->m_typeName); // push
	lua_pushstring(l, "__index"); // push
	lua_pushvalue(l, -2); // pushes copy of the metatable
	lua_settable(l, -3); // pop*2: metatable.__index = metatable

	// After all these the metatable is in the top of tha stack

	// Now, store the typeInfo
	void* ud;
	lua_getallocf(l, &ud);
	ANKI_ASSERT(ud);
	LuaBinder& binder = *static_cast<LuaBinder*>(ud);
	binder.m_userDataSigToDataInfo.emplace(binder.m_alloc, typeInfo->m_signature, typeInfo);
}

void LuaBinder::pushLuaCFuncMethod(lua_State* l, const char* name, lua_CFunction luafunc)
{
	lua_pushstring(l, name);
	lua_pushcfunction(l, luafunc);
	lua_settable(l, -3);
}

void LuaBinder::pushLuaCFuncStaticMethod(lua_State* l, const char* className, const char* name, lua_CFunction luafunc)
{
	lua_getglobal(l, className); // push
	lua_pushcfunction(l, luafunc); // push
	lua_setfield(l, -2, name); // pop global
	lua_pop(l, 1); // pop cfunc
}

void LuaBinder::pushLuaCFunc(lua_State* l, const char* name, lua_CFunction luafunc)
{
	lua_register(l, name, luafunc);
}

Error LuaBinder::checkNumberInternal(lua_State* l, I32 stackIdx, lua_Number& number)
{
	Error err = Error::NONE;
	lua_Number lnum;
	int isnum;

	lnum = lua_tonumberx(l, stackIdx, &isnum);
	if(isnum)
	{
		number = lnum;
	}
	else
	{
		err = Error::USER_DATA;
		lua_pushfstring(l, "Number expected. Got %s", luaL_typename(l, stackIdx));
	}

	return err;
}

Error LuaBinder::checkString(lua_State* l, I32 stackIdx, const char*& out)
{
	Error err = Error::NONE;
	const char* s = lua_tolstring(l, stackIdx, nullptr);
	if(s != nullptr)
	{
		out = s;
	}
	else
	{
		err = Error::USER_DATA;
		lua_pushfstring(l, "String expected. Got %s", luaL_typename(l, stackIdx));
	}

	return err;
}

Error LuaBinder::checkUserData(lua_State* l, I32 stackIdx, const LuaUserDataTypeInfo& typeInfo, LuaUserData*& out)
{
	Error err = Error::NONE;

	void* p = lua_touserdata(l, stackIdx);
	if(p != nullptr)
	{
		out = reinterpret_cast<LuaUserData*>(p);
		if(out->getSig() == typeInfo.m_signature)
		{
			// Check using a LUA method again
			ANKI_ASSERT(luaL_testudata(l, stackIdx, typeInfo.m_typeName) != nullptr
						&& "ANKI type check passes but LUA's type check failed");
		}
		else
		{
			// It's not the correct user data
			err = Error::USER_DATA;
		}
	}
	else
	{
		// It's not user data
		err = Error::USER_DATA;
	}

	if(err)
	{
		lua_pushfstring(l, "Userdata of %s expected. Got %s", typeInfo.m_typeName, luaL_typename(l, stackIdx));
	}

	return err;
}

Error LuaBinder::checkArgsCount(lua_State* l, I argsCount)
{
	const I actualArgsCount = lua_gettop(l);

	if(argsCount != actualArgsCount)
	{
		lua_pushfstring(l, "Expecting %d arguments, got %d", argsCount, actualArgsCount);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

void* LuaBinder::luaAlloc(lua_State* l, size_t size, U32 alignment)
{
	void* ud;
	lua_getallocf(l, &ud);
	ANKI_ASSERT(ud);
	LuaBinder* binder = static_cast<LuaBinder*>(ud);

	return binder->m_alloc.getMemoryPool().allocate(size, alignment);
}

void LuaBinder::luaFree(lua_State* l, void* ptr)
{
	void* ud;
	lua_getallocf(l, &ud);
	ANKI_ASSERT(ud);
	LuaBinder* binder = static_cast<LuaBinder*>(ud);

	binder->m_alloc.getMemoryPool().free(ptr);
}

void LuaBinder::serializeGlobals(lua_State* l, LuaBinderSerializeGlobalsCallback& callback)
{
	ANKI_ASSERT(l);

	lua_pushglobaltable(l);
	lua_pushnil(l);

	while(lua_next(l, -2) != 0)
	{
		// Get type of key and value
		I keyType = lua_type(l, -2);
		I valueType = lua_type(l, -1);

		// Only string keys
		if(keyType != LUA_TSTRING)
		{
			lua_pop(l, 1);
			continue;
		}

		CString keyString = lua_tostring(l, -2);
		if(keyString.isEmpty() || keyString.getLength() == 0 || keyString[0] == '_')
		{
			lua_pop(l, 1);
			continue;
		}

		switch(valueType)
		{
		case LUA_TNUMBER:
		{
			// Write name
			callback.write(keyString.cstr(), keyString.getLength() + 1);

			// Write type
			U32 type = LUA_TNUMBER;
			callback.write(&type, sizeof(type));

			// Write value
			F64 val = lua_tonumber(l, -1);
			callback.write(&val, sizeof(val));

			break;
		}
		case LUA_TSTRING:
		{
			// Write name
			callback.write(keyString.cstr(), keyString.getLength() + 1);

			// Write type
			U32 type = LUA_TSTRING;
			callback.write(&type, sizeof(type));

			// Write str len
			CString val = lua_tostring(l, -1);
			callback.write(val.cstr(), val.getLength() + 1);

			break;
		}
		case LUA_TUSERDATA:
		{
			LuaUserData* ud = static_cast<LuaUserData*>(lua_touserdata(l, -1));
			ANKI_ASSERT(ud);
			LuaUserDataSerializeCallback cb = ud->getDataTypeInfo().m_serializeCallback;
			if(cb)
			{
				Array<U8, 256> buff;
				PtrSize dumpSize;
				cb(*ud, nullptr, dumpSize);
				if(dumpSize <= buff.getSize())
				{
					cb(*ud, &buff[0], dumpSize);
				}
				else
				{
					ANKI_ASSERT(!"TODO");
				}

				// Write name
				callback.write(keyString.cstr(), keyString.getLength() + 1);

				// Write type
				U32 type = LUA_TUSERDATA;
				callback.write(&type, sizeof(type));

				// Write sig
				callback.write(&ud->getDataTypeInfo().m_signature, sizeof(ud->getDataTypeInfo().m_signature));

				// Write data
				PtrSize size = dumpSize;
				callback.write(&size, sizeof(size));
				callback.write(&buff[0], dumpSize);
			}
			else
			{
				ANKI_SCRIPT_LOGW("Can't serialize variable %s. No callback provided", keyString.cstr());
			}

			break;
		}
		}

		lua_pop(l, 1);
	}
}

void LuaBinder::deserializeGlobals(lua_State* l, const void* data, PtrSize dataSize)
{
	ANKI_ASSERT(dataSize > 0 && data);
	const U8* ptr = static_cast<const U8*>(data);
	const U8* end = ptr + dataSize;

	while(ptr < end)
	{
		// Get name
		CString name = reinterpret_cast<const char*>(ptr);
		U32 len = name.getLength();
		ANKI_ASSERT(len > 0);
		ptr += len + 1;

		// Get type
		I type = *reinterpret_cast<const U32*>(ptr);
		ptr += sizeof(U32);

		switch(type)
		{
		case LUA_TNUMBER:
		{
			const F64 val = *reinterpret_cast<const F64*>(ptr);
			ptr += sizeof(F64);
			lua_pushnumber(l, val);
			lua_setglobal(l, name.cstr());
			break;
		}
		case LUA_TSTRING:
		{
			CString val = reinterpret_cast<const char*>(ptr);
			const U len = val.getLength();
			ptr += len + 1;
			ANKI_ASSERT(len > 0);
			lua_pushstring(l, val.cstr());
			lua_setglobal(l, name.cstr());
			break;
		}
		case LUA_TUSERDATA:
		{
			// Get sig
			const I64 sig = *reinterpret_cast<const I64*>(ptr);
			ptr += sizeof(sig);

			// Get input data size
			const PtrSize dataSize = *reinterpret_cast<const PtrSize*>(ptr);
			ptr += sizeof(dataSize);

			// Get the type info
			void* ud;
			lua_getallocf(l, &ud);
			ANKI_ASSERT(ud);
			LuaBinder& binder = *static_cast<LuaBinder*>(ud);
			auto it = binder.m_userDataSigToDataInfo.find(sig);
			ANKI_ASSERT(it != binder.m_userDataSigToDataInfo.getEnd());
			const LuaUserDataTypeInfo* typeInfo = *it;

			// Create user data
			LuaUserData* userData = static_cast<LuaUserData*>(lua_newuserdata(l, typeInfo->m_structureSize));
			userData->initGarbageCollected(typeInfo);
			ANKI_ASSERT(typeInfo->m_deserializeCallback);
			typeInfo->m_deserializeCallback(ptr, *userData);
			ptr += dataSize;
			luaL_setmetatable(l, typeInfo->m_typeName);
			lua_setglobal(l, name.cstr());

			break;
		}
		}
	}
}

} // end namespace anki
