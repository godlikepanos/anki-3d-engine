#!/usr/bin/python3

# Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import enum
import copy

signatures = """
gvec4 texture(gsampler1D sampler, float P [,float bias] )
gvec4 texture(gsampler2D sampler, vec2 P [,float bias] )
gvec4 texture(gsampler3D sampler, vec3 P [, float bias] )
gvec4 texture(gsamplerCube sampler, vec3 P[, float bias] )
float texture(sampler1DShadow sampler, vec3 P [, float bias])
float texture(sampler2DShadow sampler, vec3 P [, float bias])
float texture(samplerCubeShadow sampler, vec4 P [, float bias] )
gvec4 texture(gsampler2DArray sampler, vec3 P [, float bias] )
gvec4 texture(gsamplerCubeArray sampler, vec4 P [, float bias] )
gvec4 texture(gsampler1DArray sampler, vec2 P [, float bias] )
float texture(sampler1DArrayShadow sampler, vec3 P [, float bias] )
float texture(sampler2DArrayShadow sampler, vec4 P)
float texture(samplerCubeArrayShadow sampler, vec4 P, float compare)

gvec4 textureProj(gsampler1D sampler, vec2 P [, float bias] )
gvec4 textureProj(gsampler1D sampler, vec4 P [, float bias] )
gvec4 textureProj(gsampler2D sampler, vec3 P [, float bias] )
gvec4 textureProj(gsampler2D sampler, vec4 P [, float bias] )
gvec4 textureProj(gsampler3D sampler, vec4 P [, float bias] )
float textureProj(sampler1DShadow sampler, vec4 P [, float bias] )
float textureProj(sampler2DShadow sampler, vec4 P [, float bias] )

gvec4 textureLod(gsampler1D sampler, float P, float lod)
gvec4 textureLod(gsampler2D sampler, vec2 P, float lod)
gvec4 textureLod(gsampler3D sampler, vec3 P, float lod)
gvec4 textureLod(gsamplerCube sampler, vec3 P, float lod)
float textureLod(sampler2DShadow sampler, vec3 P, float lod)
float textureLod(sampler1DShadow sampler, vec3 P, float lod)
gvec4 textureLod(gsampler1DArray sampler, vec2 P, float lod)
float textureLod(sampler1DArrayShadow sampler, vec3 P, float lod)
gvec4 textureLod(gsampler2DArray sampler, vec3 P, float lod)
gvec4 textureLod(gsamplerCubeArray sampler, vec4 P, float lod)

gvec4 textureProjLod(gsampler1D sampler, vec2 P, float lod)
gvec4 textureProjLod(gsampler1D sampler, vec4 P, float lod)
gvec4 textureProjLod(gsampler2D sampler, vec3 P, float lod)
gvec4 textureProjLod(gsampler2D sampler, vec4 P, float lod)
gvec4 textureProjLod(gsampler3D sampler, vec4 P, float lod)
float textureProjLod(sampler1DShadow sampler, vec4 P, float lod)
float textureProjLod(sampler2DShadow sampler, vec4 P, float lod)

gvec4 textureGrad(gsampler1D sampler, float P, float dPdx, float dPdy)
gvec4 textureGrad(gsampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy)
gvec4 textureGrad(gsampler3D sampler, vec3 P, vec3 dPdx, vec3 dPdy)
gvec4 textureGrad(gsamplerCube sampler, vec3 P, vec3 dPdx, vec3 dPdy)
float textureGrad(sampler1DShadow sampler, vec3 P, float dPdx, float dPdy)
gvec4 textureGrad(gsampler1DArray sampler, vec2 P, float dPdx, float dPdy)
gvec4 textureGrad(gsampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy)
float textureGrad(sampler1DArrayShadow sampler, vec3 P, float dPdx, float dPdy)
float textureGrad(sampler2DShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy)
float textureGrad(samplerCubeShadow sampler, vec4 P, vec3 dPdx, vec3 dPdy)
float textureGrad(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy)
gvec4 textureGrad(gsamplerCubeArray sampler, vec4 P, vec3 dPdx, vec3 dPdy)

gvec4 textureProjGrad(gsampler1D sampler, vec2 P, float dPdx, float dPdy)
gvec4 textureProjGrad(gsampler1D sampler, vec4 P, float dPdx, float dPdy)
gvec4 textureProjGrad(gsampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy)
gvec4 textureProjGrad(gsampler2D sampler, vec4 P, vec2 dPdx, vec2 dPdy)
gvec4 textureProjGrad(gsampler3D sampler, vec4 P, vec3 dPdx, vec3 dPdy)
float textureProjGrad(sampler1DShadow sampler, vec4 P, float dPdx, float dPdy)
float textureProjGrad(sampler2DShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy)
"""


class Type(enum.Enum):
    NONE = 0
    BASIC = 1
    VEC = 2
    COMBINED_IMAGE_SAMPLER = 3
    SAMPLER = 4
    TEXTURE = 5


class TypeFlag(enum.IntEnum):
    NONE = 0
    FLOAT = 1 << 0
    INT = 1 << 1
    UINT = 1 << 2
    GENERIC = 1 << 3
    ARRAY = 1 << 4
    SHADOW = 1 << 5
    OPTIONAL_ARG = 1 << 6


class TypeInfo:
    def __init__(self):
        self.type = Type.NONE
        self.type_flags = TypeFlag.NONE
        self.dim = 0

    def __str__(self):
        return str(self.__class__) + ": " + str(self.__dict__)


class Arg:
    def __init__(self):
        self.type = TypeInfo()
        self.name = None


class Function:
    def __init__(self):
        self.return_type = TypeInfo()
        self.func_name = None
        self.args = []
        self.is_generic = False
        self.has_optional_arg = False
        self.all_shader_stages = False

    def __str__(self):
        return str(self.__class__) + ": " + str(self.__dict__)


def tokenize(line):
    out = []
    line = line.replace("(", " ")
    line = line.replace(")", " ")
    line = line.replace(",", " ")
    line = line.replace("[", " ")
    line = line.replace("]", " ")
    tokens = line.split(" ")
    for token in tokens:
        token = token.strip()
        if len(token) == 0:
            continue
        out.append(token)
    return out


def parse_type(s):
    type = TypeInfo()
    if s.find("sampler") >= 0:
        type.type = Type.COMBINED_IMAGE_SAMPLER

        if s[0] == "g":
            type.type_flags |= TypeFlag.GENERIC
        elif s[0] == "s":
            type.type_flags |= TypeFlag.FLOAT
        elif s[0] == "u":
            type.type_flags |= TypeFlag.UINT
        elif s[0] == "i":
            type.type_flags |= TypeFlag.INT
        else:
            assert (False)

        if s.find("1D") >= 0:
            type.dim = 1
        elif s.find("2D") >= 0:
            type.dim = 2
        elif s.find("3D") >= 0:
            type.dim = 3
        elif s.find("Cube") >= 0:
            type.dim = 6

        if s.find("Array") >= 0:
            type.type_flags |= TypeFlag.ARRAY

        if s.find("Shadow") >= 0:
            type.type_flags |= TypeFlag.SHADOW

    elif s.find("vec") >= 0:
        type.type = Type.VEC

        if s[0] == "g":
            type.type_flags |= TypeFlag.GENERIC
        elif s[0] == "u":
            type.type_flags |= TypeFlag.UINT
        elif s[0] == "i":
            type.type_flags |= TypeFlag.INT
        elif s[0] == "v":
            type.type_flags |= TypeFlag.FLOAT
        else:
            assert (False)

        if s[-1] == "4":
            type.dim = 4
        elif s[-1] == "3":
            type.dim = 3
        elif s[-1] == "2":
            type.dim = 2
        else:
            assert (False)
    elif s == "float":
        type.dim = 1
        type.type_flags |= TypeFlag.FLOAT
        type.type = Type.BASIC
    elif s == "int":
        type.dim = 1
        type.type_flags |= TypeFlag.INT
        type.type = Type.BASIC
    else:
        assert (False)

    return type


def parse_func(tokens):
    func = Function()

    func.return_type = parse_type(tokens[0])
    func.is_generic = func.is_generic or (
        func.return_type.type_flags & TypeFlag.GENERIC)

    func.func_name = tokens[1]

    if func.func_name.find("Lod") >= 0:
        func.all_shader_stages = True
    else:
        func.all_shader_stages = False

    size = len(tokens)
    for i in range(2, size, 2):
        arg = Arg()

        arg.type = parse_type(tokens[i])
        arg.name = tokens[i + 1]

        if arg.name == "bias":
            arg.type.type_flags |= TypeFlag.OPTIONAL_ARG
            func.has_optional_arg = True

        func.is_generic = func.is_generic or (
            arg.type.type_flags & TypeFlag.GENERIC)

        func.args.append(arg)

    return func


def write_type(type):
    out = ""
    if type.type == Type.BASIC and type.type_flags & TypeFlag.FLOAT:
        out += "float"
    elif type.type == Type.BASIC and type.type_flags & TypeFlag.UINT:
        out += "uint"
    elif type.type == Type.BASIC and type.type_flags & TypeFlag.INT:
        out += "int"
    elif type.type == Type.VEC and type.type_flags & TypeFlag.FLOAT:
        out += "vec" + str(type.dim)
    elif type.type == Type.VEC and type.type_flags & TypeFlag.UINT:
        out += "uvec" + str(type.dim)
    elif type.type == Type.VEC and type.type_flags & TypeFlag.INT:
        out += "ivec" + str(type.dim)
    elif type.type == Type.COMBINED_IMAGE_SAMPLER:
        if type.type_flags & TypeFlag.UINT:
            out += "u"
        elif type.type_flags & TypeFlag.INT:
            out += "i"

        out += "sampler"

        if type.dim == 1:
            out += "1D"
        elif type.dim == 2:
            out += "2D"
        elif type.dim == 3:
            out += "3D"
        elif type.dim == 6:
            out += "Cube"

        if type.type_flags & TypeFlag.ARRAY:
            out += "Array"

        if type.type_flags & TypeFlag.SHADOW:
            out += "Shadow"
    elif type.type == Type.TEXTURE:
        if type.type_flags & TypeFlag.UINT:
            out += "u"
        elif type.type_flags & TypeFlag.INT:
            out += "i"

        out += "texture"

        if type.dim == 1:
            out += "1D"
        elif type.dim == 2:
            out += "2D"
        elif type.dim == 3:
            out += "3D"
        elif type.dim == 6:
            out += "Cube"

        if type.type_flags & TypeFlag.ARRAY:
            out += "Array"
    elif type.type == Type.SAMPLER:
        out += "sampler"
        if type.type_flags & TypeFlag.SHADOW:
            out += "Shadow"
    else:
        assert False

    return out


def gen_split_sampler_tex_func(func):
    out = ""

    # Generate new function signatures
    func_instances = []
    if func.is_generic:
        for generic_type in [TypeFlag.FLOAT, TypeFlag.UINT, TypeFlag.INT]:
            new_func = copy.deepcopy(func)

            # Return
            if new_func.return_type.type_flags & TypeFlag.GENERIC:
                new_func.return_type.type_flags &= ~TypeFlag.GENERIC
                new_func.return_type.type_flags |= generic_type

            # Args
            for arg in new_func.args:
                if arg.type.type_flags & TypeFlag.GENERIC:
                    arg.type.type_flags &= ~TypeFlag.GENERIC
                    arg.type.type_flags |= generic_type

            func_instances.append(new_func)
    else:
        func_instances.append(copy.deepcopy(func))

    # Populate optional arg version
    if func.has_optional_arg:
        extra_func_instances = []
        for func in func_instances:
            new_func = copy.deepcopy(func)

            assert new_func.args[-1].type.type_flags & TypeFlag.OPTIONAL_ARG
            new_func.args.pop()
            extra_func_instances.append(new_func)

        func_instances.extend(extra_func_instances)

    for func in func_instances:
        # Shader guard
        if not func.all_shader_stages:
            out += "#if defined(ANKI_FRAGMENT_SHADER)\n"

        # Return type
        out += write_type(func.return_type) + " "

        # Func name
        out += func.func_name
        out += "("

        # Args
        for i in range(len(func.args)):
            arg = func.args[i]

            if arg.type.type == Type.COMBINED_IMAGE_SAMPLER:
                # Write texture
                tex_type = copy.deepcopy(arg.type)
                tex_type.type = Type.TEXTURE
                out += write_type(tex_type)
                out += " tex, "

                # Write sampler
                sampler_type = copy.deepcopy(arg.type)
                sampler_type.type = Type.SAMPLER
                out += write_type(sampler_type)
                out += " sampl"
            else:
                out += write_type(arg.type)
                out += " " + arg.name

            if i < len(func.args) - 1:
                out += ", "

        out += ")\n"

        # Body
        out += "{\n"

        out += "\treturn " + func.func_name + "("

        for i in range(len(func.args)):
            arg = func.args[i]

            if arg.type.type == Type.COMBINED_IMAGE_SAMPLER:
                out += write_type(arg.type) + "(tex, sampl)"
            else:
                out += arg.name

            if i < len(func.args) - 1:
                out += ", "
            else:
                out += ");\n"

        out += "}\n"

        # Shader guard
        if not func.all_shader_stages:
            out += "#endif\n"

        out += "\n"

    return out


def main():
    print(
        """// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: THIS FILE IS AUTO-GENERATED. DON'T CHANGE IT BY HAND.
// This file contains some convenience texture functions.

#pragma once
""")
    lines = signatures.splitlines()

    for line in lines:
        line = line.strip()

        # Skip empty
        if len(line) == 0:
            continue

        # Parse the func signature
        func = parse_func(tokenize(line))

        # Write the alternative func
        func_code = gen_split_sampler_tex_func(func)
        print(func_code)


#    print("""layout(location = 0) out vec4 out_color;
#void main()
#{
#	out_color = vec4(0);
#}""")

if __name__ == "__main__":
    main()
