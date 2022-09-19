// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// ShaderVariableDataType defines

// ANKI_SVDT_MACRO(const, varType, baseType, rowCount, columnCount, isIntegralType)
#if defined(ANKI_SVDT_MACRO)
ANKI_SVDT_MACRO(I8, I8, 1, 1, true)
ANKI_SVDT_MACRO(U8, U8, 1, 1, true)
ANKI_SVDT_MACRO(I16, I16, 1, 1, true)
ANKI_SVDT_MACRO(U16, U16, 1, 1, true)
ANKI_SVDT_MACRO(I32, I32, 1, 1, true)
ANKI_SVDT_MACRO(U32, U32, 1, 1, true)
ANKI_SVDT_MACRO(I64, I64, 1, 1, true)
ANKI_SVDT_MACRO(U64, U64, 1, 1, true)
ANKI_SVDT_MACRO(F16, F16, 1, 1, false)
ANKI_SVDT_MACRO(F32, F32, 1, 1, false)

ANKI_SVDT_MACRO(I8Vec2, I8, 2, 1, true)
ANKI_SVDT_MACRO(U8Vec2, U8, 2, 1, true)
ANKI_SVDT_MACRO(I16Vec2, I16, 2, 1, true)
ANKI_SVDT_MACRO(U16Vec2, U16, 2, 1, true)
ANKI_SVDT_MACRO(IVec2, I32, 2, 1, true)
ANKI_SVDT_MACRO(UVec2, U32, 2, 1, true)
ANKI_SVDT_MACRO(HVec2, F16, 2, 1, false)
ANKI_SVDT_MACRO(Vec2, F32, 2, 1, false)

ANKI_SVDT_MACRO(I8Vec3, I8, 3, 1, true)
ANKI_SVDT_MACRO(U8Vec3, U8, 3, 1, true)
ANKI_SVDT_MACRO(I16Vec3, I16, 3, 1, true)
ANKI_SVDT_MACRO(U16Vec3, U16, 3, 1, true)
ANKI_SVDT_MACRO(IVec3, I32, 3, 1, true)
ANKI_SVDT_MACRO(UVec3, U32, 3, 1, true)
ANKI_SVDT_MACRO(HVec3, F16, 3, 1, false)
ANKI_SVDT_MACRO(Vec3, F32, 3, 1, false)

ANKI_SVDT_MACRO(I8Vec4, I8, 4, 1, true)
ANKI_SVDT_MACRO(U8Vec4, U8, 4, 1, true)
ANKI_SVDT_MACRO(I16Vec4, I16, 4, 1, true)
ANKI_SVDT_MACRO(U16Vec4, U16, 4, 1, true)
ANKI_SVDT_MACRO(IVec4, I32, 4, 1, true)
ANKI_SVDT_MACRO(UVec4, U32, 4, 1, true)
ANKI_SVDT_MACRO(HVec4, F16, 4, 1, false)
ANKI_SVDT_MACRO(Vec4, F32, 4, 1, false)

ANKI_SVDT_MACRO(Mat3, F32, 3, 3, false)
ANKI_SVDT_MACRO(Mat3x4, F32, 3, 4, false)
ANKI_SVDT_MACRO(Mat4, F32, 4, 4, false)
#endif

// ANKI_SVDT_MACRO_OPAQUE(const, varType)
#if defined(ANKI_SVDT_MACRO_OPAQUE)
ANKI_SVDT_MACRO_OPAQUE(Texture1D, texture1D)
ANKI_SVDT_MACRO_OPAQUE(Texture1DArray, texture1DArray)
ANKI_SVDT_MACRO_OPAQUE(Texture2D, texture2D)
ANKI_SVDT_MACRO_OPAQUE(Texture2DArray, texture2DArray)
ANKI_SVDT_MACRO_OPAQUE(Texture3D, texture3D)
ANKI_SVDT_MACRO_OPAQUE(TextureCube, textureCube)
ANKI_SVDT_MACRO_OPAQUE(TextureCubeArray, textureCubeArray)

ANKI_SVDT_MACRO_OPAQUE(Image1D, image1D)
ANKI_SVDT_MACRO_OPAQUE(Image1DArray, image1DArray)
ANKI_SVDT_MACRO_OPAQUE(Image2D, image2D)
ANKI_SVDT_MACRO_OPAQUE(Image2DArray, image2DArray)
ANKI_SVDT_MACRO_OPAQUE(Image3D, image3D)
ANKI_SVDT_MACRO_OPAQUE(ImageCube, imageCube)
ANKI_SVDT_MACRO_OPAQUE(ImageCubeArray, imageCubeArray)

ANKI_SVDT_MACRO_OPAQUE(Sampler, sampler)
#endif
