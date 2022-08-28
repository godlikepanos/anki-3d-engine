//=====================================================================
// Copyright (c) 2021    Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=====================================================================

#ifndef ASPM_GPU
#pragma warning(disable : 4244)
#pragma warning(disable : 4201)
#endif

#include "common_def.h"
#include "bcn_common_api.h"

#ifdef USE_UNITY
#include "./external/bc6h_unity.h"
#endif

#ifdef USE_BETSY
#include "./external/bc6h_betsy.h"
#endif

#ifdef USE_MSC
#include "./external/bc6h_msc.h"
#endif

static CGU_Vec4ui CompressBlockBC6H_UNORM(CMP_IN CGU_Vec3f image_src[16], CMP_IN CGU_FLOAT fquality)
{

    CGU_Vec4ui block = {0u, 0u, 0u, 0u};

#ifdef USE_BETSY
    CGU_UINT32 cmp_out[4];
    cmp_BC6H(cmp_out, image_src);
    block.x = cmp_out[0];
    block.y = cmp_out[1];
    block.z = cmp_out[2];
    block.w = cmp_out[3];
#else
    (fquality);     // unreferenced formal parameter
    (image_src);    // unreferenced formal parameter
#endif

    return block;
}
