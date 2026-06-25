#!/usr/bin/python

# Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

# Generates an EmmyLua definition stub (aka "meta" file) from the same XML
# binding specs that LuaGlueGen.py consumes. The output is *not* executed by the
# engine; it only feeds the Lua Language Server (sumneko's "Lua" VSCode
# extension) so that AnKi scene scripts get autocomplete, signatures and type
# hints. Re-run it whenever the *.xml binding specs change.
#
# Usage:
#   LuaDefsGen.py -i Scene.xml:Math.xml:Renderer.xml:Logger.xml -o AnKiScriptApi.lua

import os
import optparse
import xml.etree.ElementTree as et

# Globals
g_out_file = None
g_enum_names = []

NUMBER_TYPES = [
    "U8", "U16", "U32", "U64", "I8", "I16", "I32", "I64", "U", "I", "PtrSize", "F32", "F64", "int", "unsigned",
    "unsigned int", "short", "unsigned short", "uint", "float", "double", "Second", "Timestamp"
]

# C++ operator name -> EmmyLua "---@operator" name. Only the arithmetic operators
# have an EmmyLua equivalent; comparison metamethods work in Lua automatically and
# need no annotation.
OPERATOR_MAP = {
    "operator+": "add",
    "operator-": "sub",
    "operator*": "mul",
    "operator/": "div",
}


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]",
                                   description="Create an EmmyLua definition stub from the binding XML")

    parser.add_option("-i",
                      "--input",
                      dest="inp",
                      type="string",
                      help="specify the XML files to parse. Seperate with :")

    parser.add_option("-o",
                      "--output",
                      dest="out",
                      type="string",
                      default="AnKiScriptApi.lua",
                      help="the output lua definition file")

    (options, args) = parser.parse_args()

    if not options.inp:
        parser.error("argument is missing")

    return (options.inp.split(":"), options.out)


def wlua(txt):
    """ Write a line to the output """
    global g_out_file
    g_out_file.write("%s\n" % txt)


def parse_type_decl(arg_txt):
    """ Parse a type text and strip cv/ref/ptr qualifiers. Same logic as LuaGlueGen.py """

    tokens = arg_txt.split(" ")
    type = tokens[len(tokens) - 1]

    is_ptr = type.endswith("*")
    is_ref = type.endswith("&")
    if is_ref or is_ptr:
        type = type[:-1]

    is_const = tokens[0] == "const"

    return (type, is_ref, is_ptr, is_const)


def type_is_bool(type):
    return type == "Bool" or type == "bool"


def type_is_number(type):
    return type in NUMBER_TYPES


def type_is_enum(type):
    return type in g_enum_names


def lua_type(anki_type):
    """ Map an AnKi C++ type to a Lua Language Server type. Returns None for types
    that carry no meaningful Lua value (void / Error). """

    if anki_type is None:
        return None

    (type, is_ref, is_ptr, is_const) = parse_type_decl(anki_type)

    if type == "void" or type == "Error":
        return None
    if type_is_bool(type):
        return "boolean"
    if type_is_number(type):
        return "number"
    if type == "char" or type == "CString":
        return "string"
    if type_is_enum(type):
        # Enums are passed/returned as plain numbers in Lua but the global table
        # (e.g. SkyboxComponentType.kGenerated) resolves to an integer.
        return "integer"

    # User class type
    return type


def to_camel(s):
    """ FooBar -> fooBar """
    return s[0].lower() + s[1:] if s else s


def param_base_name(anki_type):
    """ Pick a readable parameter name based on the type """

    (type, is_ref, is_ptr, is_const) = parse_type_decl(anki_type)

    if type_is_bool(type):
        return "b"
    if type_is_number(type):
        return "num"
    if type == "char" or type == "CString":
        return "str"
    return to_camel(type)


def build_params(args_el):
    """ Return a list of (name, luaType) tuples for an <args> element """

    params = []
    if args_el is None:
        return params

    used = {}
    for arg_el in args_el.iter("arg"):
        lt = lua_type(arg_el.text) or "any"
        base = param_base_name(arg_el.text)
        if base in used:
            used[base] += 1
            name = "%s%d" % (base, used[base])
        else:
            used[base] = 1
            name = base
        params.append((name, lt))

    return params


def params_to_sig(params):
    """ params -> "a, b, c" """
    return ", ".join(name for (name, _) in params)


def params_to_fun(params, ret_lua):
    """ params -> "fun(a: T, b: T): R" for use in ---@overload """
    inner = ", ".join("%s: %s" % (name, lt) for (name, lt) in params)
    if ret_lua:
        return "fun(%s): %s" % (inner, ret_lua)
    return "fun(%s)" % inner


def get_return_lua(el):
    """ Return the Lua type of an element's <return> child, or None """
    ret_el = el.find("return")
    if ret_el is None:
        return None
    return lua_type(ret_el.text)


def emit_callable(decl, params, ret_lua, overloads=None):
    """ Emit the annotation block + the function stub.

    decl      : the "function X:y" or "function x" line without the trailing "()"
    params    : list of (name, luaType)
    ret_lua   : the Lua return type or None
    overloads : optional list of "fun(...)" strings to add as ---@overload
    """

    for ov in (overloads or []):
        wlua("---@overload %s" % ov)
    for (name, lt) in params:
        wlua("---@param %s %s" % (name, lt))
    if ret_lua:
        wlua("---@return %s" % ret_lua)
    wlua("%s(%s) end" % (decl, params_to_sig(params)))
    wlua("")


def emit_enum(enum_el):
    """ Emit an enum as a class-like global table so EnumName.<value> completes """

    enum_name = enum_el.get("name")
    wlua("---@class %s" % enum_name)
    for enumerant_el in enum_el.iter("enumerant"):
        wlua("---@field %s integer" % enumerant_el.get("name"))
    wlua("%s = {}" % enum_name)
    wlua("")


def emit_class(class_el):
    """ Emit a class: its @class header (fields + operators), then constructors and methods """

    class_name = class_el.get("name")

    # Collect member variables -> @field
    fields = []
    vars_el = class_el.find("vars")
    if vars_el is not None:
        for var_el in vars_el.iter("var"):
            fields.append((var_el.get("name"), lua_type(var_el.text) or "any"))

    # Collect arithmetic operators -> @operator
    operators = []
    meths_el = class_el.find("methods")
    if meths_el is not None:
        for meth_el in meths_el.iter("method"):
            op = OPERATOR_MAP.get(meth_el.get("name"))
            if op is None:
                continue
            params = build_params(meth_el.find("args"))
            ret_lua = get_return_lua(meth_el) or class_name
            rhs = params[0][1] if params else ""
            operators.append("---@operator %s(%s): %s" % (op, rhs, ret_lua))

    # Class header
    wlua("---@class %s" % class_name)
    for (name, lt) in fields:
        wlua("---@field %s %s" % (name, lt))
    for op in operators:
        wlua(op)
    wlua("%s = {}" % class_name)
    wlua("")

    # Constructors -> ClassName.new(...)
    constructors_el = class_el.find("constructors")
    if constructors_el is not None:
        ctors = list(constructors_el.iter("constructor"))
        ctor_params = [build_params(c.find("args")) for c in ctors]
        # Use the constructor with the most args as the primary signature so the
        # richest hint shows up; the rest become overloads.
        ctor_params.sort(key=len, reverse=True)
        primary = ctor_params[0]
        overloads = [params_to_fun(p, class_name) for p in ctor_params[1:]]
        emit_callable("function %s.new" % class_name, primary, class_name, overloads)

    # Methods
    if meths_el is not None:
        seen = set()
        for meth_el in meths_el.iter("method"):
            meth_name = meth_el.get("name")

            # Arithmetic operators are already covered by @operator above
            if meth_name in OPERATOR_MAP:
                continue
            # Comparison metamethods are implicit in Lua; skip them
            if meth_name.startswith("operator") and meth_name != "operator=":
                continue

            # operator= is bound as a regular method called "copy"
            alias = meth_el.get("alias")
            if alias is not None:
                lua_name = alias
            elif meth_name == "operator=":
                lua_name = "copy"
            else:
                lua_name = meth_name

            is_static = meth_el.get("static") == "1"
            sep = "." if is_static else ":"

            params = build_params(meth_el.find("args"))
            ret_lua = get_return_lua(meth_el)

            # Duplicate names within a class become overloads of the first decl
            key = (lua_name, is_static)
            if key in seen:
                wlua("---@overload %s" % params_to_fun(params, ret_lua))
                wlua("%s%s%s() end" % ("function ", class_name + sep, lua_name))
                wlua("")
                continue
            seen.add(key)

            emit_callable("function %s%s%s" % (class_name, sep, lua_name), params, ret_lua)


def emit_function(func_el):
    """ Emit a free/global function """

    func_name = func_el.get("name")
    params = build_params(func_el.find("args"))
    ret_lua = get_return_lua(func_el)
    emit_callable("function %s" % func_name, params, ret_lua)


def main():
    global g_out_file
    global g_enum_names

    (filenames, out_filename) = parse_commandline()

    roots = []
    for filename in filenames:
        roots.append(et.parse(filename).getroot())

    # First pass: collect every enum name across all files so type mapping works
    # regardless of declaration order.
    for root in roots:
        for enums in root.iter("enums"):
            for enum_el in enums.iter("enum"):
                g_enum_names.append(enum_el.get("name"))

    g_out_file = open(out_filename, "w", newline="\n")

    wlua("---@meta")
    wlua("")
    wlua("-- AnKi engine Lua scripting API.")
    wlua("-- AUTO GENERATED by AnKi/Script/LuaDefsGen.py from the binding XML.")
    wlua("-- This file is only consumed by the Lua Language Server; do not load it at runtime.")
    wlua("")

    for root in roots:
        for enums in root.iter("enums"):
            for enum_el in enums.iter("enum"):
                emit_enum(enum_el)

    for root in roots:
        for classes in root.iter("classes"):
            for class_el in classes.iter("class"):
                emit_class(class_el)

    for root in roots:
        for functions in root.iter("functions"):
            for func_el in functions.iter("function"):
                emit_function(func_el)

    g_out_file.close()


if __name__ == "__main__":
    main()
