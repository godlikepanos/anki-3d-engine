#ifndef ANKI_SCRIPT_LUA_BINDER_H
#define ANKI_SCRIPT_LUA_BINDER_H

#include "anki/util/Assert.h"
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#	error "See file"
#endif
#include <cstdint>

namespace anki {

/// Internal lua stuff
namespace lua_detail {

//==============================================================================
/// lua userdata
struct UserData
{
	void* ptr = nullptr;
	bool gc = false; ///< Garbage collection on?
};

//==============================================================================
/// Class proxy
template<typename Class>
struct ClassProxy
{
	static const char* NAME;

	static const char* getName()
	{
		ANKI_ASSERT(NAME != nullptr);
		return NAME;
	}
};

template<typename Class>
const char* ClassProxy<Class>::NAME = nullptr;

//==============================================================================
extern void checkArgsCount(lua_State* l, int argsCount);

//==============================================================================
/// Used mainly to push a method's return value to the stack
template<typename Class>
struct PushStack
{
	void operator()(lua_State* l, Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = new Class(x);
		d->gc = true;
	}
};

// Specialization ref
template<typename Class>
struct PushStack<Class&>
{
	void operator()(lua_State* l, Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = &x;
		d->gc = false;
	}
};

// Specialization const ref
template<typename Class>
struct PushStack<const Class&>
{
	void operator()(lua_State* l, const Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = &const_cast<Class&>(x);
		d->gc = false;
	}
};

// Specialization ptr
template<typename Class>
struct PushStack<Class*>
{
	void operator()(lua_State* l, Class* x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = x;
		d->gc = false;
	}
};

// Specialization const ptr
template<typename Class>
struct PushStack<const Class*>
{
	void operator()(lua_State* l, Class* x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = const_cast<Class*>(x);
		d->gc = false;
	}
};

// Specialization const char*
template<>
struct PushStack<const char*>
{
	void operator()(lua_State* l, const char* x)
	{
		lua_pushstring(l, x);
	}
};

#define ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(Type_, luafunc_) \
	template<> \
	struct PushStack<Type_> \
	{ \
		void operator()(lua_State* l, Type_ x) \
		{ \
			luafunc_(l, x); \
		} \
	};

ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(int8_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(int16_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(int32_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(int64_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(uint8_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(uint16_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(uint32_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(uint64_t, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(double, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(float, lua_pushnumber)

//==============================================================================
/// Used to get the function arguments from the stack
template<typename Class, int stackIndex>
struct StackGet
{
	Class operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->ptr);
		return Class(*a);
	}
};

// Specialization const ref
template<typename Class, int stackIndex>
struct StackGet<const Class&, stackIndex>
{
	const Class& operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->ptr);
		return *a;
	}
};

// Specialization ref
template<typename Class, int stackIndex>
struct StackGet<Class&, stackIndex>
{
	Class& operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		Class* a = reinterpret_cast<Class*>(udata->ptr);
		return *a;
	}
};

// Specialization const ptr
template<typename Class, int stackIndex>
struct StackGet<const Class*, stackIndex>
{
	const Class* operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->ptr);
		return a;
	}
};

// Specialization ptr
template<typename Class, int stackIndex>
struct StackGet<Class*, stackIndex>
{
	Class* operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		Class* a = reinterpret_cast<Class*>(udata->ptr);
		return a;
	}
};

#define ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(Type_, luafunc_) \
	template<int stackIndex> \
	struct StackGet<Type_, stackIndex> \
	{ \
		Type_ operator()(lua_State* l) \
		{ \
			return luafunc_(l, stackIndex); \
		} \
	};

ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(float, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(double, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(int8_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(int16_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(int32_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(int64_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(uint8_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(uint16_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(uint32_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(uint64_t, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(const char*, luaL_checkstring)

//==============================================================================
/// Call a method
template<typename T>
struct CallMethod;

// R (_1)
template<typename Class, typename TReturn, typename Arg0>
struct CallMethod<TReturn (Class::*)(Arg0)>
{
	int operator()(lua_State* l, Class* obj, TReturn (Class::* method)(Arg0))
	{
		TReturn out = (obj->*method)(StackGet<Arg0, 2>()(l));
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1) const
template<typename Class, typename TReturn, typename Arg0>
struct CallMethod<TReturn (Class::*)(Arg0) const>
{
	int operator()(lua_State* l, const Class* obj, 
		TReturn (Class::* method)(Arg0) const)
	{
		TReturn out = (obj->*method)(StackGet<Arg0, 2>()(l));
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1, _2)
template<typename Class, typename TReturn, typename Arg0, typename Arg1>
struct CallMethod<TReturn (Class::*)(Arg0, Arg1)>
{
	int operator()(lua_State* l, Class* obj, 
		TReturn (Class::* method)(Arg0, Arg1))
	{
		TReturn out = (obj->*method)(StackGet<Arg0, 2>()(l), 
			StackGet<Arg1, 3>()(l));
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

// void (_1)
template<typename Class, typename Arg0>
struct CallMethod<void (Class::*)(Arg0)>
{
	int operator()(lua_State* l, Class* obj, void (Class::* method)(Arg0))
	{
		(obj->*method)(StackGet<Arg0, 2>()(l));
		return 0;
	}
};

// void (_1, _2)
template<typename Class, typename Arg0, typename Arg1>
struct CallMethod<void (Class::*)(Arg0, Arg1)>
{
	int operator()(lua_State* l, Class* obj, void (Class::* method)(Arg0, Arg1))
	{
		(obj->*method)(StackGet<Arg0, 2>()(l), StackGet<Arg1, 3>()(l));
		return 0;
	}
};

// void (void)
template<typename Class>
struct CallMethod<void (Class::*)(void)>
{
	int operator()(lua_State* /*l*/, Class* obj, void (Class::* method)(void))
	{
		(obj->*method)();
		return 0;
	}
};

// R (void)
template<typename Class, typename TReturn>
struct CallMethod<TReturn (Class::*)(void)>
{
	int operator()(lua_State* l, Class* obj, TReturn (Class::* method)(void))
	{
		TReturn out = (obj->*method)();
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

//==============================================================================
/// Call a function
template<typename T>
struct CallFunction;

// R (_1)
template<typename TReturn, typename Arg0>
struct CallFunction<TReturn (*)(Arg0)>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l));
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1, _2)
template<typename TReturn, typename Arg0, typename Arg1>
struct CallFunction<TReturn (*)(Arg0, Arg1)>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0, Arg1))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l), 
			StackGet<Arg1, 2>()(l));
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

// void (_1)
template<typename Arg0>
struct CallFunction<void (*)(Arg0)>
{
	int operator()(lua_State* l, void (*func)(Arg0))
	{
		(*func)(StackGet<Arg0, 1>()(l));
		return 0;
	}
};

// void (_1, _2)
template<typename Arg0, typename Arg1>
struct CallFunction<void (*)(Arg0, Arg1)>
{
	int operator()(lua_State* l, void (*func)(Arg0, Arg1))
	{
		(*func)(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l));
		return 0;
	}
};

// R (void)
template<typename TReturn>
struct CallFunction<TReturn (*)(void)>
{
	int operator()(lua_State* l, TReturn (*func)(void))
	{
		TReturn out = (*func)();
		PushStack<TReturn> ps;
		ps(l, out);
		return 1;
	}
};

// void (void)
template<>
struct CallFunction<void (*)(void)>
{
	int operator()(lua_State* /*l*/, void (*func)(void))
	{
		(*func)();
		return 0;
	}
};

//==============================================================================
/// Call constructor
template<typename Class, typename... Args>
struct CallConstructor;

// none
template<typename Class>
struct CallConstructor<Class>
{
	Class* operator()(lua_State*)
	{
		return new Class();
	}
};

// _1
template<typename Class, typename Arg0>
struct CallConstructor<Class, Arg0>
{
	Class* operator()(lua_State* l)
	{
		return new Class(StackGet<Arg0, 1>()(l));
	}
};

// _1, _2
template<typename Class, typename Arg0, typename Arg1>
struct CallConstructor<Class, Arg0, Arg1>
{
	Class* operator()(lua_State* l)
	{
		return new Class(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l));
	}
};

//==============================================================================
/// Signature for constructor
template<typename Class, typename... Args>
struct ConstructorSignature
{
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(Args));
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));

		luaL_setmetatable(l, ClassProxy<Class>::getName());

		d->ptr = CallConstructor<Class, Args...>()(l);
		d->gc = true;
		return 1;
	}
};

//==============================================================================
/// Destructor signature
template<typename Class>
struct DestructorSignature
{
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, 1);
		UserData* d = (UserData*)luaL_checkudata(l, 1, 
			ClassProxy<Class>::getName());
		if(d->gc)
		{
			Class* obj = reinterpret_cast<Class*>(d->ptr);
			delete obj;
		}
		return 0;
	}
};

//==============================================================================
/// Method signature
template<typename T>
struct MethodSignature;

template<typename Class, typename TReturn, typename... Args>
struct MethodSignature<TReturn (Class::*)(Args...)>
{
	template<TReturn (Class::* method)(Args...)>
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(Args) + 1);
		UserData* d = (UserData*)luaL_checkudata(l, 1, 
			ClassProxy<Class>::getName());
		Class* obj = reinterpret_cast<Class*>(d->ptr);
		CallMethod<decltype(method)> cm;
		return cm(l, obj, method);
	}
};

template<typename Class, typename TReturn, typename... Args>
struct MethodSignature<TReturn (Class::*)(Args...) const>
{
	template<TReturn (Class::* method)(Args...) const>
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(Args) + 1);
		UserData* d = (UserData*)luaL_checkudata(l, 1, 
			ClassProxy<Class>::getName());
		const Class* obj = reinterpret_cast<const Class*>(d->ptr);
		CallMethod<decltype(method)> cm;
		return cm(l, obj, method);
	}
};

//==============================================================================
/// Function signature
template<typename T>
struct FunctionSignature;

template<typename TReturn, typename... Args>
struct FunctionSignature<TReturn (*)(Args...)>
{
	template<TReturn (* func)(Args...)>
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(Args));
		auto ff = func; // A hack that saves GCC
		CallFunction<decltype(ff)> cf;
		return cf(l, func);
	}
};

} // end namespace lua_detail

//==============================================================================
// Macros

#define ANKI_LUA_DESTRUCTOR() \
	lua_pushstring(l_, "__gc"); \
	lua_pushcfunction(l_, (&lua_detail::DestructorSignature<Class>::luafunc)); \
	lua_settable(l_, -3);

#define ANKI_LUA_CLASS_BEGIN(luaBinder_, Class_) { \
	typedef Class_ Class; \
	lua_State* l_ = luaBinder_._getLuaState(); \
	lua_detail::ClassProxy<Class>::NAME = #Class_; \
	lua_newtable(l_); \
	lua_setglobal(l_, lua_detail::ClassProxy<Class>::getName()); \
	luaL_newmetatable(l_, lua_detail::ClassProxy<Class>::getName()); \
	lua_pushstring(l_, "__index"); \
	lua_pushvalue(l_, -2);  /* pushes the metatable */ \
	lua_settable(l_, -3);  /* metatable.__index = metatable */ \
	ANKI_LUA_DESTRUCTOR()

#define ANKI_LUA_CLASS_END() }

#define ANKI_LUA_CONSTRUCTOR(...) \
	lua_getglobal(l_, lua_detail::ClassProxy<Class>::getName()); \
	lua_pushcfunction(l_, \
		(&lua_detail::ConstructorSignature<Class, __VA_ARGS__>::luafunc)); \
	lua_setfield(l_, -2, "new"); \
	lua_pop(l_, 1);

#define ANKI_LUA_STATIC_METHOD(name_, smethodPtr_) \
	lua_getglobal(l_, lua_detail::ClassProxy<Class>::getName()); \
	lua_pushcfunction(l_, \
		(&lua_detail::FunctionSignature< \
		decltype(smethodPtr_)>::luafunc<smethodPtr_>)); \
	lua_setfield(l_, -2, name_); \
	lua_pop(l_, 1);

#define ANKI_LUA_METHOD(name_, methodPtr_) \
	lua_pushstring(l_, name_); \
	lua_pushcfunction(l_, \
		(&lua_detail::MethodSignature<decltype(methodPtr_)>:: \
		 luafunc<methodPtr_>)); \
	lua_settable(l_, -3);

#define ANKI_LUA_FUNCTION_AS_METHOD(name_, funcPtr_) \
	lua_pushstring(l_, name_); \
	lua_pushcfunction(l_, (&lua_detail::FunctionSignature< \
		decltype(funcPtr_)>::luafunc<funcPtr_>)); \
	lua_settable(l_, -3);

//==============================================================================
/// Lua binder class. A wrapper on ton of LUA
class LuaBinder
{
public:
	LuaBinder();
	~LuaBinder();

	/// @name Accessors
	/// {
	lua_State* _getLuaState()
	{
		return l;
	}
	/// }

	/// Expose a variable to the lua state
	template<typename T>
	void exposeValue(T* y);

	/// Evaluate a file
	void evalFile(const char* filename);
	/// Evaluate a string
	void evalString(const char* str);

	/// For debugging purposes
	static void stackDump(lua_State* l);

private:
	lua_State* l = nullptr;
};

} // end namespace anki

#endif
