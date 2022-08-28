//=========================================================================
// Copyright (c) 2021    Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
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

#include "common_def.h"
#include "bcn_common_api.h"

#ifdef USE_CMPMSC
#include "bc7_cmpmsc.h"
#endif

#ifdef USE_MSC
#include "./external/bc7_msc.h"
#endif

#ifdef USE_MSC16
#include "./external/bc7_msc16.h"
#endif

#ifdef USE_MSC1
#include "./external/bc7_msc1.h"
#endif

#ifdef USE_INT
#include "./external/bc7_int.h"
#endif

#ifdef USE_RGBCX_RDO
#include "./external/bc7_rgbcx_rdo.h"
#endif

#ifdef USE_VOLT
#include "./external/bc7_volt.h"
#endif

#ifdef USE_ICBC
#include "./external/bc7_icbc.h"
#endif

#ifdef USE_ARRIS
#include "./external/bc7_arris.h"
#endif

static CGU_Vec4ui CompressBlockBC7_UNORM(CMP_IN CGU_Vec4f image_src[16], CMP_IN CGU_FLOAT fquality)
{
#ifdef USE_CMPMSC
    return CompressBlockBC7_CMPMSC(image_src, fquality);
#endif
#ifdef USE_INT
    return CompressBlockBC7_INT(image_src, fquality);
#endif
#ifdef USE_RGBCX_RDO
    return CompressBlockBC7_RGBCX(image_src, fquality);
#endif
#ifdef USE_VOLT
    return CompressBlockBC7_VOLT(image_src, fquality);
#endif
#ifdef USE_ICBC
    return CompressBlockBC7_ICBC(image_src, fquality);
#endif
#if defined(USE_MSC)
    CGU_Vec4ui res = {0, 0, 0, 0};
    return res;
#endif
#ifdef USE_ARRIS
    return CompressBlockBC7_ARRIS(image_src, fquality);
#endif

#ifndef USE_NEW_SINGLE_HEADER_INTERFACES
    CGU_Vec4ui cmp = {0,0,0,0};
    CMP_UNUSED(fquality);
    CMP_UNUSED(image_src);
    return cmp;
#endif
}
