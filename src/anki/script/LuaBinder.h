// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/script/Common.h>
#include <anki/util/Assert.h>
#include <anki/util/StdTypes.h>
#include <anki/util/Allocator.h>
#include <anki/util/String.h>
#include <anki/util/Functions.h>
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#error "Wrong LUA header included"
#endif

namespace anki
{

/// LUA userdata.
class UserData
{
public:
	/// @note NEVER ADD A DESTRUCTOR. LUA cannot call that.
	~UserData() = delete;

	I64 getSig() const
	{
		return m_sig;
	}

	void initGarbageCollected(I64 sig)
	{
		m_sig = sig;
		m_addressAndGarbageCollect = GC_MASK;
	}

	void initPointed(I64 sig, void* ptrToObject)
	{
		m_sig = sig;
		U64 addr = ptrToNumber(ptrToObject);
		ANKI_ASSERT((addr & GC_MASK) == 0 && "Address too high, cannot encode a flag");
		m_addressAndGarbageCollect = addr;
	}

	Bool isGarbageCollected() const
	{
		return m_addressAndGarbageCollect == GC_MASK;
	}

	template<typename T>
	T* getData()
	{
		T* out = nullptr;
		if(isGarbageCollected())
		{
			// Garbage collected
			PtrSize mem = ptrToNumber(this);
			mem += getAlignedRoundUp(alignof(T), sizeof(UserData));
			out = numberToPtr<T*>(mem);
		}
		else
		{
			// Pointed
			PtrSize mem = static_cast<PtrSize>(m_addressAndGarbageCollect);
			out = numberToPtr<T*>(mem);
		}

		ANKI_ASSERT(out);
		ANKI_ASSERT(isAligned(alignof(T), out));
		return out;
	}

	template<typename T>
	static PtrSize computeSizeForGarbageCollected()
	{
		return getAlignedRoundUp(alignof(T), sizeof(UserData)) + sizeof(T);
	}

private:
	static constexpr U64 GC_MASK = U64(1) << U64(63);

	I64 m_sig = 0; ///< Signature to identify the user data.

	/// Encodes an address and a flag for garbage collection.
	/// High bit is the GC flag and the rest are an address.
	U64 m_addressAndGarbageCollect = 0;
};

/// Lua binder class. A wrapper on top of LUA
class LuaBinder
{
public:
	template<typename T>
	using Allocator = ChainAllocator<T>;

	LuaBinder();
	~LuaBinder();

	ANKI_USE_RESULT Error create(Allocator<U8>& alloc, void* parent);

	lua_State* getLuaState()
	{
		return m_l;
	}

	Allocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	void* getParent() const
	{
		return m_parent;
	}

	/// Expose a variable to the lua state
	template<typename T>
	void exposeVariable(const char* name, T* y);

	/// Evaluate a string
	ANKI_USE_RESULT Error evalString(const CString& str);

	/// For debugging purposes
	static void stackDump(lua_State* l);

	/// Make sure that the arguments match the argsCount number
	static void checkArgsCount(lua_State* l, I argsCount);

	/// Create a new LUA class
	static void createClass(lua_State* l, const char* className);

	/// Add new function in a class that it's already in the stack
	static void pushLuaCFuncMethod(lua_State* l, const char* name, lua_CFunction luafunc);

	/// Add a new static function in the class.
	static void pushLuaCFuncStaticMethod(lua_State* l, const char* className, const char* name, lua_CFunction luafunc);

	/// Add a new function.
	static void pushLuaCFunc(lua_State* l, const char* name, lua_CFunction luafunc);

	/// Get a number from the stack.
	template<typename TNumber>
	static ANKI_USE_RESULT Error checkNumber(lua_State* l, I stackIdx, TNumber& number);

	/// Get a string from the stack.
	static ANKI_USE_RESULT Error checkString(lua_State* l, I32 stackIdx, const char*& out);

	/// Get some user data from the stack.
	/// The function uses the type signature to validate the type and not the
	/// typeName. That is supposed to be faster.
	static ANKI_USE_RESULT Error checkUserData(
		lua_State* l, I32 stackIdx, const char* typeName, I64 typeSignature, UserData*& out);

	/// Allocate memory.
	static void* luaAlloc(lua_State* l, size_t size, U32 alignment);

	/// Free memory.
	static void luaFree(lua_State* l, void* ptr);

	template<typename TWrapedType>
	static I64 getWrappedTypeSignature();

	template<typename TWrapedType>
	static const char* getWrappedTypeName();

private:
	Allocator<U8> m_alloc;
	lua_State* m_l = nullptr;
	void* m_parent = nullptr; ///< Point to the ScriptManager

	static void* luaAllocCallback(void* userData, void* ptr, PtrSize osize, PtrSize nsize);

	static ANKI_USE_RESULT Error checkNumberInternal(lua_State* l, I32 stackIdx, lua_Number& number);
};

template<typename TNumber>
inline Error LuaBinder::checkNumber(lua_State* l, I stackIdx, TNumber& number)
{
	lua_Number lnum;
	Error err = checkNumberInternal(l, stackIdx, lnum);
	if(!err)
	{
		number = lnum;
	}

	return err;
}

template<typename T>
inline void LuaBinder::exposeVariable(const char* name, T* y)
{
	void* ptr = lua_newuserdata(m_l, sizeof(UserData));
	UserData* ud = static_cast<UserData*>(ptr);
	ud->initPointed(getWrappedTypeSignature<T>(), y);
	luaL_setmetatable(m_l, getWrappedTypeName<T>());
	lua_setglobal(m_l, name);
}

} // end namespace anki
