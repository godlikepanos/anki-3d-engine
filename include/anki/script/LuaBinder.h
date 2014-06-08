// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCRIPT_LUA_BINDER_H
#define ANKI_SCRIPT_LUA_BINDER_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Allocator.h"
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#	error "Wrong LUA header included"
#endif
#include <functional>

namespace anki {

//==============================================================================
// Flags
enum BinderFlag: U8
{
	NONE = 0,
	TRANFER_OWNERSHIP = 1 ///< Transfer owneship to LUA garbage collector
};

inline BinderFlag operator|(BinderFlag a, BinderFlag b)
{
	typedef std::underlying_type<BinderFlag>::type Int;
	return static_cast<BinderFlag>(static_cast<Int>(a) | static_cast<Int>(b));
}

inline BinderFlag operator&(BinderFlag a, BinderFlag b)
{
	typedef std::underlying_type<BinderFlag>::type Int;
	return static_cast<BinderFlag>(static_cast<Int>(a) & static_cast<Int>(b));
}

/// Lua binder class. A wrapper on top of LUA
class LuaBinder
{
public:
	template<typename T>
	using Allocator = HeapAllocator<T>;

	LuaBinder(Allocator<U8>& alloc);
	~LuaBinder();

	/// @name Accessors
	/// {
	lua_State* _getLuaState()
	{
		return m_l;
	}

	Allocator<U8> _getAllocator() const
	{
		return m_alloc;
	}
	/// }

	/// Expose a variable to the lua state
	template<typename T>
	void exposeVariable(const char* name, T* y);

	/// Evaluate a file
	void evalFile(const char* filename);
	/// Evaluate a string
	void evalString(const char* str);

	/// For debugging purposes
	static void stackDump(lua_State* l);

private:
	Allocator<U8> m_alloc;
	lua_State* m_l = nullptr;

	static void* luaAllocCallback(
		void* userData, void* ptr, PtrSize osize, PtrSize nsize);
};

/// Internal lua stuff
namespace detail {

//==============================================================================
/// lua userdata
class UserData
{
public:
	void* m_ptr = nullptr;
	Bool8 m_gc = false; ///< Garbage collection on?
};

//==============================================================================
/// Class identification
template<typename Class>
struct ClassProxy
{
	static const char* NAME; ///< Used to check the signature of the user data

	static void setName(const char* name)
	{
		ANKI_ASSERT(NAME == nullptr && "Class already wrapped elsewhere");
		NAME = name;
	}

	static const char* getName()
	{
		ANKI_ASSERT(NAME != nullptr && "Class not wrapped");
		return NAME;
	}
};

template<typename Class>
const char* ClassProxy<Class>::NAME = nullptr;

//==============================================================================

/// Make sure that the arguments match the argsCount number
void checkArgsCount(lua_State* l, I argsCount);

/// Create a new LUA class
void createClass(lua_State* l, const char* className);

/// Add new function in a class that it's already in the stack
void pushCFunctionMethod(lua_State* l, const char* name,
	lua_CFunction luafunc);

/// Add a new static function in the class
void pushCFunctionStatic(lua_State* l, const char* className,
	const char* name, lua_CFunction luafunc);

//==============================================================================
/// Used mainly to push a method's return value to the stack
template<typename Class, BinderFlag flags>
struct PushStack
{
	void operator()(lua_State* l, Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());

		LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
		ANKI_ASSERT(binder);
		LuaBinder::Allocator<U8> alloc = binder->_getAllocator();

		d->m_ptr = alloc.template newInstance<Class>(x);
		d->m_gc = true;
	}
};

// Specialization ref
template<typename Class, BinderFlag flags>
struct PushStack<Class&, flags>
{
	void operator()(lua_State* l, Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->m_ptr = &x;
		d->m_gc = (flags & BinderFlag::TRANFER_OWNERSHIP) != BinderFlag::NONE;
	}
};

// Specialization const ref
template<typename Class, BinderFlag flags>
struct PushStack<const Class&, flags>
{
	void operator()(lua_State* l, const Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->m_ptr = &const_cast<Class&>(x);
		d->m_gc = (flags & BinderFlag::TRANFER_OWNERSHIP) != BinderFlag::NONE;
	}
};

// Specialization ptr
template<typename Class, BinderFlag flags>
struct PushStack<Class*, flags>
{
	void operator()(lua_State* l, Class* x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->m_ptr = x;
		d->m_gc = (flags & BinderFlag::TRANFER_OWNERSHIP) != BinderFlag::NONE;
	}
};

// Specialization const ptr
template<typename Class, BinderFlag flags>
struct PushStack<const Class*, flags>
{
	void operator()(lua_State* l, Class* x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->m_ptr = const_cast<Class*>(x);
		d->m_gc = (flags & BinderFlag::TRANFER_OWNERSHIP) != BinderFlag::NONE;
	}
};

// Specialization const char*
template<BinderFlag flags>
struct PushStack<const char*, flags>
{
	void operator()(lua_State* l, const char* x)
	{
		lua_pushstring(l, x);
	}
};

#define ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(Type_, luafunc_) \
	template<BinderFlag flags> \
	struct PushStack<Type_, flags> \
	{ \
		void operator()(lua_State* l, Type_ x) \
		{ \
			luafunc_(l, x); \
		} \
	};

ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I8, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I16, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I32, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I64, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U8, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U16, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U32, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U64, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(F64, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(F32, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(Bool, lua_pushnumber)

//==============================================================================
/// Used to get the function arguments from the stack
template<typename Class, I stackIndex>
struct StackGet
{
	Class operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->m_ptr);
		return Class(*a);
	}
};

// Specialization const ref
template<typename Class, I stackIndex>
struct StackGet<const Class&, stackIndex>
{
	const Class& operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->m_ptr);
		return *a;
	}
};

// Specialization ref
template<typename Class, I stackIndex>
struct StackGet<Class&, stackIndex>
{
	Class& operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		Class* a = reinterpret_cast<Class*>(udata->m_ptr);
		return *a;
	}
};

// Specialization const ptr
template<typename Class, I stackIndex>
struct StackGet<const Class*, stackIndex>
{
	const Class* operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->m_ptr);
		return a;
	}
};

// Specialization ptr
template<typename Class, I stackIndex>
struct StackGet<Class*, stackIndex>
{
	Class* operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		Class* a = reinterpret_cast<Class*>(udata->m_ptr);
		return a;
	}
};

#define ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(Type_, luafunc_) \
	template<I stackIndex> \
	struct StackGet<Type_, stackIndex> \
	{ \
		Type_ operator()(lua_State* l) \
		{ \
			return luafunc_(l, stackIndex); \
		} \
	}; \
	template<I stackIndex> \
	struct StackGet<Type_&&, stackIndex> \
	{ \
		Type_ operator()(lua_State* l) \
		{ \
			return luafunc_(l, stackIndex); \
		} \
	}; \
	template<I stackIndex> \
	struct StackGet<Type_&, stackIndex> \
	{ \
		Type_ operator()(lua_State* l) \
		{ \
			return luafunc_(l, stackIndex); \
		} \
	}; \
	template<I stackIndex> \
	struct StackGet<Type_*, stackIndex> \
	{ \
		Type_ operator()(lua_State* l) \
		{ \
			return luafunc_(l, stackIndex); \
		} \
	};

ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(F32, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(F64, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I8, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I16, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I32, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I64, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U8, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U16, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U32, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U64, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(const char*, luaL_checkstring)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(Bool, luaL_checkunsigned)

//==============================================================================
/// Call a function
template<typename T, BinderFlag flags>
struct CallFunction;

// R (_1)
template<typename TReturn, typename Arg0, BinderFlag flags>
struct CallFunction<TReturn (*)(Arg0), flags>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l));
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1, _2)
template<typename TReturn, typename Arg0, typename Arg1, BinderFlag flags>
struct CallFunction<TReturn (*)(Arg0, Arg1), flags>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0, Arg1))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l), 
			StackGet<Arg1, 2>()(l));
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1, _2, _3)
template<typename TReturn, typename Arg0, typename Arg1, typename Arg2,
	BinderFlag flags>
struct CallFunction<TReturn (*)(Arg0, Arg1, Arg2), flags>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0, Arg1, Arg2))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l),
			StackGet<Arg1, 2>()(l), StackGet<Arg2, 3>()(l));
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// void (_1)
template<typename Arg0, BinderFlag flags>
struct CallFunction<void (*)(Arg0), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0))
	{
		(*func)(StackGet<Arg0, 1>()(l));
		return 0;
	}
};

// void (_1, _2)
template<typename Arg0, typename Arg1, BinderFlag flags>
struct CallFunction<void (*)(Arg0, Arg1), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0, Arg1))
	{
		(*func)(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l));
		return 0;
	}
};

// void (_1, _2, _3)
template<typename Arg0, typename Arg1, typename Arg2, BinderFlag flags>
struct CallFunction<void (*)(Arg0, Arg1, Arg2), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0, Arg1, Arg2))
	{
		(*func)(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l),
			StackGet<Arg2, 3>()(l));
		return 0;
	}
};

// void (_1, _2, _3, _4)
template<typename Arg0, typename Arg1, typename Arg2, typename Arg3, 
	BinderFlag flags>
struct CallFunction<void (*)(Arg0, Arg1, Arg2, Arg3), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0, Arg1, Arg2, Arg3))
	{
		(*func)(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l),
			StackGet<Arg2, 3>()(l), StackGet<Arg3, 4>()(l));
		return 0;
	}
};

// R (void)
template<typename TReturn, BinderFlag flags>
struct CallFunction<TReturn (*)(void), flags>
{
	int operator()(lua_State* l, TReturn (*func)(void))
	{
		TReturn out = (*func)();
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// void (void)
template<BinderFlag flags>
struct CallFunction<void (*)(void), flags>
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
	Class* operator()(lua_State* l)
	{
		LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
		ANKI_ASSERT(binder);
		LuaBinder::Allocator<U8> alloc = binder->_getAllocator();
		return alloc.template newInstance<Class>();
	}
};

// _1
template<typename Class, typename Arg0>
struct CallConstructor<Class, Arg0>
{
	Class* operator()(lua_State* l)
	{
		LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
		ANKI_ASSERT(binder);
		LuaBinder::Allocator<U8> alloc = binder->_getAllocator();
		return alloc.template newInstance<Class>(StackGet<Arg0, 1>()(l));
	}
};

// _1, _2
template<typename Class, typename Arg0, typename Arg1>
struct CallConstructor<Class, Arg0, Arg1>
{
	Class* operator()(lua_State* l)
	{
		LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
		ANKI_ASSERT(binder);
		LuaBinder::Allocator<U8> alloc = binder->_getAllocator();
		return alloc.template newInstance<Class>(StackGet<Arg0, 1>()(l),
			StackGet<Arg1, 2>()(l));
	}
};

// _1, _2, _3
template<typename Class, typename Arg0, typename Arg1, typename Arg2>
struct CallConstructor<Class, Arg0, Arg1, Arg2>
{
	Class* operator()(lua_State* l)
	{
		LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
		ANKI_ASSERT(binder);
		LuaBinder::Allocator<U8> alloc = binder->_getAllocator();
		return alloc.template newInstance<Class>(StackGet<Arg0, 1>()(l),
			StackGet<Arg1, 2>()(l), StackGet<Arg2, 3>()(l));
	}
};

// _1, _2, _3, _4
template<typename Class, typename Arg0, typename Arg1, typename Arg2,
	typename Arg3>
struct CallConstructor<Class, Arg0, Arg1, Arg2, Arg3>
{
	Class* operator()(lua_State* l)
	{
		LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
		ANKI_ASSERT(binder);
		LuaBinder::Allocator<U8> alloc = binder->_getAllocator();
		return alloc.template newInstance<Class>(StackGet<Arg0, 1>()(l),
			StackGet<Arg1, 2>()(l), StackGet<Arg2, 3>()(l),
			StackGet<Arg3, 4>()(l));
	}
};

//==============================================================================
/// Make a method function. Used like this:
/// @code
/// Foo foo; // An instance
/// MethodFunctionalizer<decltype(&Foo::bar), &Foo::bar>::func(&foo, 123);
/// // Equivelent of:
/// foo.bar(123);
/// @endcode
template <typename T, T> struct MethodFunctionalizer;

template <typename T, typename R, typename ...Args, R (T::* mf)(Args...)>
struct MethodFunctionalizer<R (T::*)(Args...), mf>
{
    static R func(T* obj, Args&&... args)
    {
        return (obj->*mf)(std::forward<Args>(args)...);
    }
};

template <typename T, typename R, typename ...Args, R (T::* mf)(Args...) const>
struct MethodFunctionalizer<R (T::*)(Args...) const, mf>
{
    static R func(const T* obj, Args&&... args)
    {
        return (obj->*mf)(std::forward<Args>(args)...);
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

		d->m_ptr = CallConstructor<Class, Args...>()(l);
		d->m_gc = true;
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
		if(d->m_gc)
		{
			Class* obj = reinterpret_cast<Class*>(d->m_ptr);

			LuaBinder* binder = (LuaBinder*)lua_getuserdata(l);
			ANKI_ASSERT(binder);
			LuaBinder::Allocator<U8> alloc = binder->_getAllocator();
			alloc.deleteInstance(obj);
		}
		return 0;
	}
};

//==============================================================================
/// Function signature
template<BinderFlag flags, typename T, T>
struct FunctionSignature;

template<BinderFlag flags, typename TReturn, typename... TArgs, 
	TReturn (*f)(TArgs...)>
struct FunctionSignature<flags, TReturn (*)(TArgs...), f>
{
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(TArgs));
		CallFunction<decltype(f), flags> cf;
		return cf(l, f);
	}
};

} // end namespace detail

//==============================================================================
// Macros

/// Don't use it directly
#define ANKI_LUA_DESTRUCTOR() \
	detail::pushCFunctionMethod(l_, "__gc", \
		&detail::DestructorSignature<Class>::luafunc);

/// Start wrapping a class. Don't add a destructor (if for example the class 
/// has a private derstructor)
#define ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(luaBinder_, Class_) { \
	typedef Class_ Class; \
	lua_State* l_ = luaBinder_._getLuaState(); \
	detail::ClassProxy<Class>::setName(#Class_); \
	detail::createClass(l_, detail::ClassProxy<Class>::getName());

/// Start wrapping a class
#define ANKI_LUA_CLASS_BEGIN(luaBinder_, Class_) \
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(luaBinder_, Class_) \
	ANKI_LUA_DESTRUCTOR()

/// End wrapping a class
#define ANKI_LUA_CLASS_END() lua_settop(l_, 0); }

/// Define a constructor. Call it from lua @code a = Foo.new(...) @endcode.
#define ANKI_LUA_CONSTRUCTOR(...) \
	detail::pushCFunctionStatic(l_, \
		detail::ClassProxy<Class>::getName(), "new", \
		&detail::ConstructorSignature<Class, __VA_ARGS__>::luafunc);

/// Define an empty constructor. Call it from lua @code a = Foo.new() @endcode.
#define ANKI_LUA_EMPTY_CONSTRUCTOR() \
	detail::pushCFunctionStatic(l_, \
		detail::ClassProxy<Class>::getName(), "new", \
		&detail::ConstructorSignature<Class>::luafunc);

/// Define a static method with flags
#define ANKI_LUA_STATIC_METHOD_DETAIL( \
	name_, SMethodType_, smethodPtr_, flags_) \
	detail::pushCFunctionStatic(l_, \
		detail::ClassProxy<Class>::getName(), name_, \
		(&detail::FunctionSignature<flags_, \
		SMethodType_, smethodPtr_>::luafunc));

/// Define a static method no flags
#define ANKI_LUA_STATIC_METHOD(name_, smethodPtr_) \
	ANKI_LUA_STATIC_METHOD_DETAIL(name_, decltype(smethodPtr_), smethodPtr_, \
		BinderFlag::NONE)

/// Define a function as method with flags
#define ANKI_LUA_FUNCTION_AS_METHOD_DETAIL( \
	name_, FuncType_, funcPtr_, flags_) \
	detail::pushCFunctionMethod(l_, name_, \
		(&detail::FunctionSignature<flags_, FuncType_, \
			funcPtr_>::luafunc));

/// Define a function as method no flags
#define ANKI_LUA_FUNCTION_AS_METHOD(name_, funcPtr_) \
	ANKI_LUA_FUNCTION_AS_METHOD_DETAIL( \
		name_, decltype(funcPtr_), funcPtr_, BinderFlag::NONE)

/// Define a method with flags
#define ANKI_LUA_METHOD_DETAIL(name_, MethodType_, methodPtr_, flags_) \
	ANKI_LUA_FUNCTION_AS_METHOD_DETAIL(name_, \
		decltype(&detail::MethodFunctionalizer< \
			MethodType_, methodPtr_>::func), \
		(&detail::MethodFunctionalizer<MethodType_, methodPtr_>::func), \
		flags_)

/// Define a method no flags
#define ANKI_LUA_METHOD(name_, methodPtr_) \
	ANKI_LUA_METHOD_DETAIL(name_, decltype(methodPtr_), \
		methodPtr_, BinderFlag::NONE)

//==============================================================================
template<typename T>
void LuaBinder::exposeVariable(const char* name, T* y)
{
	using namespace detail;

	UserData* d = (UserData*)lua_newuserdata(m_l, sizeof(UserData));
	d->m_ptr = y;
	d->m_gc = false;
	luaL_setmetatable(m_l, ClassProxy<T>::getName());
	lua_setglobal(m_l, name);
}

} // end namespace anki

#endif
