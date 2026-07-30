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
import re

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
    """ Parse a type text and strip cv/ref/ptr qualifiers. A full expression that can be parsed is: WeakArray<const type&>. Keep this in sync with
    LuaGlueGen.py's parse_type_decl() """

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

    qual = m.group("qual")

    return (m.group("type"), qual == "&", qual == "*", m.group("const") is not None, m.group("array") is not None)


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

    (type, is_ref, is_ptr, is_const, is_weak_array) = parse_type_decl(anki_type)

    if type == "void" or type == "Error":
        return None
    if type_is_bool(type):
        lua = "boolean"
    elif type_is_number(type):
        lua = "number"
    elif type == "char" or type == "CString":
        lua = "string"
    elif type_is_enum(type):
        # Enums are passed/returned as plain numbers in Lua but the global table
        # (e.g. SkyboxComponentType.kGenerated) resolves to an integer.
        lua = "integer"
    else:
        # User class type
        lua = type

    # A WeakArray is passed as a Lua table with 1-based integer keys
    return (lua + "[]") if is_weak_array else lua


def to_camel(s):
    """ FooBar -> fooBar """
    return s[0].lower() + s[1:] if s else s


def param_base_name(anki_type):
    """ Pick a readable parameter name based on the type """

    (type, is_ref, is_ptr, is_const, is_weak_array) = parse_type_decl(anki_type)

    if type_is_bool(type):
        name = "b"
    elif type_is_number(type):
        name = "num"
    elif type == "char" or type == "CString":
        name = "str"
    else:
        name = to_camel(type)

    return (name + "s") if is_weak_array else name


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


def emit_overloaded(decl, els, ret_override=None):
    """ Emit a single declaration for a group of same-named overloads. The signature with the most args becomes the primary one so the richest hint
    shows up and the rest become ---@overload lines. Emitting one declaration per overload instead would make the language server report a duplicate
    definition. """

    sigs = [(build_params(el.find("args")), ret_override or get_return_lua(el)) for el in els]
    sigs.sort(key=lambda sig: len(sig[0]), reverse=True)
    overloads = [params_to_fun(params, ret_lua) for (params, ret_lua) in sigs[1:]]
    emit_callable(decl, sigs[0][0], sigs[0][1], overloads)


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
        emit_overloaded("function %s.new" % class_name, list(constructors_el.iter("constructor")), class_name)

    # Methods. Group the same-named ones because the XML expresses an overload as a repeated <method>
    if meths_el is not None:
        groups = group_by_lua_name(meths_el.iter("method"), method_lua_name)

        for ((lua_name, is_static), meth_els) in groups:
            sep = "." if is_static else ":"
            emit_overloaded("function %s%s%s" % (class_name, sep, lua_name), meth_els)


def method_lua_name(meth_el):
    """ The name a method is bound under, or None if it shouldn't appear as a method at all. Mirrors LuaGlueGen.py's get_meth_alias() """

    meth_name = meth_el.get("name")

    # Arithmetic operators are already covered by @operator on the class header
    if meth_name in OPERATOR_MAP:
        return None
    # Comparison metamethods are implicit in Lua; skip them
    if meth_name.startswith("operator") and meth_name != "operator=":
        return None

    # operator= is bound as a regular method called "copy"
    lua_name = "copy" if meth_name == "operator=" else meth_name
    return (lua_name, meth_el.get("static") == "1")


def group_by_lua_name(els, key_func):
    """ Group elements by the key key_func returns, preserving the XML order of both the groups and the elements within a group. Elements whose key is
    None are dropped. """

    groups = []
    key_to_group = {}

    for el in els:
        key = key_func(el)
        if key is None:
            continue

        if key not in key_to_group:
            key_to_group[key] = []
            groups.append((key, key_to_group[key]))

        key_to_group[key].append(el)

    return groups


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

    # Global functions share a single Lua namespace, so group across every <functions> block of every file
    func_els = []
    for root in roots:
        for functions in root.iter("functions"):
            func_els += list(functions.iter("function"))

    for ((func_name, ), func_group) in group_by_lua_name(func_els, lambda el: (el.get("name"), )):
        emit_overloaded("function %s" % func_name, func_group)

    g_out_file.close()


if __name__ == "__main__":
    main()
