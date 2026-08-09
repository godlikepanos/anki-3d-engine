#!/usr/bin/python

# Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import os
import optparse
import hashlib
import xml.etree.ElementTree as et
import re

# Globals
g_identation_level = 0
g_out_file = None
g_enum_names = []


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]", description="Create LUA bindings using XML")

    parser.add_option("-i",
                      "--input",
                      dest="inp",
                      type="string",
                      help="specify the XML files to parse. Seperate with :")

    (options, args) = parser.parse_args()

    if not options.inp:
        parser.error("argument is missing")

    return options.inp.split(":")


def type_sig(value):
    """ Calculate a stable signature of a type. Must be deterministic across runs and Python versions because the value
    ends up in generated source (and potentially in serialized data), so don't use the built-in hash() which is salted
    per process. """
    if not isinstance(value, str):
        raise Exception("Expecting string")
    # Take the first 8 bytes of a SHA-256 digest and interpret them as a signed 64-bit int to match the I64 m_sig field
    digest = hashlib.sha256(value.encode("utf-8")).digest()
    return int.from_bytes(digest[:8], byteorder="little", signed=True)


def get_base_fname(path):
    """ From path/to/a/file.ext return the "file" """
    return os.path.splitext(os.path.basename(path))[0]


def wglue(txt):
    """ Write glue code to the output """
    global g_out_file
    global g_identation_level
    g_out_file.write("%s%s\n" % ("\t" * g_identation_level, txt))


def ident(number):
    """ Increase or recrease identation for the wglue """
    global g_identation_level
    g_identation_level += number


def type_is_bool(type):
    """ Check if a type is boolean """
    return type == "Bool" or type == "bool"


def type_is_enum(type):
    """ Check if a type string is an enum """
    global g_enum_names
    return type in g_enum_names

def type_is_number(type):
    """ Check if a type is number """

    numbers = [
        "U8", "U16", "U32", "U64", "I8", "I16", "I32", "I64", "U", "I", "PtrSize", "F32", "F64", "int", "unsigned",
        "unsigned int", "short", "unsigned short", "uint", "float", "double"
    ]

    it_is = False
    for num in numbers:
        if num == type:
            it_is = True
            break

    return it_is


def parse_type_decl(arg_txt):
    """ Parse an arg text. A full expression that can be parsed is: WeakArray<const type&> """

    regex = re.compile(
        r"^(?P<array>(?:Const)?WeakArray<)?"  # optional WeakArray< wrapper
        r"(?:(?P<const>const)\s+)?"           # optional const
        r"(?P<type>.+?)"                      # the type (non-greedy)
        r"\s*(?P<qual>[&*]?)"                 # optional & or *
        r"(?(array)>)$"                       # closing > required iff WeakArray< matched
    )

    m = regex.match(arg_txt.strip())
    if not m:
        raise RuntimeError("Cannot parse type declaration: %s" % arg_txt)

    is_weak_array = m.group("array") is not None
    is_const = m.group("const") is not None
    type = m.group("type")
    qual = m.group("qual")

    return (type, qual == "&", qual == "*", is_const, is_weak_array)


def ret(ret_el):
    """ Push return value """

    if ret_el is None:
        wglue("return 0;")
        return

    wglue("// Push return value")

    type_txt = ret_el.text
    (type, is_ref, is_ptr, is_const, is_weak_array) = parse_type_decl(type_txt)

    if is_ptr:
        if ret_el.get("canBeNullptr") is not None and ret_el.get("canBeNullptr") == "1":
            can_be_nullptr = True
        else:
            can_be_nullptr = False

        wglue("if(ret == nullptr) [[unlikely]]")
        wglue("{")
        ident(1)
        if can_be_nullptr:
            wglue("lua_pushnil(l);")
            wglue("return 1;")
        else:
            wglue("return luaL_error(l, \"Returned nullptr. Location %s:%d %s\", ANKI_FILE, __LINE__, ANKI_FUNC);")
        ident(-1)
        wglue("}")
        wglue("")

    if type_is_bool(type):
        wglue("lua_pushboolean(l, ret);")
    elif type_is_number(type):
        wglue("lua_pushnumber(l, lua_Number(ret));")
    elif type == "char" or type == "CString":
        wglue("lua_pushstring(l, &ret[0]);")
    elif type == "Error":
        wglue("if(ret) [[unlikely]]")
        wglue("{")
        ident(1)
        wglue("return luaL_error(l, \"Returned an error. Location %s:%d %s\", ANKI_FILE, __LINE__, ANKI_FUNC);")
        ident(-1)
        wglue("}")
        wglue("")
        wglue("lua_pushnumber(l, lua_Number(!!ret));")
    else:
        if is_ptr or is_ref:
            wglue("voidp = lua_newuserdata(l, sizeof(LuaUserData));")
            wglue("ud = static_cast<LuaUserData*>(voidp);")
            wglue("luaL_setmetatable(l, \"%s\");" % type)

            wglue("extern LuaUserDataTypeInfo g_luaUserDataTypeInfo%s;" % type)
            if is_ptr:
                wglue("ud->initPointed(&g_luaUserDataTypeInfo%s, ret);" % type)
            elif is_ref:
                wglue("ud->initPointed(&g_luaUserDataTypeInfo%s, &ret);" % type)
        else:
            wglue("size = LuaUserData::computeSizeForGarbageCollected<%s>();" % type)
            wglue("voidp = lua_newuserdata(l, size);")
            wglue("luaL_setmetatable(l, \"%s\");" % type)

            wglue("ud = static_cast<LuaUserData*>(voidp);")
            wglue("extern LuaUserDataTypeInfo g_luaUserDataTypeInfo%s;" % type)
            wglue("ud->initGarbageCollected(&g_luaUserDataTypeInfo%s);" % type)

            wglue("::new(ud->getData<%s>()) %s(std::move(ret));" % (type, type))

    wglue("")
    wglue("return 1;")


def arg(arg_txt, stack_index, index):
    """ Write the pop code for a single argument """

    (type, is_ref, is_ptr, is_const, is_weak_array) = parse_type_decl(arg_txt)

    if is_weak_array:
        if is_ref:
            raise RuntimeError("References not supported: %s" % arg_txt)

        if is_const:
            raise RuntimeError("const not supported: %s" % arg_txt)

        wglue("U32 arg%dTableSize;" % index)
        wglue("if(LuaBinder::checkTable(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d, arg%dTableSize)) [[unlikely]]" % (stack_index, index))
        wglue("{")
        ident(1)
        wglue("return lua_error(l);")
        ident(-1)
        wglue("}")

        wglue("ScriptDynamicArray<%s%s> arg%dArr;" % (type, "*" if is_ptr else "", index))
        wglue("arg%dArr.resize(arg%dTableSize);" % (index, index))
        wglue("WeakArray<%s%s> arg%d(arg%dArr);" % (type, "*" if is_ptr else "", index, index))

        wglue("for(U32 i = 0; i < arg%dTableSize; ++i)" % index)
        wglue("{")
        ident(1)
        wglue("lua_rawgeti(l, %d, int(i) + 1); // Push the element at the top of the stack" % stack_index)

        if type_is_bool(type):
            wglue("if(LuaBinder::checkBool(l, ANKI_FILE, __LINE__, ANKI_FUNC, -1, arg%d[i])) [[unlikely]]" % index)
            wglue("{")
            ident(1)
            wglue("return lua_error(l);")
            ident(-1)
            wglue("}")
        elif type_is_number(type):
            wglue("if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, -1, arg%d[i])) [[unlikely]]" % index)
            wglue("{")
            ident(1)
            wglue("return lua_error(l);")
            ident(-1)
            wglue("}")
        elif type == "char" or type == "CString":
            raise RuntimeError("Strings not supported at the moment: %s" % arg_txt)
        elif type_is_enum(type):
            wglue("lua_Number tmp;")
            wglue("if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, -1, tmp)) [[unlikely]]")
            wglue("{")
            ident(1)
            wglue("return lua_error(l);")
            ident(-1)
            wglue("}")
            wglue("arg%d[i] = %s(tmp);" % (index, type))
        else:
            # User data
            wglue("extern LuaUserDataTypeInfo g_luaUserDataTypeInfo%s;" % type)
            wglue("if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, -1, g_luaUserDataTypeInfo%s, ud)) [[unlikely]]" % type)
            wglue("{")
            ident(1)
            wglue("return lua_error(l);")
            ident(-1)
            wglue("}")
            wglue("")

            wglue("%s* iarg = ud->getData<%s>();" % (type, type))

            if is_ptr:
                wglue("arg%d[i] = iarg;" % index)
            else:
                wglue("arg%d[i] = *iarg;" % index)

        wglue("lua_pop(l, 1); // Pop because of the rawgeti")
        ident(-1)
        wglue("}")
    elif type_is_bool(type):
        wglue("%s arg%d;" % (type, index))
        wglue("if(LuaBinder::checkBool(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d, arg%d)) [[unlikely]]" % (stack_index, index))
        wglue("{")
        ident(1)
        wglue("return lua_error(l);")
        ident(-1)
        wglue("}")
    elif type_is_number(type):
        wglue("%s arg%d;" % (type, index))
        wglue("if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d, arg%d)) [[unlikely]]" % (stack_index, index))
        wglue("{")
        ident(1)
        wglue("return lua_error(l);")
        ident(-1)
        wglue("}")
    elif type == "char" or type == "CString":
        wglue("const char* arg%d;" % index)
        wglue("if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d, arg%d)) [[unlikely]]" % (stack_index, index))
        wglue("{")
        ident(1)
        wglue("return lua_error(l);")
        ident(-1)
        wglue("}")
    elif type_is_enum(type):
        wglue("lua_Number arg%dTmp;" % index)
        wglue("if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d, arg%dTmp)) [[unlikely]]" % (stack_index, index))
        wglue("{")
        ident(1)
        wglue("return lua_error(l);")
        ident(-1)
        wglue("}")
        wglue("const %s arg%d = %s(arg%dTmp);" % (type, index, type, index))
    else:
        # Must be user type
        wglue("extern LuaUserDataTypeInfo g_luaUserDataTypeInfo%s;" % type)
        wglue("if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d, g_luaUserDataTypeInfo%s, ud)) [[unlikely]]" % (stack_index, type))
        wglue("{")
        ident(1)
        wglue("return lua_error(l);")
        ident(-1)
        wglue("}")
        wglue("")

        wglue("%s* iarg%d = ud->getData<%s>();" % (type, index, type))

        if is_ptr:
            wglue("%s arg%d(iarg%d);" % (arg_txt, index, index))
        else:
            wglue("%s arg%d(*iarg%d);" % (arg_txt, index, index))


def args(args_el, stack_index):
    """ Write the pop code for argument parsing and return the arg list """

    if args_el is None:
        return ""

    wglue("// Pop arguments")
    arg_index = 0

    # Do the work
    args_str = ""
    arg_index = 0
    for arg_el in args_el.iter("arg"):
        arg(arg_el.text, stack_index, arg_index)
        args_str += "arg%d, " % arg_index
        wglue("")
        stack_index += 1
        arg_index += 1

    if len(args_str) > 0:
        args_str = args_str[:-2]

    return args_str


def count_args(args_el):
    """ Count the number of arguments """

    if args_el is None:
        return 0

    count = 0
    for arg_el in args_el.iter("arg"):
        count += 1

    return count


def args_signature(args_el, var_name : str, class_name):
    """ Generates an imperfect signature of the function arguments. It's imperfect because it needs to be fast to compute at runtime. Signature used
        to choose the proper function overload """

    signature = ""
    is_first = True
    count = 0

    # Add the "this" which is the implicit 1st argument in methods
    if class_name is not None:
        signature += "LUA_TUSERDATA, " + str(type_sig(class_name))
        is_first = False
        count += 2

    if args_el is not None:
        for arg_el in args_el.iter("arg"):
            (type, is_ref, is_ptr, is_const, is_weak_array) = parse_type_decl(arg_el.text)

            if not is_first:
                signature += ", "
            else:
                is_first = False

            count += 1

            if is_weak_array:
                signature += "LUA_TTABLE"
            elif type_is_bool(type):
                signature += "LUA_TBOOLEAN"
            elif type_is_number(type):
                signature += "LUA_TNUMBER"
            elif type == "char" or type == "CString":
                signature += "LUA_TSTRING"
            elif type_is_enum(type):
                signature += "LUA_TNUMBER"
            else:
                signature += "LUA_TUSERDATA, " + str(type_sig(type))
                count += 1

    if count == 0:
        wglue("constexpr U64 %s = 0;" % var_name)
    else:
        wglue("constexpr I64 %sArr[] = {%s};" % (var_name, signature))
        wglue("constexpr U64 %s = computeArrayHashConstexpr(%sArr, sizeof(%sArr) / sizeof(I64));" % (var_name, var_name, var_name))


def check_args(args_el, bias):
    """ Check number of args. Call that first because it throws error """

    if args_el is not None:
        count = bias + count_args(args_el)
    else:
        count = bias

    wglue("if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, %d)) [[unlikely]]" % count)
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")


def get_meth_alias(meth_name : str):
    """ Return the method alias. Some C++ method names don't map to LUA directly """

    assert(isinstance(meth_name, str))

    if meth_name == "operator+":
        meth_alias = "__add"
    elif meth_name == "operator-":
        meth_alias = "__sub"
    elif meth_name == "operator*":
        meth_alias = "__mul"
    elif meth_name == "operator/":
        meth_alias = "__div"
    elif meth_name == "operator==":
        meth_alias = "__eq"
    elif meth_name == "operator<":
        meth_alias = "__lt"
    elif meth_name == "operator<=":
        meth_alias = "__le"
    elif meth_name == "operator>":
        meth_alias = "__gt"
    elif meth_name == "operator>=":
        meth_alias = "__ge"
    elif meth_name == "operator=":
        meth_alias = "copy"
    else:
        meth_alias = meth_name

    return meth_alias


def write_local_vars():
    wglue("[[maybe_unused]] LuaUserData* ud;")
    wglue("[[maybe_unused]] void* voidp;")
    wglue("[[maybe_unused]] PtrSize size;")
    wglue("")


def method(class_name, meth_el, overload_idx):
    """ Handle a method """

    is_overloaded = overload_idx is not None
    args_el = meth_el.find("args")
    meth_name = meth_el.get("name")
    meth_alias = get_meth_alias(meth_name)

    wglue("// Wrap method %s::%s" % (class_name, meth_name))
    if is_overloaded:
        args_signature(args_el, "k%s%s%dArgsSignature" % (class_name, meth_alias, overload_idx), class_name)
    wglue("static inline int wrap%s%s%s(lua_State* l)" % (class_name, meth_alias, str(overload_idx) if is_overloaded else ""))
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(args_el, 1)

    # Get this pointer
    wglue("// Get \"this\" as \"self\"")
    wglue("if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfo%s, ud)) [[unlikely]]" % class_name)
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")
    wglue("%s* self = ud->getData<%s>();" % (class_name, class_name))
    wglue("")

    args_str = args(args_el, 2)

    # Return value
    ret_txt = None
    ret_el = meth_el.find("return")
    if ret_el is not None:
        ret_txt = ret_el.text

    # Method call
    wglue("// Call the method")
    override_call = meth_el.find("overrideCall")
    if override_call is not None:
        override_call = override_call.text

    if override_call is not None:
        wglue("%s" % override_call)
    else:
        wglue("%sself->%s(%s);" % ("" if ret_txt is None else ret_txt + " ret = ", meth_name, args_str))

    wglue("")
    ret(ret_el)

    ident(-1)
    wglue("}")
    wglue("")


def static_method(class_name : str, meth_el, overload_idx):
    """ Handle a static method """

    is_overloaded = overload_idx is not None

    meth_name = meth_el.get("name")
    meth_alias = get_meth_alias(meth_name)

    wglue("// Wrap static method %s::%s" % (class_name, meth_name))
    if is_overloaded:
        args_signature(meth_el.find("args"), "k%s%s%dArgsSignature" % (class_name, meth_alias, overload_idx), None)
    wglue("static inline int wrap%s%s%s(lua_State* l)" % (class_name, meth_alias, str(overload_idx) if is_overloaded else ""))
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(meth_el.find("args"), 0)

    # Args
    args_str = args(meth_el.find("args"), 1)

    # Return value
    ret_txt = None
    ret_el = meth_el.find("return")
    if ret_el is not None:
        ret_txt = ret_el.text

    # Method call
    wglue("// Call the method")
    override_call = meth_el.find("overrideCall")
    if override_call is not None:
        override_call = override_call.text

    if override_call is not None:
        wglue("%s" % override_call)
    else:
        wglue("%s%s::%s(%s);" % ("" if ret_txt is None else ret_txt + " ret = ", class_name, meth_name, args_str))

    wglue("")
    ret(ret_el)

    ident(-1)
    wglue("}")
    wglue("")


def constructor(constr_el, class_name, constructor_idx):
    """ Handle constructor """

    args_el = constr_el.find("args")

    wglue("// Wrap constructor for %s" % (class_name))
    args_signature(args_el, "k%sCtor%dArgsSignature" % (class_name, constructor_idx), None) # Pass None, it's essentially a static function
    wglue("static inline int wrap%sCtor%d(lua_State* l)" % (class_name, constructor_idx))
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(args_el, 0)

    # Args
    args_str = args(args_el, 1)

    # Create new userdata
    wglue("// Create user data")

    wglue("size = LuaUserData::computeSizeForGarbageCollected<%s>();" % class_name)
    wglue("voidp = lua_newuserdata(l, size);")
    wglue("luaL_setmetatable(l, g_luaUserDataTypeInfo%s.m_typeName);" % class_name)
    wglue("ud = static_cast<LuaUserData*>(voidp);")
    wglue("extern LuaUserDataTypeInfo g_luaUserDataTypeInfo%s;" % class_name)
    wglue("ud->initGarbageCollected(&g_luaUserDataTypeInfo%s);" % class_name)
    wglue("::new(ud->getData<%s>()) %s(%s);" % (class_name, class_name, args_str))
    wglue("")

    wglue("return 1;")

    ident(-1)
    wglue("}")
    wglue("")


def constructors(constructors_el, class_name):
    """ Wrap all constructors """

    ctor_count = 0

    # Create the pre-wrap C functions
    for constructor_el in constructors_el.iter("constructor"):
        constructor(constructor_el, class_name, ctor_count)
        ctor_count += 1

    if ctor_count == 0:
        raise Exception("Found no <constructor>")

    # Create the landing function. If there are signature collisions the C++ compiler will fail
    wglue("// Wrap constructors for %s." % class_name)
    wglue("static int wrap%sCtor(lua_State* l)" % class_name)
    wglue("{")
    ident(1)
    if ctor_count == 1:
        wglue("int ret = wrap%sCtor0(l);" % class_name)
    else:
        wglue("// Chose the right overload")
        wglue("const U64 argsSignature = LuaBinder::computeFunctionArgumentSignature(l);")
        wglue("int ret;")
        wglue("switch(argsSignature)")
        wglue("{")
        for i in range(0, ctor_count):
            wglue("case k%sCtor%dArgsSignature:" % (class_name, i))
            ident(1)
            wglue("ret = wrap%sCtor%d(l);" % (class_name, i))
            wglue("break;")
            ident(-1)

        wglue("default:")
        ident(1)
        wglue("lua_pushfstring(l, \"Wrong arguments for constructor of class: %s\");" % class_name)
        wglue("ret = lua_error(l);")
        ident(-1)
        wglue("}")
    wglue("")

    wglue("return ret;")
    ident(-1)
    wglue("}")
    wglue("")


def destructor(class_name):
    """ Create a destructor """

    wglue("// Wrap destructor for %s." % (class_name))
    wglue("static int wrap%sDtor(lua_State* l)" % class_name)
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(None, 1)

    wglue("if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfo%s, ud)) [[unlikely]]" % class_name)
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")

    wglue("if(ud->isGarbageCollected())")
    wglue("{")
    ident(1)
    wglue("%s* inst = ud->getData<%s>();" % (class_name, class_name))
    wglue("inst->~%s();" % class_name)
    ident(-1)
    wglue("}")
    wglue("")

    wglue("return 0;")

    ident(-1)
    wglue("}")
    wglue("")


def do__newindex(vars_el, class_name):
    """ Write the __newindex cfunction which is called when assigning to a class instance """

    wglue("// Wrap writing the member vars of %s" % class_name)
    wglue("static int wrap%s__newindex(lua_State* l)" % class_name)
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(None, 3)

    wglue("// Get \"this\" as \"self\"")
    wglue("if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfo%s, ud)) [[unlikely]]" % class_name)
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")
    wglue("%s* self = ud->getData<%s>();" % (class_name, class_name))
    wglue("")

    wglue("// Get the member variable name")
    wglue("const Char* ckey;")
    wglue("if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]")
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")
    wglue("CString key = ckey;")
    wglue("")

    wglue("// Try to find the member variable")
    count = 0
    for var_el in vars_el.iter("var"):
        wglue("%sif(key == \"%s\")" % (("else " if count > 0 else ""), var_el.get("name")))
        count = count + 1
        wglue("{")
        ident(1)

        arg(var_el.text, 3, 0)
        wglue("self->%s = arg0;" % var_el.get("name"))
        wglue("return 0;")

        ident(-1)
        wglue("}")
    wglue("")

    wglue("return luaL_error(l, \"Unknown field %s. Location %s:%d %s\", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);")

    ident(-1)
    wglue("}")
    wglue("")


def do__index(vars_el, class_name):
    """ Write the __index cfunction which is called when assigning to a class instance """

    wglue("// Wrap reading the member vars of %s" % class_name)
    wglue("static int wrap%s__index(lua_State* l)" % class_name)
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(None, 2)

    wglue("// Get \"this\" as \"self\"")
    wglue("if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfo%s, ud)) [[unlikely]]" % class_name)
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")
    wglue("%s* self = ud->getData<%s>();" % (class_name, class_name))
    wglue("")

    wglue("// Get the member variable name")
    wglue("const Char* ckey;")
    wglue("if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]")
    wglue("{")
    ident(1)
    wglue("return lua_error(l);")
    ident(-1)
    wglue("}")
    wglue("")
    wglue("CString key = ckey;")
    wglue("")

    wglue("// Try to find the member variable")
    count = 0
    for var_el in vars_el.iter("var"):
        wglue("%sif(key == \"%s\")" % (("else " if count > 0 else ""), var_el.get("name")))
        count = count + 1
        wglue("{")
        ident(1)

        wglue("%s ret = self->%s;" % (var_el.text, var_el.get("name")))
        ret(var_el)

        ident(-1)
        wglue("}")
    wglue("")

    wglue("// Fallback to methods")
    wglue("luaL_getmetatable(l, \"%s\");" % class_name)
    wglue("lua_getfield(l, -1, ckey);")
    wglue("if (!lua_isnil(l, -1))")
    wglue("{")
    ident(1)
    wglue("return 1;")
    ident(-1)
    wglue("}")
    wglue("")

    wglue("return luaL_error(l, \"Unknown field %s. Location %s:%d %s\", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);")

    ident(-1)
    wglue("}")
    wglue("")


def class_(class_el):
    """ Create a class """

    class_name = class_el.get("name")

    # Write serializer
    serialize = class_el.get("serialize") is not None and class_el.get("serialize") == "true"
    if serialize:
        # Serialize
        serialize_cb_name = "serialize%s" % class_name
        wglue("// Serialize %s" % class_name)
        wglue("static void %s(const LuaUserData& self, void* data, PtrSize& size)" % serialize_cb_name)
        wglue("{")
        ident(1)
        wglue("const %s* obj = self.getData<%s>();" % (class_name, class_name))
        wglue("obj->serialize(data, size);")
        ident(-1)
        wglue("}")
        wglue("")

        # Deserialize
        deserialize_cb_name = "deserialize%s" % class_name
        wglue("// De-serialize %s" % class_name)
        wglue("static void %s(const void* data, LuaUserData& self)" % deserialize_cb_name)
        wglue("{")
        ident(1)
        wglue("ANKI_ASSERT(data);")
        wglue("%s* obj = self.getData<%s>();" % (class_name, class_name))
        wglue("::new(obj) %s();" % class_name)
        wglue("obj->deserialize(data);")
        ident(-1)
        wglue("}")
        wglue("")
    else:
        serialize_cb_name = "nullptr"
        deserialize_cb_name = "nullptr"

    # Write the type info
    wglue("LuaUserDataTypeInfo g_luaUserDataTypeInfo%s = {" % class_name)
    ident(1)
    wglue("%d, \"%s\", LuaUserData::computeSizeForGarbageCollected<%s>(), %s, %s" %
          (type_sig(class_name), class_name, class_name, serialize_cb_name, deserialize_cb_name))
    ident(-1)
    wglue("};")
    wglue("")

    # Specialize the getDataTypeInfoFor
    wglue("template<>")
    wglue("const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<%s>()" % class_name)
    wglue("{")
    ident(1)
    wglue("return g_luaUserDataTypeInfo%s;" % class_name)
    ident(-1)
    wglue("}")
    wglue("")

    # Constructor declarations
    has_constructor = False
    constructors_el = class_el.find("constructors")
    if constructors_el is not None:
        has_constructor = True
        constructors(constructors_el, class_name)

    # Destructor declarations
    if has_constructor:
        destructor(class_name)

    # Member variables
    has_member_vars = False
    vars_el = class_el.find("vars")
    if vars_el is not None:
        has_member_vars = True
        do__newindex(vars_el, class_name)
        do__index(vars_el, class_name)

    # Find which methods are overloaded and how many times
    meths_el = class_el.find("methods")
    method_is_overloaded = {}
    method_to_overload_count = {}
    if meths_el is not None:
        for meth_el in meths_el.iter("method"):
            meth_name = meth_el.get("name")

            if meth_name in method_is_overloaded:
                method_is_overloaded[meth_name] = True
                method_to_overload_count[meth_name] = 0
            else:
                method_is_overloaded[meth_name] = False

    # Methods LUA C functions declarations
    meth_names_aliases = []
    if meths_el is not None:
        for meth_el in meths_el.iter("method"):
            meth_name = meth_el.get("name")
            meth_alias = get_meth_alias(meth_name)

            is_static = meth_el.get("static")
            is_static = is_static is not None and is_static == "1"
            is_overloaded = method_is_overloaded[meth_name]

            if is_overloaded:
                overload_idx = method_to_overload_count[meth_name]
                method_to_overload_count[meth_name] += 1
            else:
                overload_idx = None

            if is_static:
                static_method(class_name, meth_el, overload_idx)
            else:
                method(class_name, meth_el, overload_idx)

            if overload_idx is None or overload_idx == 0:
                meth_names_aliases.append([meth_name, meth_alias, is_static])

    # Generate the overload selector functions. If there are collisions in the hash C++ will fail to compile
    for meth_name, overload_count in method_to_overload_count.items():
        meth_alias = get_meth_alias(meth_name)
        wglue("// Wrap overload selector for %s::%s" % (class_name, meth_name))
        wglue("static inline int wrap%s%s(lua_State* l)" % (class_name, meth_alias))
        wglue("{")
        ident(1)
        wglue("const U64 argsSignature = LuaBinder::computeFunctionArgumentSignature(l);")
        wglue("int ret;")

        wglue("switch(argsSignature)")
        wglue("{")
        for i in range(0, overload_count):
            wglue("case k%s%s%dArgsSignature:" % (class_name, meth_alias, i))
            ident(1)
            wglue("ret = wrap%s%s%d(l);" % (class_name, meth_alias, i))
            wglue("break;")
            ident(-1)
        wglue("default:")
        ident(1)
        wglue("lua_pushfstring(l, \"Wrong arguments for method: %s::%s\");" % (class_name, meth_alias))
        wglue("ret = lua_error(l);")
        ident(-1)
        wglue("}")

        wglue("return ret;")
        ident(-1)
        wglue("}")
        wglue("")

    # Start class declaration
    wglue("// Wrap class %s." % class_name)
    wglue("static inline void wrap%s(lua_State* l)" % class_name)
    wglue("{")
    ident(1)
    wglue("LuaBinder::createClass(l, &g_luaUserDataTypeInfo%s);" % class_name)

    # Register constructor
    if has_constructor:
        wglue("LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfo%s.m_typeName, \"new\", wrap%sCtor);" %
              (class_name, class_name))

    # Register destructor
    if has_constructor:
        wglue("LuaBinder::pushLuaCFuncMethod(l, \"__gc\", wrap%sDtor);" % class_name)

    # Register methods
    if len(meth_names_aliases) > 0:
        for meth_name_alias in meth_names_aliases:
            meth_alias = meth_name_alias[1]
            is_static = meth_name_alias[2]
            if is_static:
                wglue("LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfo%s.m_typeName, \"%s\", wrap%s%s);" %
                      (class_name, meth_alias, class_name, meth_alias))
            else:
                wglue("LuaBinder::pushLuaCFuncMethod(l, \"%s\", wrap%s%s);" % (meth_alias, class_name, meth_alias))

    # Register member vars
    if has_member_vars:
        wglue("LuaBinder::pushLuaCFuncMethod(l, \"__newindex\", wrap%s__newindex);" % class_name)
        wglue("LuaBinder::pushLuaCFuncMethod(l, \"__index\", wrap%s__index);" % class_name)

    wglue("lua_settop(l, 0);")

    ident(-1)
    wglue("}")
    wglue("")


def enum(enum_el):
    enum_name = enum_el.get("name")

    # Write the type info
    wglue("LuaUserDataTypeInfo g_luaUserDataTypeInfo%s = {" % enum_name)
    ident(1)
    wglue("%d, \"%s\", 0, nullptr, nullptr" % (type_sig(enum_name), enum_name))
    ident(-1)
    wglue("};")
    wglue("")

    # Specialize the getDataTypeInfoFor
    wglue("template<>")
    wglue("const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<%s>()" % enum_name)
    wglue("{")
    ident(1)
    wglue("return g_luaUserDataTypeInfo%s;" % enum_name)
    ident(-1)
    wglue("}")
    wglue("")

    # Start declaration
    wglue("// Wrap enum %s." % enum_name)
    wglue("static inline void wrap%s(lua_State* l)" % enum_name)
    wglue("{")
    ident(1)

    wglue("lua_newtable(l);")  # Push new table
    wglue("lua_setglobal(l, g_luaUserDataTypeInfo%s.m_typeName);" % enum_name)  # Pop and make global
    wglue("lua_getglobal(l, g_luaUserDataTypeInfo%s.m_typeName);" % enum_name)  # Push the table again
    wglue("")

    # Now the table is at the top of the stack

    for enumerant_el in enum_el.iter("enumerant"):
        enumerant_name = enumerant_el.get("name")

        wglue("lua_pushstring(l, \"%s\");" % enumerant_name)  # Push key
        wglue("ANKI_ASSERT(%s(lua_Number(%s::%s)) == %s::%s && \"Can't map the enumerant to a lua_Number\");" %
              (enum_name, enum_name, enumerant_name, enum_name, enumerant_name))
        wglue("lua_pushnumber(l, lua_Number(%s::%s));" % (enum_name, enumerant_name))  # Push value
        wglue("lua_settable(l, -3);")  # Do table[key]=value and pop 2. The table is at the top of the stack
        wglue("")

    # Done
    wglue("lua_settop(l, 0);")

    ident(-1)
    wglue("}")
    wglue("")


def function(func_el, overload_idx):
    """ Handle a plain function """

    func_name = func_el.get("name")
    func_alias = get_meth_alias(func_name)
    is_overloaded = overload_idx is not None
    args_el = func_el.find("args")

    wglue("// Wrap function %s" % func_name)
    if is_overloaded:
        args_signature(args_el, "k%s%dArgsSignature" % (func_alias, overload_idx), None)
    wglue("static inline int wrap%s%s(lua_State* l)" % (func_alias, str(overload_idx) if is_overloaded else ""))
    wglue("{")
    ident(1)
    write_local_vars()

    check_args(args_el, 0)

    # Args
    args_str = args(args_el, 1)

    # Return value
    ret_txt = None
    ret_el = func_el.find("return")
    if ret_el is not None:
        ret_txt = ret_el.text

    # Call
    wglue("// Call the function")
    override_call = func_el.find("overrideCall")
    if override_call is not None:
        override_call = override_call.text

    if override_call is not None:
        wglue("%s" % override_call)
    else:
        wglue("%s%s(%s);" % ("" if ret_txt is None else ret_txt + " ret = ", func_name, args_str))

    wglue("")
    ret(ret_el)

    ident(-1)
    wglue("}")
    wglue("")


def functions(funcs_el) :
    func_is_overloaded = {}
    func_to_overload_count = {}

    # Find which funcs are overloaded and how many times
    func_names = []
    for func_el in funcs_el.iter("function"):
        func_name = func_el.get("name")
        func_alias = get_meth_alias(func_name)

        if func_name in func_is_overloaded:
            func_is_overloaded[func_name] = True
            func_to_overload_count[func_name] = 0
        else:
            func_is_overloaded[func_name] = False
            func_names.append(func_alias)

    # Do func declarations
    for func_el in funcs_el.iter("function"):
        func_name = func_el.get("name")

        is_overloaded = func_is_overloaded[func_name]

        if is_overloaded:
            overload_idx = func_to_overload_count[func_name]
            func_to_overload_count[func_name] += 1
        else:
            overload_idx = None

        function(func_el, overload_idx)

    # Generate the overload selector functions. If there are collisions in the hash C++ will fail to compile
    for func_name, overload_count in func_to_overload_count.items():
        func_alias = get_meth_alias(func_name)

        wglue("// Wrap overload selector for %s" % func_name)
        wglue("static inline int wrap%s(lua_State* l)" % func_alias)
        wglue("{")
        ident(1)
        wglue("const U64 argsSignature = LuaBinder::computeFunctionArgumentSignature(l);")
        wglue("int ret;")

        wglue("switch(argsSignature)")
        wglue("{")
        for i in range(0, overload_count):
            wglue("case k%s%dArgsSignature:" % (func_alias, i))
            ident(1)
            wglue("ret = wrap%s%d(l);" % (func_alias, i))
            wglue("break;")
            ident(-1)
        wglue("default:")
        ident(1)
        wglue("lua_pushfstring(l, \"Wrong arguments for function: %s\");" % func_alias)
        wglue("ret = lua_error(l);")
        ident(-1)
        wglue("}")

        wglue("return ret;")
        ident(-1)
        wglue("}")
        wglue("")

    return func_names

def main():
    """ Main function """

    global g_out_file
    filenames = parse_commandline()

    for filename in filenames:
        out_filename = get_base_fname(filename) + ".cpp"
        g_out_file = open(out_filename, "w", newline="\n")

        tree = et.parse(filename)
        root = tree.getroot()

        # Head
        head = root.find("head")
        if head is not None:
            wglue("%s" % head.text)
            wglue("")

        # Enums (First because others use the g_enum_names)
        global g_enum_names
        for enums in root.iter("enums"):
            for enum_el in enums.iter("enum"):
                enum(enum_el)
                g_enum_names.append(enum_el.get("name"))

        # Classes
        class_names = []
        for cls in root.iter("classes"):
            for cl in cls.iter("class"):
                class_(cl)
                class_names.append(cl.get("name"))

        # Functions
        func_names = []
        for fs in root.iter("functions"):
            func_names += functions(fs)

        # Final wrap function
        wglue("// Wrap the module.")
        wglue("void wrapModule%s(lua_State* l)" % get_base_fname(filename))
        wglue("{")
        ident(1)
        for class_name in class_names:
            wglue("wrap%s(l);" % class_name)
        for func_name in func_names:
            wglue("LuaBinder::pushLuaCFunc(l, \"%s\", wrap%s);" % (func_name, func_name))
        for enum_name in g_enum_names:
            wglue("wrap%s(l);" % enum_name)
        ident(-1)
        wglue("}")
        wglue("")

        # Tail
        tail = root.find("tail")
        if tail is not None:
            wglue("%s" % tail.text)
            wglue("")

        g_out_file.close()


if __name__ == "__main__":
    main()
