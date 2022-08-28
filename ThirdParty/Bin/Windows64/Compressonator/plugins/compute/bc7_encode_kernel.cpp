//===================================================================================
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
//==================================================================================
// Ref: GPUOpen-Tools/Compressonator

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------
// Common BC7 Header
//--------------------------------------
#include "bc7_encode_kernel.h"

//#define USE_ICMP

#ifndef ASPM_OPENCL
//#define USE_NEW_SINGLE_HEADER_INTERFACES
#endif

#ifdef USE_NEW_SINGLE_HEADER_INTERFACES
#define USE_CMPMSC
//#define USE_MSC
//#define USE_INT
//#define USE_RGBCX_RDO
//#define USE_VOLT
//#define USE_ICBC
#endif

#include "bc7_common_encoder.h"

#ifndef ASPM
//---------------------------------------------
// Predefinitions for GPU and CPU compiled code
//---------------------------------------------
INLINE CGU_INT a_compare(const void* arg1, const void* arg2)
{
    if (((CMP_di*)arg1)->image - ((CMP_di*)arg2)->image > 0)
        return 1;
    if (((CMP_di*)arg1)->image - ((CMP_di*)arg2)->image < 0)
        return -1;
    return 0;
};
#endif

#ifndef ASPM_GPU
CMP_GLOBAL BC7_EncodeRamps BC7EncodeRamps
#ifndef ASPM
    = {0}
#endif
;

//---------------------------------------------
// CPU: Computes max of two float values
//---------------------------------------------
float bc7_maxf(float l1, float r1)
{
    return (l1 > r1 ? l1 : r1);
}

//---------------------------------------------
// CPU: Computes max of two float values
//---------------------------------------------
float bc7_minf(float l1, float r1)
{
    return (l1 < r1 ? l1 : r1);
}

#endif

INLINE CGV_INT shift_right_epocode(CGV_INT v, CGU_INT bits)
{
    return v >> bits;  // (perf warning expected)
}

INLINE CGV_INT expand_epocode(CGV_INT v, CGU_INT bits)
{
    CGV_INT vv = v << (8 - bits);
    return vv + shift_right_epocode(vv, bits);
}

// valid bit range is 0..8
CGU_INT expandbits(CGU_INT bits, CGU_INT v)
{
    return (v << (8 - bits) | v >> (2 * bits - 8));
}

CMP_EXPORT CGU_INT bc7_isa()
{
#ifndef ASPM_GPU
#if defined(ISPC_TARGET_SSE2)
    ASPM_PRINT(("SSE2"));
    return 0;
#elif defined(ISPC_TARGET_SSE4)
    ASPM_PRINT(("SSE4"));
    return 1;
#elif defined(ISPC_TARGET_AVX)
    ASPM_PRINT(("AVX"));
    return 2;
#elif defined(ISPC_TARGET_AVX2)
    ASPM_PRINT(("AVX2"));
    return 3;
#else
    ASPM_PRINT(("CPU"));
#endif
#endif
    return -1;
}

CMP_EXPORT void init_BC7ramps()
{
#ifdef ASPM_GPU
#else
    CMP_STATIC CGU_BOOL g_rampsInitialized = FALSE;
    if (g_rampsInitialized == TRUE)
        return;
    g_rampsInitialized       = TRUE;
    BC7EncodeRamps.ramp_init = TRUE;

    //bc7_isa(); ASPM_PRINT((" INIT Ramps\n"));

    CGU_INT bits;
    CGU_INT p1;
    CGU_INT p2;
    CGU_INT clogBC7;
    CGU_INT index;
    CGU_INT j;
    CGU_INT o1;
    CGU_INT o2;
    CGU_INT maxi = 0;

    for (bits = BIT_BASE; bits < BIT_RANGE; bits++)
    {
        for (p1 = 0; p1 < (1 << bits); p1++)
        {
            BC7EncodeRamps.ep_d[BTT(bits)][p1] = expandbits(bits, p1);
        }  //p1
    }      //bits<BIT_RANGE

    for (clogBC7 = LOG_CL_BASE; clogBC7 < LOG_CL_RANGE; clogBC7++)
    {
        for (bits = BIT_BASE; bits < BIT_RANGE; bits++)
        {

#ifdef USE_BC7_RAMP
            for (p1 = 0; p1 < (1 << bits); p1++)
            {
                for (p2 = 0; p2 < (1 << bits); p2++)
                {
                    for (index = 0; index < (1 << clogBC7); index++)
                    {
                        if (index > maxi)
                            maxi = index;
                        BC7EncodeRamps.ramp[(CLT(clogBC7) * 4 * 256 * 256 * 16) + (BTT(bits) * 256 * 256 * 16) + (p1 * 256 * 16) + (p2 * 16) + index] =
                            //floor((CGV_FLOAT)BC7EncodeRamps.ep_d[BTT(bits)][p1] + rampWeights[clogBC7][index] * (CGV_FLOAT)((BC7EncodeRamps.ep_d[BTT(bits)][p2] - BC7EncodeRamps.ep_d[BTT(bits)][p1]))+ 0.5F);
                            floor(BC7EncodeRamps.ep_d[BTT(bits)][p1] +
                                  rampWeights[clogBC7][index] * ((BC7EncodeRamps.ep_d[BTT(bits)][p2] - BC7EncodeRamps.ep_d[BTT(bits)][p1])) + 0.5F);
                    }  //index<(1 << clogBC7)
                }      //p2<(1 << bits)
            }          //p1<(1 << bits)
#endif

#ifdef USE_BC7_SP_ERR_IDX
            for (j = 0; j < 256; j++)
            {
                for (o1 = 0; o1 < 2; o1++)
                {
                    for (o2 = 0; o2 < 2; o2++)
                    {
                        for (index = 0; index < 16; index++)
                        {
                            BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) + (j * 2 * 2 * 16 * 2) +
                                                  (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 0] = 0;
                            BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) + (j * 2 * 2 * 16 * 2) +
                                                  (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 1] = 255;
                            BC7EncodeRamps.sp_err[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16) + (BTT(bits) * 256 * 2 * 2 * 16) + (j * 2 * 2 * 16) + (o1 * 2 * 16) +
                                                  (o2 * 16) + index]                                   = 255;
                        }  // i<16
                    }      //o2<2;
                }          //o1<2
            }              //j<256

            for (p1 = 0; p1 < (1 << bits); p1++)
            {
                for (p2 = 0; p2 < (1 << bits); p2++)
                {
                    for (index = 0; index < (1 << clogBC7); index++)
                    {
#ifdef USE_BC7_RAMP
                        CGV_INT floatf =
                            (CGV_INT)
                                BC7EncodeRamps.ramp[(CLT(clogBC7) * 4 * 256 * 256 * 16) + (BTT(bits) * 256 * 256 * 16) + (p1 * 256 * 16) + (p2 * 16) + index];
#else
                        CGV_INT floatf =
                            floor((CGV_FLOAT)BC7EncodeRamps.ep_d[BTT(bits)][p1] +
                                  rampWeights[clogBC7][index] * (CGV_FLOAT)((BC7EncodeRamps.ep_d[BTT(bits)][p2] - BC7EncodeRamps.ep_d[BTT(bits)][p1])) + 0.5F);
#endif
                        BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) + (floatf * 2 * 2 * 16 * 2) +
                                              ((p1 & 0x1) * 2 * 16 * 2) + ((p2 & 0x1) * 16 * 2) + (index * 2) + 0] = p1;
                        BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) + (floatf * 2 * 2 * 16 * 2) +
                                              ((p1 & 0x1) * 2 * 16 * 2) + ((p2 & 0x1) * 16 * 2) + (index * 2) + 1] = p2;
                        BC7EncodeRamps.sp_err[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16) + (BTT(bits) * 256 * 2 * 2 * 16) + (floatf * 2 * 2 * 16) +
                                              ((p1 & 0x1) * 2 * 16) + (p2 & 0x1 * 16) + index]                     = 0;
                    }  //i<(1 << clogBC7)
                }      //p2
            }          //p1<(1 << bits)

            for (j = 0; j < 256; j++)
            {
                for (o1 = 0; o1 < 2; o1++)
                {
                    for (o2 = 0; o2 < 2; o2++)
                    {
                        for (index = 0; index < (1 << clogBC7); index++)
                        {
                            if (  // check for unitialized sp_idx
                                (BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) + (j * 2 * 2 * 16 * 2) +
                                                       (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 0] == 0) &&
                                (BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) + (j * 2 * 2 * 16 * 2) +
                                                       (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 1] == 255))

                            {
                                CGU_INT k;
                                CGU_INT tf;
                                CGU_INT tc;

                                for (k = 1; k < 256; k++)
                                {
                                    tf = j - k;
                                    tc = j + k;
                                    if ((tf >= 0 && BC7EncodeRamps.sp_err[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16) + (BTT(bits) * 256 * 2 * 2 * 16) +
                                                                          (tf * 2 * 2 * 16) + (o1 * 2 * 16) + (o2 * 16) + index] == 0))
                                    {
                                        BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) +
                                                              (j * 2 * 2 * 16 * 2) + (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 0] =
                                            BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) +
                                                                  (tf * 2 * 2 * 16 * 2) + (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 0];
                                        BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) +
                                                              (j * 2 * 2 * 16 * 2) + (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 1] =
                                            BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) +
                                                                  (tf * 2 * 2 * 16 * 2) + (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 1];
                                        break;
                                    }
                                    else if ((tc < 256 && BC7EncodeRamps.sp_err[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16) + (BTT(bits) * 256 * 2 * 2 * 16) +
                                                                                (tc * 2 * 2 * 16) + (o1 * 2 * 16) + (o2 * 16) + index] == 0))
                                    {
                                        BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) +
                                                              (j * 2 * 2 * 16 * 2) + (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 0] =
                                            BC7EncodeRamps.sp_idx[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits) * 256 * 2 * 2 * 16 * 2) +
                                                                  (tc * 2 * 2 * 16 * 2) + (o1 * 2 * 16 * 2) + (o2 * 16 * 2) + (index * 2) + 0];
                                        break;
                                    }
                                }

                                //BC7EncodeRamps.sp_err[(CLT(clogBC7)*4*256*2*2*16)+(BTT(bits)*256*2*2*16)+(j*2*2*16)+(o1*2*16)+(o2*16)+index] = (CGV_FLOAT) k;
                                BC7EncodeRamps.sp_err[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16) + (BTT(bits) * 256 * 2 * 2 * 16) + (j * 2 * 2 * 16) +
                                                      (o1 * 2 * 16) + (o2 * 16) + index] = (CGU_UINT8)k;

                            }  //sp_idx < 0
                        }      //i<(1 << clogBC7)
                    }          //o2
                }              //o1
            }                  //j
#endif

        }  //bits<BIT_RANGE
    }      //clogBC7<LOG_CL_RANGE
#endif
}

//----------------------------------------------------------
//====== Common BC7 ASPM Code used for SPMD (CPU/GPU) ======
//----------------------------------------------------------

#define SOURCE_BLOCK_SIZE 16      // Size of a source block in pixels (each pixel has RGBA:8888 channels)
#define COMPRESSED_BLOCK_SIZE 16  // Size of a compressed block in bytes
#define MAX_CHANNELS 4
#define MAX_SUBSETS 3       // Maximum number of possible subsets
#define MAX_SUBSET_SIZE 16  // Largest possible size for an individual subset

#ifndef ASPM_GPU
extern "C" CGU_INT timerStart(CGU_INT id);
extern "C" CGU_INT timerEnd(CGU_INT id);

#define TIMERSTART(x) timerStart(x)
#define TIMEREND(x) timerEnd(x)
#else
#define TIMERSTART(x)
#define TIMEREND(x)
#endif

#ifdef ASPM_GPU
#define GATHER_UINT8(x, y) x[y]
#else
#define GATHER_UINT8(x, y) gather_uint8(x, y)
#endif
// INLINE CGV_UINT8  gather_uint8 (CMP_CONSTANT  CGU_UINT8 * __constant uniform ptr, CGV_INT idx)
// {
//    return ptr[idx]; // (perf warning expected)
// }
//
// INLINE CGV_UINT8 gather_cmpout(CMP_CONSTANT CGV_UINT8 * __constant uniform ptr, CGU_INT idx)
// {
//    return ptr[idx]; // (perf warning expected)
// }
//
//INLINE CGV_UINT8 gather_index(CMP_CONSTANT varying CGV_UINT8* __constant uniform ptr, CGV_INT idx)
//{
//   return ptr[idx]; // (perf warning expected)
//}
//
//INLINE void       scatter_index(CGV_UINT8* ptr, CGV_INT idx, CGV_UINT8 value)
//{
//   ptr[idx] = value; // (perf warning expected)
//}
//

#ifdef USE_VARYING
INLINE CGV_INT gather_epocode(CMP_CONSTANT CGV_INT* ptr, CGV_INT idx)
{
    return ptr[idx];  // (perf warning expected)
}
#endif

INLINE CGV_UINT32 gather_partid(CMP_CONSTANT CGV_UINT32* uniform ptr, CGV_INT idx)
{
    return ptr[idx];  // (perf warning expected)
}

//INLINE CGV_UINT8 gather_vuint8(CMP_CONSTANT varying CGV_UINT8* __constant uniform ptr, CGV_INT idx)
//{
//   return ptr[idx]; // (perf warning expected)
//}

INLINE void cmp_swap_epo(CGV_INT u[], CGV_INT v[], CGV_INT n)
{
    for (CGU_INT i = 0; i < n; i++)
    {
        CGV_INT t = u[i];
        u[i]      = v[i];
        v[i]      = t;
    }
}

INLINE void cmp_swap_index(CGV_UINT8 u[], CGV_UINT8 v[], CGU_INT n)
{
    for (CGU_INT i = 0; i < n; i++)
    {
        CGV_UINT8 t = u[i];
        u[i]        = v[i];
        v[i]        = t;
    }
}

void cmp_memsetBC7(CGV_UINT8 ptr[], CGV_UINT8 value, CGU_UINT32 size)
{
    for (CGV_UINT32 i = 0; i < size; i++)
    {
        ptr[i] = value;
    }
}

void cmp_memcpy(CMP_GLOBAL CGU_UINT8 dst[], CGU_UINT8 src[], CGU_UINT32 size)
{
#ifdef ASPM_GPU
    for (CGV_INT i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
#else
    memcpy(dst, src, size);
#endif
}

INLINE CGV_FLOAT sq_image(CGV_FLOAT v)
{
    return v * v;
}

INLINE CGV_INT clampEPO(CGV_INT v, CGV_INT a, CGV_INT b)
{
    if (v < a)
        return a;
    else if (v > b)
        return b;
    return v;
}

INLINE CGV_UINT8 clampIndex(CGV_UINT8 v, CGV_UINT8 a, CGV_UINT8 b)
{
    if (v < a)
        return a;
    else if (v > b)
        return b;
    return v;
}

INLINE CGV_UINT32 shift_right_uint32(CGV_UINT32 v, CGU_INT bits)
{
    return v >> bits;  // (perf warning expected)
}

INLINE CGV_UINT8 shift_right_uint8(CGV_UINT8 v, CGU_UINT8 bits)
{
    return v >> bits;  // (perf warning expected)
}

INLINE CGV_UINT8 shift_right_uint8V(CGV_UINT8 v, CGV_UINT8 bits)
{
    return v >> bits;  // (perf warning expected)
}

// valid bit range is 0..8
INLINE CGV_INT expandEPObits(CGV_INT v, uniform CGV_INT bits)
{
    CGV_INT vv = v << (8 - bits);
    return vv + shift_right_uint32(vv, bits);
}

CGV_FLOAT err_absf(CGV_FLOAT a)
{
    return a > 0.0F ? a : -a;
}
CGV_FLOAT img_absf(CGV_FLOAT a)
{
    return a > 0.0F ? a : -a;
}

CGU_UINT8 min8(CGU_UINT8 a, CGU_UINT8 b)
{
    return a < b ? a : b;
}
CGU_UINT8 max8(CGU_UINT8 a, CGU_UINT8 b)
{
    return a > b ? a : b;
}

void pack_index(CGV_UINT32 packed_index[2], CGV_UINT8 src_index[MAX_SUBSET_SIZE])
{
    // Converts from unpacked index to packed index
    packed_index[0] = 0x0000;
    packed_index[1] = 0x0000;
    CGV_UINT8 shift = 0;  // was CGV_UINT8
    for (CGU_INT k = 0; k < 16; k++)
    {
        packed_index[k / 8] |= (CGV_UINT32)(src_index[k] & 0x0F) << shift;
        shift += 4;
    }
}

void unpack_index(CGV_UINT8 unpacked_index[MAX_SUBSET_SIZE], CGV_UINT32 src_packed[2])
{
    // Converts from packed index to unpacked index
    CGV_UINT8 shift = 0;  // was CGV_UINT8
    for (CGV_UINT8 k = 0; k < 16; k++)
    {
        unpacked_index[k] = (CGV_UINT8)(src_packed[k / 8] >> shift) & 0xF;
        if (k == 7)
            shift = 0;
        else
            shift += 4;
    }
}

//====================================== CMP MATH UTILS  ============================================
CGV_FLOAT err_Total(CGV_FLOAT image_src1[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                    CGV_FLOAT image_src2[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                    CGV_INT   numEntries,  // < 16
                    CGU_UINT8 channels3or4)
{  // IN:  3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
    CGV_FLOAT err_t = 0.0F;
    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        for (CGV_INT k = 0; k < numEntries; k++)
        {
            err_t = err_t + sq_image(image_src1[k + ch * SOURCE_BLOCK_SIZE] - image_src2[k + ch * SOURCE_BLOCK_SIZE]);
        }
    return err_t;
};

void GetImageCentered(CGV_FLOAT image_centered_out[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                      CGV_FLOAT mean_out[MAX_CHANNELS],
                      CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                      CGV_INT   numEntries,  // < 16
                      CGU_UINT8 channels3or4)
{  // IN:  3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        mean_out[ch] = 0.0F;
        if (numEntries > 0)
        {
            for (CGV_INT k = 0; k < numEntries; k++)
            {
                mean_out[ch] = mean_out[ch] + image_src[k + (ch * SOURCE_BLOCK_SIZE)];
            }
            mean_out[ch] /= numEntries;
            for (CGV_INT k = 0; k < numEntries; k++)
                image_centered_out[k + (ch * SOURCE_BLOCK_SIZE)] = image_src[k + (ch * SOURCE_BLOCK_SIZE)] - mean_out[ch];
        }
    }
}

void GetCovarianceVector(CGV_FLOAT covariance_out[MAX_CHANNELS * MAX_CHANNELS],  // OUT: Covariance vector
                         CGV_FLOAT image_centered[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                         CGV_INT   numEntries,  // < 16
                         CGU_UINT8 channels3or4)
{  // IN:  3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
    for (CGU_UINT8 ch1 = 0; ch1 < channels3or4; ch1++)
        for (CGU_UINT8 ch2 = 0; ch2 <= ch1; ch2++)
        {
            covariance_out[ch1 + ch2 * 4] = 0;
            for (CGV_INT k = 0; k < numEntries; k++)
                covariance_out[ch1 + ch2 * 4] += image_centered[k + (ch1 * SOURCE_BLOCK_SIZE)] * image_centered[k + (ch2 * SOURCE_BLOCK_SIZE)];
        }

    for (CGU_UINT8 ch1 = 0; ch1 < channels3or4; ch1++)
        for (CGU_UINT8 ch2 = ch1 + 1; ch2 < channels3or4; ch2++)
            covariance_out[ch1 + ch2 * 4] = covariance_out[ch2 + ch1 * 4];
}

void GetProjecedImage(CGV_FLOAT projection_out[SOURCE_BLOCK_SIZE],  //output projected data
                      CGV_FLOAT image_centered[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                      CGV_INT   numEntries,  // < 16
                      CGV_FLOAT EigenVector[MAX_CHANNELS],
                      CGU_UINT8 channels3or4)
{  // 3 = RGB or 4 = RGBA
    projection_out[0] = 0.0F;

    // EigenVector must be normalized
    for (CGV_INT k = 0; k < numEntries; k++)
    {
        projection_out[k] = 0.0F;
        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            projection_out[k] = projection_out[k] + (image_centered[k + (ch * SOURCE_BLOCK_SIZE)] * EigenVector[ch]);
        }
    }
}

INLINE CGV_UINT8 get_partition_subset(CGV_INT part_id, CGU_INT maxSubsets, CGV_INT index)
{
    if (maxSubsets == 2)
    {
        CGV_UINT32 mask_packed = subset_mask_table[part_id];
        return ((mask_packed & (0x01 << index)) ? 1 : 0);  // This can be moved to caller, just return mask!!
    }

    // 3 region subsets
    part_id += 64;
    CGV_UINT32 mask0 = subset_mask_table[part_id] & 0xFFFF;
    CGV_UINT32 mask1 = subset_mask_table[part_id] >> 16;
    CGV_UINT32 mask  = 0x01 << index;

    return ((mask1 & mask) ? 2 : 0 + (mask0 & mask) ? 1 : 0);  // This can be moved to caller, just return mask!!
}

void GetPartitionSubSet_mode01237(CGV_FLOAT subsets_out[MAX_SUBSETS][SOURCE_BLOCK_SIZE][MAX_CHANNELS],  // OUT: Subset pattern mapped with image src colors
                                  CGV_INT   entryCount_out[MAX_SUBSETS],                                // OUT: Number of entries per subset
                                  CGV_UINT8 partition,                                                  // Partition Shape 0..63
                                  CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],                // Image colors
                                  CGU_INT   blockMode,                                                  // [0,1,2,3 or 7]
                                  CGU_UINT8 channels3or4)
{  // 3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
    CGU_UINT8 maxSubsets = 2;
    if (blockMode == 0 || blockMode == 2)
        maxSubsets = 3;

    entryCount_out[0] = 0;
    entryCount_out[1] = 0;
    entryCount_out[2] = 0;

    for (CGV_INT i = 0; i < MAX_SUBSET_SIZE; i++)
    {
        CGV_UINT8 subset = get_partition_subset(partition, maxSubsets, i);

        for (CGU_INT ch = 0; ch < 3; ch++)
            subsets_out[subset][entryCount_out[subset]][ch] = image_src[i + (ch * SOURCE_BLOCK_SIZE)];
        //subsets_out[subset*64+(entryCount_out[subset]*MAX_CHANNELS+ch)] = image_src[i+(ch*SOURCE_BLOCK_SIZE)];

        // if we have only 3 channels then set the alpha subset to 0
        if (channels3or4 == 3)
            subsets_out[subset][entryCount_out[subset]][3] = 0.0F;
        else
            subsets_out[subset][entryCount_out[subset]][3] = image_src[i + (COMP_ALPHA * SOURCE_BLOCK_SIZE)];
        entryCount_out[subset]++;
    }
}

INLINE void GetClusterMean(CGV_FLOAT cluster_mean_out[SOURCE_BLOCK_SIZE][MAX_CHANNELS],
                           CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                           CGV_UINT8 index_in[MAX_SUBSET_SIZE],
                           CGV_INT   numEntries,  // < 16
                           CGU_UINT8 channels3or4)
{  // IN: 3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
    // unused index values are underfined
    CGV_UINT8 i_cnt[MAX_SUBSET_SIZE];
    CGV_UINT8 i_comp[MAX_SUBSET_SIZE];

    for (CGV_INT i = 0; i < numEntries; i++)
        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            CGV_UINT8 idx             = index_in[i] & 0x0F;
            cluster_mean_out[idx][ch] = 0;
            i_cnt[idx]                = 0;
        }

    CGV_UINT8 ic = 0;  // was CGV_INT
    for (CGV_INT i = 0; i < numEntries; i++)
    {
        CGV_UINT8 idx = index_in[i] & 0x0F;
        if (i_cnt[idx] == 0)
            i_comp[ic++] = idx;
        i_cnt[idx]++;

        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            cluster_mean_out[idx][ch] += image_src[i + (ch * SOURCE_BLOCK_SIZE)];
        }
    }

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        for (CGU_INT i = 0; i < ic; i++)
        {
            if (i_cnt[i_comp[i]] != 0)
            {
                CGV_UINT8 icmp             = i_comp[i];
                cluster_mean_out[icmp][ch] = (CGV_FLOAT)floor((cluster_mean_out[icmp][ch] / (CGV_FLOAT)i_cnt[icmp]) + 0.5F);
            }
        }
}

INLINE void GetImageMean(CGV_FLOAT image_mean_out[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                         CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                         CGV_INT   numEntries,
                         CGU_UINT8 channels)
{
    for (CGU_UINT8 ch = 0; ch < channels; ch++)
        image_mean_out[ch] = 0;

    for (CGV_INT i = 0; i < numEntries; i++)
        for (CGU_UINT8 ch = 0; ch < channels; ch++)
            image_mean_out[ch] += image_src[i + ch * SOURCE_BLOCK_SIZE];

    for (CGU_UINT8 ch = 0; ch < channels; ch++)
        image_mean_out[ch] /= (CGV_FLOAT)numEntries;  // Performance Warning: Conversion from unsigned int to float is slow. Use "int" if possible
}

// calculate an eigen vector corresponding to a biggest eigen value
// will work for non-zero non-negative matricies only
void GetEigenVector(CGV_FLOAT EigenVector_out[MAX_CHANNELS],                  // Normalized Eigen Vector output
                    CGV_FLOAT CovarianceVector[MAX_CHANNELS * MAX_CHANNELS],  // Covariance Vector
                    CGU_UINT8 channels3or4)
{  // IN: 3 = RGB or 4 = RGBA
    CGV_FLOAT vector_covIn[MAX_CHANNELS * MAX_CHANNELS];
    CGV_FLOAT vector_covOut[MAX_CHANNELS * MAX_CHANNELS];
    CGV_FLOAT vector_maxCovariance;

    for (CGU_UINT8 ch1 = 0; ch1 < channels3or4; ch1++)
        for (CGU_UINT8 ch2 = 0; ch2 < channels3or4; ch2++)
        {
            vector_covIn[ch1 + ch2 * 4] = CovarianceVector[ch1 + ch2 * 4];
        }

    vector_maxCovariance = 0;

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        if (vector_covIn[ch + ch * 4] > vector_maxCovariance)
            vector_maxCovariance = vector_covIn[ch + ch * 4];
    }

    // Normalize Input Covariance Vector
    for (CGU_UINT8 ch1 = 0; ch1 < channels3or4; ch1++)
        for (CGU_UINT8 ch2 = 0; ch2 < channels3or4; ch2++)
        {
            if (vector_maxCovariance > 0)
                vector_covIn[ch1 + ch2 * 4] = vector_covIn[ch1 + ch2 * 4] / vector_maxCovariance;
        }

    for (CGU_UINT8 ch1 = 0; ch1 < channels3or4; ch1++)
    {
        for (CGU_UINT8 ch2 = 0; ch2 < channels3or4; ch2++)
        {
            CGV_FLOAT vector_temp_cov = 0;
            for (CGU_UINT8 ch3 = 0; ch3 < channels3or4; ch3++)
            {
                vector_temp_cov = vector_temp_cov + vector_covIn[ch1 + ch3 * 4] * vector_covIn[ch3 + ch2 * 4];
            }
            vector_covOut[ch1 + ch2 * 4] = vector_temp_cov;
        }
    }

    vector_maxCovariance = 0;

    CGV_INT maxCovariance_channel = 0;

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        if (vector_covOut[ch + ch * 4] > vector_maxCovariance)
        {
            maxCovariance_channel = ch;
            vector_maxCovariance  = vector_covOut[ch + ch * 4];
        }
    }

    CGV_FLOAT vector_t = 0;

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        vector_t            = vector_t + vector_covOut[maxCovariance_channel + ch * 4] * vector_covOut[maxCovariance_channel + ch * 4];
        EigenVector_out[ch] = vector_covOut[maxCovariance_channel + ch * 4];
    }

    // Normalize the Eigen Vector
    vector_t = sqrt(vector_t);
    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        if (vector_t > 0)
            EigenVector_out[ch] = EigenVector_out[ch] / vector_t;
    }
}

CGV_UINT8 index_collapse(CGV_UINT8 index[MAX_SUBSET_SIZE], CGV_INT numEntries)
{
    CGV_UINT8 minIndex = index[0];
    CGV_UINT8 MaxIndex = index[0];

    for (CGV_INT k = 1; k < numEntries; k++)
    {
        if (index[k] < minIndex)
            minIndex = index[k];
        if (index[k] > MaxIndex)
            MaxIndex = index[k];
    }

    CGV_UINT8 D = 1;

    for (CGV_UINT8 d = 2; d <= MaxIndex - minIndex; d++)
    {
        for (CGV_INT ent = 0; ent < numEntries; ent++)
        {
            if ((index[ent] - minIndex) % d != 0)
            {
                if (ent >= numEntries)
                    D = d;
                break;
            }
        }
    }

    for (CGV_INT k = 0; k < numEntries; k++)
    {
        index[k] = (index[k] - minIndex) / D;
    }

    for (CGV_INT k = 1; k < numEntries; k++)
    {
        if (index[k] > MaxIndex)
            MaxIndex = index[k];
    }

    return (MaxIndex);
}

void sortProjected_indexs(CGV_UINT8 index_ordered[MAX_SUBSET_SIZE],
                          CGV_FLOAT projection[SOURCE_BLOCK_SIZE],
                          CGV_INT   numEntries  // max 16
)
{
    CMP_di what[SOURCE_BLOCK_SIZE];

    for (CGV_UINT8 i = 0; i < numEntries; i++)
    {
        what[i].index = i;
        what[i].image = projection[i];
    }

    CGV_UINT8 tmp_index;
    CGV_FLOAT tmp_image;

    for (CGV_INT i = 1; i < numEntries; i++)
    {
        for (CGV_INT j = i; j > 0; j--)
        {
            if (what[j - 1].image > what[j].image)
            {
                tmp_index         = what[j].index;
                tmp_image         = what[j].image;
                what[j].index     = what[j - 1].index;
                what[j].image     = what[j - 1].image;
                what[j - 1].index = tmp_index;
                what[j - 1].image = tmp_image;
            }
        }
    }

    for (CGV_INT i = 0; i < numEntries; i++)
        index_ordered[i] = what[i].index;
};

void sortPartitionProjection(CGV_FLOAT projection[MAX_PARTITION_ENTRIES],
                             CGV_UINT8 order[MAX_PARTITION_ENTRIES],
                             CGU_UINT8 numPartitions  // max 64
)
{
    CMP_du what[MAX_PARTITION_ENTRIES];

    for (CGU_UINT8 Parti = 0; Parti < numPartitions; Parti++)
    {
        what[Parti].index = Parti;
        what[Parti].image = projection[Parti];
    }

    CGV_UINT8 index;
    CGV_FLOAT data;

    for (CGU_UINT8 Parti = 1; Parti < numPartitions; Parti++)
    {
        for (CGU_UINT8 Partj = Parti; Partj > 0; Partj--)
        {
            if (what[Partj - 1].image > what[Partj].image)
            {
                index                 = what[Partj].index;
                data                  = what[Partj].image;
                what[Partj].index     = what[Partj - 1].index;
                what[Partj].image     = what[Partj - 1].image;
                what[Partj - 1].index = index;
                what[Partj - 1].image = data;
            }
        }
    }

    for (CGU_UINT8 Parti = 0; Parti < numPartitions; Parti++)
        order[Parti] = what[Parti].index;
};

void cmp_Write8Bit(CGV_UINT8 base[], CGU_INT* uniform offset, CGU_INT bits, CGV_UINT8 bitVal)
{
    base[*offset / 8] |= bitVal << (*offset % 8);
    if (*offset % 8 + bits > 8)
    {
        base[*offset / 8 + 1] |= shift_right_uint8(bitVal, 8 - *offset % 8);
    }
    *offset += bits;
}

void cmp_Write8BitV(CGV_UINT8 base[], CGV_INT offset, CGU_INT bits, CGV_UINT8 bitVal)
{
    base[offset / 8] |= bitVal << (offset % 8);
    if (offset % 8 + bits > 8)
    {
        base[offset / 8 + 1] |= shift_right_uint8V(bitVal, 8 - offset % 8);
    }
}

INLINE CGV_INT ep_find_floor(CGV_FLOAT v, CGU_UINT8 bits, CGV_UINT8 use_par, CGV_UINT8 odd)
{
    CGV_INT i1 = 0;
    CGV_INT i2 = 1 << (bits - use_par);
    odd        = use_par ? odd : 0;
    while (i2 - i1 > 1)
    {
        CGV_INT j    = (i1 + i2) / 2;  // Warning in ASMP code
        CGV_INT ep_d = expandEPObits((j << use_par) + odd, bits);
        if (v >= ep_d)
            i1 = j;
        else
            i2 = j;
    }

    return (i1 << use_par) + odd;
}

//==========================================================

// Not used for Modes 4&5
INLINE CGV_FLOAT GetRamp(CGU_INT   clogBC7,  // ramp bits Valid range 2..4
                         CGU_INT   bits,     // Component Valid range 5..8
                         CGV_INT   p1,       // 0..255
                         CGV_INT   p2,       // 0..255
                         CGV_UINT8 index)
{                // 0..15
#ifdef ASPM_GPU  // GPU Code
    CGV_FLOAT rampf = 0.0F;
    CGV_INT   e1    = expand_epocode(p1, bits);
    CGV_INT   e2    = expand_epocode(p2, bits);
    CGV_FLOAT ramp  = gather_epocode(rampI, clogBC7 * 16 + index) / 64.0F;
    rampf           = floor(e1 + ramp * (e2 - e1) + 0.5F);  // returns 0..255 values
    return rampf;
#else  // CPU ASPM Code
#ifdef USE_BC7_RAMP
    CGV_FLOAT rampf = BC7EncodeRamps.ramp[(CLT(clogBC7) * 4 * 256 * 256 * 16) + (BTT(bits) * 256 * 256 * 16) + (p1 * 256 * 16) + (p2 * 16) + index];
    return rampf;
#else
    return (CGV_FLOAT)floor((CGV_FLOAT)BC7EncodeRamps.ep_d[BTT(bits)][p1] +
                            rampWeights[clogBC7][index] * (CGV_FLOAT)((BC7EncodeRamps.ep_d[BTT(bits)][p2] - BC7EncodeRamps.ep_d[BTT(bits)][p1])) + 0.5F);
#endif
#endif
}

// Not used for Modes 4&5
INLINE CGV_FLOAT get_sperr(CGU_INT   clogBC7,  // ramp bits Valid range 2..4
                           CGU_INT   bits,     // Component Valid range 5..8
                           CGV_INT   p1,       // 0..255
                           CGU_INT   t1,
                           CGU_INT   t2,
                           CGV_UINT8 index)
{
#ifdef ASPM_GPU
    return 0.0F;
#else
#ifdef USE_BC7_SP_ERR_IDX
    if (BC7EncodeRamps.ramp_init)
        return BC7EncodeRamps
            .sp_err[(CLT(clogBC7) * 4 * 256 * 2 * 2 * 16) + (BTT(bits) * 256 * 2 * 2 * 16) + (p1 * 2 * 2 * 16) + (t1 * 2 * 16) + (t2 * 16) + index];
    else
        return 0.0F;
#else
    return 0.0F;
#endif
#endif
}

INLINE void get_fixuptable(CGV_INT fixup[3], CGV_INT part_id)
{
    CGV_INT skip_packed = FIXUPINDEX[part_id];  // gather_int2(FIXUPINDEX, part_id);
    fixup[0]            = 0;
    fixup[1]            = skip_packed >> 4;
    fixup[2]            = skip_packed & 15;
}

//===================================== COMPRESS CODE =============================================
INLINE void SetDefaultIndex(CGV_UINT8 index_io[MAX_SUBSET_SIZE])
{
    // Use this a final call
    for (CGU_INT i = 0; i < MAX_SUBSET_SIZE; i++)
        index_io[i] = 0;
}

INLINE void SetDefaultEPOCode(CGV_INT epo_code_io[8], CGV_INT R, CGV_INT G, CGV_INT B, CGV_INT A)
{
    epo_code_io[0] = R;
    epo_code_io[1] = G;
    epo_code_io[2] = B;
    epo_code_io[3] = A;
    epo_code_io[4] = R;
    epo_code_io[5] = G;
    epo_code_io[6] = B;
    epo_code_io[7] = A;
}

void GetProjectedIndex(CGV_UINT8 projected_index_out[MAX_SUBSET_SIZE],  //output: index, uncentered, in the range 0..clusters-1
                       CGV_FLOAT image_projected[SOURCE_BLOCK_SIZE],    // image_block points, might be uncentered
                       CGV_INT   clusters,                              // clusters: number of points in the ramp   (max 16)
                       CGV_INT   numEntries)
{  // n - number of points in v_  max 15
    CMP_di    what[SOURCE_BLOCK_SIZE];
    CGV_FLOAT image_v[SOURCE_BLOCK_SIZE];
    CGV_FLOAT image_z[SOURCE_BLOCK_SIZE];
    CGV_FLOAT image_l;
    CGV_FLOAT image_mm;
    CGV_FLOAT image_r  = 0.0F;
    CGV_FLOAT image_dm = 0.0F;
    CGV_FLOAT image_min;
    CGV_FLOAT image_max;
    CGV_FLOAT image_s;

    SetDefaultIndex(projected_index_out);

    image_min = image_projected[0];
    image_max = image_projected[0];

    for (CGV_INT i = 1; i < numEntries; i++)
    {
        if (image_min < image_projected[i])
            image_min = image_projected[i];
        if (image_max > image_projected[i])
            image_max = image_projected[i];
    }

    CGV_FLOAT img_diff = image_max - image_min;

    if (img_diff == 0.0f)
        return;
    if (cmp_isnan(img_diff))
        return;

    image_s = (clusters - 1) / img_diff;

    for (CGV_UINT8 i = 0; i < numEntries; i++)
    {
        image_v[i]             = image_projected[i] * image_s;
        image_z[i]             = floor(image_v[i] + 0.5F - image_min * image_s);
        projected_index_out[i] = (CGV_UINT8)image_z[i];

        what[i].image = image_v[i] - image_z[i] - image_min * image_s;
        what[i].index = i;
        image_dm += what[i].image;
        image_r += what[i].image * what[i].image;
    }

    if (numEntries * image_r - image_dm * image_dm >= (CGV_FLOAT)(numEntries - 1) / 8)
    {
        image_dm /= numEntries;

        for (CGV_INT i = 0; i < numEntries; i++)
            what[i].image -= image_dm;

        CGV_UINT8 tmp_index;
        CGV_FLOAT tmp_image;
        for (CGV_INT i = 1; i < numEntries; i++)
        {
            for (CGV_INT j = i; j > 0; j--)
            {
                if (what[j - 1].image > what[j].image)
                {
                    tmp_index         = what[j].index;
                    tmp_image         = what[j].image;
                    what[j].index     = what[j - 1].index;
                    what[j].image     = what[j - 1].image;
                    what[j - 1].index = tmp_index;
                    what[j - 1].image = tmp_image;
                }
            }
        }

        // got into fundamental simplex
        // move coordinate system origin to its center

        // i=0 < numEntries avoids varying int division by 0
        for (CGV_INT i = 0; i < numEntries; i++)
        {
            what[i].image = what[i].image - (CGV_FLOAT)(((2.0f * i + 1) - numEntries) / (2.0f * numEntries));
        }

        image_mm = 0.0F;
        image_l  = 0.0F;

        CGV_INT j = -1;
        for (CGV_INT i = 0; i < numEntries; i++)
        {
            image_l += what[i].image;
            if (image_l < image_mm)
            {
                image_mm = image_l;
                j        = i;
            }
        }

        j = j + 1;
        // avoid  j = j%numEntries us this
        while (j > numEntries)
            j = j - numEntries;

        for (CGV_INT i = j; i < numEntries; i++)
        {
            CGV_UINT8 idx            = what[i].index;
            CGV_UINT8 pidx           = projected_index_out[idx] + 1;  //gather_index(projected_index_out,idx)+1;
            projected_index_out[idx] = pidx;                          // scatter_index(projected_index_out,idx,pidx);
        }
    }

    // get minimum index
    CGV_UINT8 index_min = projected_index_out[0];
    for (CGV_INT i = 1; i < numEntries; i++)
    {
        if (projected_index_out[i] < index_min)
            index_min = projected_index_out[i];
    }

    // reposition all index by min index (using min index as 0)
    for (CGV_INT i = 0; i < numEntries; i++)
    {
        projected_index_out[i] = clampIndex(projected_index_out[i] - index_min, 0, 15);
    }
}

CGV_FLOAT GetQuantizeIndex(CGV_UINT32 index_packed_out[2],
                           CGV_UINT8  index_out[MAX_SUBSET_SIZE],  // OUT:
                           CGV_FLOAT  image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                           CGV_INT    numEntries,  //IN: range 0..15 (MAX_SUBSET_SIZE)
                           CGU_INT    numClusters,
                           CGU_UINT8  channels3or4)
{  // IN: 3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
    CGV_FLOAT image_centered[SOURCE_BLOCK_SIZE * MAX_CHANNELS];
    CGV_FLOAT image_mean[MAX_CHANNELS];
    CGV_FLOAT eigen_vector[MAX_CHANNELS];
    CGV_FLOAT covariance_vector[MAX_CHANNELS * MAX_CHANNELS];

    GetImageCentered(image_centered, image_mean, image_src, numEntries, channels3or4);
    GetCovarianceVector(covariance_vector, image_centered, numEntries, channels3or4);

    //-----------------------------------------------------
    // check if all covariances are the same
    // if so then set all index to same value 0 and return
    // use EPSILON to set the limit for all same limit
    //-----------------------------------------------------

    CGV_FLOAT image_covt = 0.0F;
    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        image_covt = image_covt + covariance_vector[ch + ch * 4];

    if (image_covt < EPSILON)
    {
        SetDefaultIndex(index_out);
        index_packed_out[0] = 0;
        index_packed_out[1] = 0;
        return 0.;
    }

    GetEigenVector(eigen_vector, covariance_vector, channels3or4);

    CGV_FLOAT image_projected[SOURCE_BLOCK_SIZE];

    GetProjecedImage(image_projected, image_centered, numEntries, eigen_vector, channels3or4);
    GetProjectedIndex(index_out, image_projected, numClusters, numEntries);

    //==========================================
    // Refine
    //==========================================
    CGV_FLOAT image_q = 0.0F;
    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        eigen_vector[ch] = 0;
        for (CGV_INT k = 0; k < numEntries; k++)
            eigen_vector[ch] = eigen_vector[ch] + image_centered[k + (ch * SOURCE_BLOCK_SIZE)] * index_out[k];
        image_q = image_q + eigen_vector[ch] * eigen_vector[ch];
    }

    image_q = sqrt(image_q);

    // direction needs to be normalized
    if (image_q != 0.0F)
        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
            eigen_vector[ch] = eigen_vector[ch] / image_q;

    // Get new projected data
    GetProjecedImage(image_projected, image_centered, numEntries, eigen_vector, channels3or4);
    GetProjectedIndex(index_out, image_projected, numClusters, numEntries);

    // pack the index for use in icmp
    pack_index(index_packed_out, index_out);

    //===========================
    // Calc Error
    //===========================
    // Get the new image based on new index

    CGV_FLOAT image_t       = 0.0F;
    CGV_FLOAT index_average = 0.0F;

    for (CGV_INT ik = 0; ik < numEntries; ik++)
    {
        index_average = index_average + index_out[ik];
        image_t       = image_t + index_out[ik] * index_out[ik];
    }

    index_average = index_average / (CGV_FLOAT)numEntries;
    image_t       = image_t - index_average * index_average * (CGV_FLOAT)numEntries;

    if (image_t != 0.0F)
        image_t = 1.0F / image_t;

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        eigen_vector[ch] = 0;
        for (CGV_INT nk = 0; nk < numEntries; nk++)
            eigen_vector[ch] = eigen_vector[ch] + image_centered[nk + (ch * SOURCE_BLOCK_SIZE)] * index_out[nk];
    }

    CGV_FLOAT image_decomp[SOURCE_BLOCK_SIZE * MAX_CHANNELS];
    for (CGV_INT i = 0; i < numEntries; i++)
        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
            image_decomp[i + (ch * SOURCE_BLOCK_SIZE)] = image_mean[ch] + eigen_vector[ch] * image_t * (index_out[i] - index_average);

    CGV_FLOAT err_1 = err_Total(image_src, image_decomp, numEntries, channels3or4);

    return err_1;
    //    return 0.0F;
}

CGV_FLOAT quant_solid_color(CGV_UINT8 index_out[MAX_SUBSET_SIZE],
                            CGV_INT   epo_code_out[2 * MAX_CHANNELS],
                            CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                            CGV_INT   numEntries,
                            CGU_UINT8 Mi_,      // last cluster
                            CGU_UINT8 bits[3],  // including parity
                            CGU_INT   type,
                            CGU_UINT8 channels3or4  // IN: 3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
)
{
    CGU_INT clogBC7 = 0;
    CGU_INT iv      = Mi_ + 1;
    while (iv >>= 1)
        clogBC7++;

    // init epo_0
    CGV_INT epo_0[2 * MAX_CHANNELS];
    SetDefaultEPOCode(epo_0, 0xFF, 0, 0, 0);

    CGV_UINT8 image_log = 0;
    CGV_UINT8 image_idx = 0;
    CGU_BOOL  use_par   = FALSE;
    if (type != 0)
        use_par = TRUE;
    CGV_FLOAT error_1 = CMP_FLOAT_MAX;

    for (CGU_INT pn = 0; pn < npv_nd[channels3or4 - 3][type] && (error_1 != 0.0F); pn++)
    {
        //1

        CGU_INT o1[2 * MAX_CHANNELS];  // = { 0,2 };
        CGU_INT o2[2 * MAX_CHANNELS];  // = { 0,2 };

        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            //A
            o2[ch] = o1[ch] = 0;
            o2[4 + ch] = o1[4 + ch] = 2;

            if (use_par == TRUE)
            {
                if (par_vectors_nd[channels3or4 - 3][type][pn][0][ch])
                    o1[ch] = 1;
                else
                    o1[4 + ch] = 1;
                if (par_vectors_nd[channels3or4 - 3][type][pn][1][ch])
                    o2[ch] = 1;
                else
                    o2[4 + ch] = 1;
            }
        }  //A

        CGV_INT   image_tcr[MAX_CHANNELS];
        CGV_INT   epo_dr_0[MAX_CHANNELS];
        CGV_FLOAT error_tr;
        CGV_FLOAT error_0 = CMP_FLOAT_MAX;

        for (CGV_UINT8 iclogBC7 = 0; iclogBC7 < (1 << clogBC7) && (error_0 != 0); iclogBC7++)
        {
            //E
            CGV_FLOAT error_t = 0;
            CGV_INT   t1o[MAX_CHANNELS], t2o[MAX_CHANNELS];

            for (CGU_UINT8 ch1 = 0; ch1 < channels3or4; ch1++)
            {
                // D
                CGV_FLOAT error_ta = CMP_FLOAT_MAX;

                for (CGU_INT t1 = o1[ch1]; t1 < o1[4 + ch1]; t1++)
                {
                    // C
                    // This is needed for non-integer mean points of "collapsed" sets
                    for (CGU_INT t2 = o2[ch1]; t2 < o2[4 + ch1]; t2++)
                    {
                        // B
                        CGV_INT image_tf;
                        CGV_INT image_tc;
                        image_tf = (CGV_INT)floor(image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)]);
                        image_tc = (CGV_INT)ceil(image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)]);

#ifdef USE_BC7_SP_ERR_IDX
                        CGV_FLOAT err_tf = get_sperr(clogBC7, bits[ch1], image_tf, t1, t2, iclogBC7);
                        CGV_FLOAT err_tc = get_sperr(clogBC7, bits[ch1], image_tc, t1, t2, iclogBC7);
                        if (err_tf > err_tc)
                            image_tcr[ch1] = image_tc;
                        else if (err_tf < err_tc)
                            image_tcr[ch1] = image_tf;
                        else
                            image_tcr[ch1] = (CGV_INT)floor(image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)] + 0.5F);

                        //image_tcr[ch1] = image_tf + (image_tc - image_tf)/2;

                        //===============================
                        // Refine this for better quality!
                        //===============================
                        error_tr = get_sperr(clogBC7, bits[ch1], image_tcr[ch1], t1, t2, iclogBC7);
                        error_tr = (error_tr * error_tr) + 2 * error_tr * img_absf(image_tcr[ch1] - image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)]) +
                                   (image_tcr[ch1] - image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)]) *
                                       (image_tcr[ch1] - image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)]);

                        if (error_tr < error_ta)
                        {
                            error_ta      = error_tr;
                            t1o[ch1]      = t1;
                            t2o[ch1]      = t2;
                            epo_dr_0[ch1] = clampEPO(image_tcr[ch1], 0, 255);
                        }
#else
                        image_tcr[ch1] = floor(image_src[COMP_RED + (ch1 * SOURCE_BLOCK_SIZE)] + 0.5F);
                        error_ta       = 0;
                        t1o[ch1]       = t1;
                        t2o[ch1]       = t2;
                        epo_dr_0[ch1]  = clampEPO(image_tcr[ch1], 0, 255);
#endif
                    }  // B
                }      //C

                error_t += error_ta;
            }  // D

            if (error_t < error_0)
            {
                // We have a solid color: Use image src if on GPU
                image_log = iclogBC7;
                image_idx = image_log;

#ifdef ASPM_GPU  // This needs improving
                CGV_FLOAT MinC[4] = {255, 255, 255, 255};
                CGV_FLOAT MaxC[4] = {0, 0, 0, 0};
                // get min max colors
                for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
                    for (CGV_INT k = 0; k < numEntries; k++)
                    {
                        if (image_src[k + ch * SOURCE_BLOCK_SIZE] < MinC[ch])
                            MinC[ch] = image_src[k + ch * SOURCE_BLOCK_SIZE];
                        if (image_src[k + ch * SOURCE_BLOCK_SIZE] > MaxC[ch])
                            MaxC[ch] = image_src[k + ch * SOURCE_BLOCK_SIZE];
                    }
                for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
                {
                    epo_0[ch]     = MinC[ch];
                    epo_0[4 + ch] = MaxC[ch];
                }

#else  // This is good on CPU
                for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
                {
#ifdef USE_BC7_SP_ERR_IDX
                    if (BC7EncodeRamps.ramp_init)
                    {
                        CGV_INT index = (CLT(clogBC7) * 4 * 256 * 2 * 2 * 16 * 2) + (BTT(bits[ch]) * 256 * 2 * 2 * 16 * 2) + (epo_dr_0[ch] * 2 * 2 * 16 * 2) +
                                        (t1o[ch] * 2 * 16 * 2) + (t2o[ch] * 16 * 2) + (iclogBC7 * 2);
                        epo_0[ch]     = BC7EncodeRamps.sp_idx[index + 0] & 0xFF;  // gather_epocode(u_BC7Encode->sp_idx,index+0)&0xFF;
                        epo_0[4 + ch] = BC7EncodeRamps.sp_idx[index + 1] & 0xFF;  // gather_epocode(u_BC7Encode->sp_idx,index+1)&0xFF;
                    }
                    else
                    {
                        epo_0[ch]     = 0;
                        epo_0[4 + ch] = 0;
                    }
#else
                    epo_0[ch]     = 0;
                    epo_0[4 + ch] = 0;
#endif
                }
#endif
                error_0 = error_t;
            }
            //if (error_0 == 0)
            //    break;
        }  // E

        if (error_0 < error_1)
        {
            image_idx = image_log;
            for (CGU_UINT8 chE = 0; chE < channels3or4; chE++)
            {
                epo_code_out[chE]     = epo_0[chE];
                epo_code_out[4 + chE] = epo_0[4 + chE];
            }
            error_1 = error_0;
        }

    }  //1

    // Get Image error
    CGV_FLOAT image_decomp[SOURCE_BLOCK_SIZE * MAX_CHANNELS];
    for (CGV_INT i = 0; i < numEntries; i++)
    {
        index_out[i] = image_idx;
        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            image_decomp[i + (ch * SOURCE_BLOCK_SIZE)] = GetRamp(clogBC7, bits[ch], epo_code_out[ch], epo_code_out[4 + ch], image_idx);
        }
    }
    // Do we need to do this rather then err_1 * numEntries
    CGV_FLOAT error_quant;
    error_quant = err_Total(image_src, image_decomp, numEntries, channels3or4);

    return error_quant;
    //return err_1 * numEntries;
}

CGV_FLOAT requantized_image_err(CGV_UINT8 index_out[MAX_SUBSET_SIZE],
                                CGV_INT   epo_code[2 * MAX_CHANNELS],
                                CGU_INT   clogBC7,
                                CGU_UINT8 max_bits[MAX_CHANNELS],
                                CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                                CGV_INT   numEntries,  // max 16
                                CGU_UINT8 channels3or4)
{  // IN: 3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)

    //=========================================
    // requantized image based on new epo_code
    //=========================================
    CGV_FLOAT image_requantize[SOURCE_BLOCK_SIZE][MAX_CHANNELS];
    CGV_FLOAT err_r = 0.0F;

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        for (CGU_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
        {
            image_requantize[k][ch] = GetRamp(clogBC7, max_bits[ch], epo_code[ch], epo_code[4 + ch], (CGV_UINT8)k);
        }
    }

    //=========================================
    // Calc the error for the requantized image
    //=========================================

    for (CGV_INT k = 0; k < numEntries; k++)
    {
        CGV_FLOAT err_cmin     = CMP_FLOAT_MAX;
        CGV_INT   hold_index_j = 0;

        for (CGV_INT iclogBC7 = 0; iclogBC7 < (1 << clogBC7); iclogBC7++)
        {
            CGV_FLOAT image_err = 0.0F;

            for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
            {
                image_err += sq_image(image_requantize[iclogBC7][ch] - image_src[k + (ch * SOURCE_BLOCK_SIZE)]);
            }

            if (image_err < err_cmin)
            {
                err_cmin     = image_err;
                hold_index_j = iclogBC7;
            }
        }

        index_out[k] = (CGV_UINT8)hold_index_j;
        err_r += err_cmin;
    }

    return err_r;
}

CGU_BOOL get_ideal_cluster(CGV_FLOAT image_out[2 * MAX_CHANNELS],
                           CGV_UINT8 index_in[MAX_SUBSET_SIZE],
                           CGU_INT   Mi_,
                           CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                           CGV_INT   numEntries,
                           CGU_UINT8 channels3or4)
{
    // get ideal cluster centers
    CGV_FLOAT image_cluster_mean[SOURCE_BLOCK_SIZE][MAX_CHANNELS];
    GetClusterMean(image_cluster_mean, image_src, index_in, numEntries, channels3or4);  // unrounded

    CGV_FLOAT image_matrix0[2] = {0, 0};   // matrix /inverse matrix
    CGV_FLOAT image_matrix1[2] = {0, 0};   // matrix /inverse matrix
    CGV_FLOAT image_rp[2 * MAX_CHANNELS];  // right part for RMS fit problem

    for (CGU_INT i = 0; i < 2 * MAX_CHANNELS; i++)
        image_rp[i] = 0;

    // weight with cnt if runnning on compacted index
    for (CGV_INT k = 0; k < numEntries; k++)
    {
        image_matrix0[0] += (Mi_ - index_in[k]) * (Mi_ - index_in[k]);
        image_matrix0[1] += index_in[k] * (Mi_ - index_in[k]);  // im is symmetric
        image_matrix1[1] += index_in[k] * index_in[k];

        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            image_rp[ch] += (Mi_ - index_in[k]) * image_cluster_mean[index_in[k]][ch];
            image_rp[4 + ch] += index_in[k] * image_cluster_mean[index_in[k]][ch];
        }
    }

    CGV_FLOAT matrix_dd = image_matrix0[0] * image_matrix1[1] - image_matrix0[1] * image_matrix0[1];

    // assert(matrix_dd !=0);
    // matrix_dd=0 means that index_cidx[k] and (Mi_-index_cidx[k]) collinear which implies only one active index;
    // taken care of separately
    if (matrix_dd == 0)
    {
        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            image_out[ch]     = 0;
            image_out[4 + ch] = 0;
        }
        return FALSE;
    }
    image_matrix1[0] = image_matrix0[0];
    image_matrix0[0] = image_matrix1[1] / matrix_dd;
    image_matrix1[1] = image_matrix1[0] / matrix_dd;
    image_matrix1[0] = image_matrix0[1] = -image_matrix0[1] / matrix_dd;

    CGV_FLOAT Mif = (CGV_FLOAT)Mi_;

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        image_out[ch]     = (image_matrix0[0] * image_rp[ch] + image_matrix0[1] * image_rp[4 + ch]) * Mif;
        image_out[4 + ch] = (image_matrix1[0] * image_rp[ch] + image_matrix1[1] * image_rp[4 + ch]) * Mif;
    }

    return TRUE;
}

CGV_FLOAT shake(CGV_INT   epo_code_shaker_out[2 * MAX_CHANNELS],
                CGV_FLOAT image_ep[2 * MAX_CHANNELS],
                CGV_UINT8 index_cidx[MAX_SUBSET_SIZE],
                CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                CGU_INT   clogBC7,
                CGU_INT   type,
                CGU_UINT8 max_bits[MAX_CHANNELS],
                CGU_UINT8 use_par,
                CGV_INT   numEntries,  // max 16
                CGU_UINT8 channels3or4)
{
#define SHAKESIZE1 1
#define SHAKESIZE2 2
    // shake single or                                   - cartesian
    // shake odd/odd and even/even or                    - same parity
    // shake odd/odd odd/even , even/odd and even/even   - bcc

    CGV_FLOAT best_err = CMP_FLOAT_MAX;

    CGV_FLOAT err_ed[16] = {0};
    CGV_INT   epo_code_par[2][2][2][MAX_CHANNELS];

    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
    {
        CGU_UINT8 ppA = 0;
        CGU_UINT8 ppB = 0;
        CGU_UINT8 rr  = (use_par ? 2 : 1);
        CGV_INT   epo_code_epi[2][2];  // first/second, coord, begin rage end range

        for (ppA = 0; ppA < rr; ppA++)
        {  // loop max =2
            for (ppB = 0; ppB < rr; ppB++)
            {  //loop  max =2

                // set default ranges
                epo_code_epi[0][0] = epo_code_epi[0][1] = ep_find_floor(image_ep[ch], max_bits[ch], use_par, ppA);
                epo_code_epi[1][0] = epo_code_epi[1][1] = ep_find_floor(image_ep[4 + ch], max_bits[ch], use_par, ppB);

                // set begin range
                epo_code_epi[0][0] -= ((epo_code_epi[0][0] < SHAKESIZE1 ? epo_code_epi[0][0] : SHAKESIZE1)) & (~use_par);
                epo_code_epi[1][0] -= ((epo_code_epi[1][0] < SHAKESIZE1 ? epo_code_epi[1][0] : SHAKESIZE1)) & (~use_par);

                // set end range
                epo_code_epi[0][1] +=
                    ((1 << max_bits[ch]) - 1 - epo_code_epi[0][1] < SHAKESIZE2 ? (1 << max_bits[ch]) - 1 - epo_code_epi[0][1] : SHAKESIZE2) & (~use_par);
                epo_code_epi[1][1] +=
                    ((1 << max_bits[ch]) - 1 - epo_code_epi[1][1] < SHAKESIZE2 ? (1 << max_bits[ch]) - 1 - epo_code_epi[1][1] : SHAKESIZE2) & (~use_par);

                CGV_INT step                       = (1 << use_par);
                err_ed[(ppA * 8) + (ppB * 4) + ch] = CMP_FLOAT_MAX;

                for (CGV_INT epo_p1 = epo_code_epi[0][0]; epo_p1 <= epo_code_epi[0][1]; epo_p1 += step)
                {
                    for (CGV_INT epo_p2 = epo_code_epi[1][0]; epo_p2 <= epo_code_epi[1][1]; epo_p2 += step)
                    {
                        CGV_FLOAT image_square_diff = 0.0F;
                        CGV_INT   _mc               = numEntries;
                        CGV_FLOAT image_ramp;

                        while (_mc > 0)
                        {
                            image_ramp = GetRamp(clogBC7, max_bits[ch], epo_p1, epo_p2, index_cidx[_mc - 1]);

                            image_square_diff += sq_image(image_ramp - image_src[(_mc - 1) + (ch * SOURCE_BLOCK_SIZE)]);
                            _mc--;
                        }
                        if (image_square_diff < err_ed[(ppA * 8) + (ppB * 4) + ch])
                        {
                            err_ed[(ppA * 8) + (ppB * 4) + ch] = image_square_diff;
                            epo_code_par[ppA][ppB][0][ch]      = epo_p1;
                            epo_code_par[ppA][ppB][1][ch]      = epo_p2;
                        }
                    }
                }
            }  // pp1
        }      // pp0
    }          // j

    //---------------------------------------------------------
    for (CGU_INT pn = 0; pn < npv_nd[channels3or4 - 3][type]; pn++)
    {
        CGV_FLOAT err_2 = 0.0F;
        CGU_INT   d1;
        CGU_INT   d2;

        for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        {
            d1 = par_vectors_nd[channels3or4 - 3][type][pn][0][ch];
            d2 = par_vectors_nd[channels3or4 - 3][type][pn][1][ch];
            err_2 += err_ed[(d1 * 8) + (d2 * 4) + ch];
        }

        if (err_2 < best_err)
        {
            best_err = err_2;
            for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
            {
                d1                          = par_vectors_nd[channels3or4 - 3][type][pn][0][ch];
                d2                          = par_vectors_nd[channels3or4 - 3][type][pn][1][ch];
                epo_code_shaker_out[ch]     = epo_code_par[d1][d2][0][ch];
                epo_code_shaker_out[4 + ch] = epo_code_par[d1][d2][1][ch];
            }
        }
    }

    return best_err;
}

CGV_FLOAT optimize_IndexAndEndPoints(CGV_UINT8 index_io[MAX_SUBSET_SIZE],
                                     CGV_INT   epo_code_out[8],
                                     CGV_FLOAT image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS],
                                     CGV_INT   numEntries,    // max 16
                                     CGU_UINT8 Mi_,           // last cluster , This should be no larger than 16
                                     CGU_UINT8 bits,          // total for all components
                                     CGU_UINT8 channels3or4,  // IN: 3 = RGB or 4 = RGBA (4 = MAX_CHANNELS)
                                     uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
    CGV_FLOAT err_best = CMP_FLOAT_MAX;
    CGU_INT   type;
    CGU_UINT8 channels2 = 2 * channels3or4;

    type = bits % channels2;

    CGU_UINT8 use_par = (type != 0);

    CGU_UINT8 max_bits[MAX_CHANNELS];
    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
        max_bits[ch] = (bits + channels2 - 1) / channels2;

    CGU_INT iv;
    CGU_INT clogBC7 = 0;
    iv              = Mi_;
    while (iv >>= 1)
        clogBC7++;

    CGU_INT clt_clogBC7 = CLT(clogBC7);

    if (clt_clogBC7 > 3)
    {
        ASPM_PRINT(("Err: optimize_IndexAndEndPoints, clt_clogBC7\n"));
        return CMP_FLOAT_MAX;
    }

    Mi_ = Mi_ - 1;

    CGV_UINT8 MaxIndex;
    CGV_UINT8 index_tmp[MAX_SUBSET_SIZE];
    CGU_INT   maxTry = 5;

    CGV_UINT8 index_best[MAX_SUBSET_SIZE];

    for (CGV_INT k = 0; k < numEntries; k++)
    {
        index_best[k] = index_tmp[k] = clampIndex(index_io[k], 0, 15);
    }

    CGV_INT epo_code_best[2 * MAX_CHANNELS];

    SetDefaultEPOCode(epo_code_out, 0xFF, 0, 0, 0);
    SetDefaultEPOCode(epo_code_best, 0, 0, 0, 0);

    CGV_FLOAT err_requant = 0.0F;

    MaxIndex = index_collapse(index_tmp, numEntries);

    //===============================
    // we have a solid color 4x4 block
    //===============================
    if (MaxIndex == 0)
    {
        return quant_solid_color(index_io, epo_code_out, image_src, numEntries, Mi_, max_bits, type, channels3or4);
    }

    do
    {
        //===============================
        // We have ramp colors to process
        //===============================
        CGV_FLOAT err_cluster = CMP_FLOAT_MAX;
        CGV_FLOAT err_shake;
        CGV_UINT8 index_cluster[MAX_PARTITION_ENTRIES];

        for (CGV_UINT8 index_slope = 1; (MaxIndex != 0) && (index_slope * MaxIndex <= Mi_); index_slope++)
        {
            for (CGV_UINT8 index_offset = 0; index_offset <= Mi_ - index_slope * MaxIndex; index_offset++)
            {
                //-------------------------------------
                // set a new index data to try
                //-------------------------------------
                for (CGV_INT k = 0; k < numEntries; k++)
                    index_cluster[k] = index_tmp[k] * index_slope + index_offset;

                CGV_FLOAT image_cluster[2 * MAX_CHANNELS];
                CGV_INT   epo_code_shake[2 * MAX_CHANNELS];
                SetDefaultEPOCode(epo_code_shake, 0, 0, 0xFF, 0);

                if (get_ideal_cluster(image_cluster, index_cluster, Mi_, image_src, numEntries, channels3or4) == FALSE)
                {
                    break;
                }

                err_shake = shake(epo_code_shake,  // return new epo
                                  image_cluster,
                                  index_cluster,
                                  image_src,
                                  clogBC7,
                                  type,
                                  max_bits,
                                  use_par,
                                  numEntries,  // max 16
                                  channels3or4);

                if (err_shake < err_cluster)
                {
                    err_cluster = err_shake;
                    for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
                    {
                        epo_code_best[ch]     = clampEPO(epo_code_shake[ch], 0, 255);
                        epo_code_best[4 + ch] = clampEPO(epo_code_shake[4 + ch], 0, 255);
                    }
                }
            }
        }

        CGV_INT change = 0;
        CGV_INT better = 0;

        if ((err_cluster != CMP_FLOAT_MAX))
        {
            //=========================
            // test results for quality
            //=========================
            err_requant = requantized_image_err(index_best,     // new index results
                                                epo_code_best,  // prior result input
                                                clogBC7,
                                                max_bits,
                                                image_src,
                                                numEntries,
                                                channels3or4);

            // change/better
            // Has the index values changed from that last set
            for (CGV_INT k = 0; k < numEntries; k++)
                change = change || (index_cluster[k] != index_best[k]);

            if (err_requant < err_best)
            {
                better = 1;
                for (CGV_INT k = 0; k < numEntries; k++)
                {
                    index_io[k] = index_tmp[k] = index_best[k];
                }

                for (CGU_UINT8 ch = 0; ch < channels3or4; ch++)
                {
                    epo_code_out[ch]     = epo_code_best[0 * 4 + ch];
                    epo_code_out[4 + ch] = epo_code_best[1 * 4 + ch];
                }
                err_best = err_requant;
            }
        }

        // Early out if we have our target err
        if (err_best <= u_BC7Encode->errorThreshold)
        {
            break;
        }

        CGV_INT done;
        done = !(change && better);
        if ((maxTry > 0) && (!done))
        {
            maxTry--;
            MaxIndex = index_collapse(index_tmp, numEntries);
        }
        else
        {
            maxTry = 0;
        }

    } while (maxTry);

    if (err_best == CMP_FLOAT_MAX)
    {
        ASPM_PRINT(("Err: requantized_image_err\n"));
    }

    return err_best;
}

CGU_UINT8 get_partitionsToTry(uniform CMP_GLOBAL BC7_Encode u_BC7Encode[], CGU_UINT8 maxPartitions)
{
    CGU_FLOAT u_minPartitionSearchSize = 0.30f;
    if (u_BC7Encode->quality <= BC7_qFAST_THRESHOLD)
    {  // Using this to match performance and quality of CPU code
        u_minPartitionSearchSize = u_minPartitionSearchSize + (u_BC7Encode->quality * BC7_qFAST_THRESHOLD);
    }
    else
    {
        u_minPartitionSearchSize = u_BC7Encode->quality;
    }
    return (CGU_UINT8)(maxPartitions * u_minPartitionSearchSize);
}

INLINE void cmp_encode_swap(CGV_INT endpoint[], CGU_INT channels, CGV_UINT8 block_index[MAX_SUBSET_SIZE], CGU_INT bits)
{
    CGU_INT levels = 1 << bits;
    if (block_index[0] >= levels / 2)
    {
        cmp_swap_epo(&endpoint[0], &endpoint[channels], channels);
        for (CGU_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
#ifdef ASPM_GPU
            block_index[k] = (levels - 1) - block_index[k];
#else
            block_index[k] = CGV_UINT8(levels - 1) - block_index[k];
#endif
    }
}

void cmp_encode_index(CGV_UINT8 data[16], CGU_INT* uniform pPos, CGV_UINT8 block_index[MAX_SUBSET_SIZE], CGU_INT bits)
{
    cmp_Write8Bit(data, pPos, bits - 1, block_index[0]);
    for (CGU_INT j = 1; j < SOURCE_BLOCK_SIZE; j++)
    {
        CGV_UINT8 qbits = block_index[j] & 0xFF;
        cmp_Write8Bit(data, pPos, bits, qbits);
    }
}

void encode_endpoint(CGV_UINT8 data[16], CGU_INT* uniform pPos, CGV_UINT8 block_index[16], CGU_INT bits, CGV_UINT32 flips)
{
    CGU_INT levels        = 1 << bits;
    CGV_INT flips_shifted = flips;
    for (CGU_INT k1 = 0; k1 < 16; k1++)
    {
        CGV_UINT8 qbits_shifted = block_index[k1];
        for (CGU_INT k2 = 0; k2 < 8; k2++)
        {
            CGV_INT q = qbits_shifted & 15;
            if ((flips_shifted & 1) > 0)
                q = (levels - 1) - q;

            if (k1 == 0 && k2 == 0)
                cmp_Write8Bit(data, pPos, bits - 1, CMP_STATIC_CAST(CGV_UINT8, q));
            else
                cmp_Write8Bit(data, pPos, bits, CMP_STATIC_CAST(CGV_UINT8, q));
            qbits_shifted >>= 4;
            flips_shifted >>= 1;
        }
    }
}

INLINE CGV_UINT32 pow32(CGV_UINT32 x)
{
    return 1 << x;
}

void Encode_mode01237(CGU_INT    blockMode,
                      CGV_UINT8  bestPartition,
                      CGV_UINT32 packedEndpoints[6],
                      CGV_UINT8  index16[16],
                      CGV_UINT8  cmp_out[COMPRESSED_BLOCK_SIZE])
{
    CGU_INT    partitionBits;
    CGU_UINT32 componentBits;
    CGU_UINT8  maxSubsets;
    CGU_INT    channels;
    CGU_UINT8  indexBits;

    switch (blockMode)
    {
    case 0:
        componentBits = 4;
        maxSubsets    = 3;
        partitionBits = 4;
        channels      = 3;
        indexBits     = 3;
        break;
    case 2:
        componentBits = 5;
        maxSubsets    = 3;
        partitionBits = 6;
        channels      = 3;
        indexBits     = 2;
        break;
    case 3:
        componentBits = 7;
        maxSubsets    = 2;
        partitionBits = 6;
        channels      = 3;
        indexBits     = 2;
        break;
    case 7:
        componentBits = 5;
        maxSubsets    = 2;
        partitionBits = 6;
        channels      = 4;
        indexBits     = 2;
        break;
    default:
    case 1:
        componentBits = 6;
        maxSubsets    = 2;
        partitionBits = 6;
        channels      = 3;
        indexBits     = 3;
        break;
    }

    CGV_UINT8 blockindex[SOURCE_BLOCK_SIZE];
    CGV_INT   indexBitsV = indexBits;

    for (CGU_INT k = 0; k < COMPRESSED_BLOCK_SIZE; k++)
        cmp_out[k] = 0;

    // mode 0 = 1, mode 1 = 01, mode 2 = 001, mode 3 = 0001, ...
    CGU_INT bitPosition = blockMode;
    cmp_Write8Bit(cmp_out, &bitPosition, 1, 1);

    // Write partition bits
    cmp_Write8Bit(cmp_out, &bitPosition, partitionBits, bestPartition);

    // Sort out the index set and tag whether we need to flip the
    // endpoints to get the correct state in the implicit index bits
    // The implicitly encoded MSB of the fixup index must be 0
    CGV_INT fixup[3];
    get_fixuptable(fixup, (maxSubsets == 2 ? bestPartition : bestPartition + 64));

    // Extract indices and mark subsets that need to have their colours flipped to get the
    // right state for the implicit MSB of the fixup index
    CGV_INT flipColours[3] = {0, 0, 0};

    for (CGV_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
    {
        blockindex[k] = index16[k];
        for (CGU_UINT8 j = 0; j < maxSubsets; j++)
        {
            if (k == fixup[j])
            {
                if (blockindex[k] & (1 << (indexBitsV - 1)))
                {
                    flipColours[j] = 1;
                }
            }
        }
    }

    // Now we must flip the endpoints where necessary so that the implicitly encoded
    // index bits have the correct state
    for (CGU_INT subset = 0; subset < maxSubsets; subset++)
    {
        if (flipColours[subset] == 1)
        {
            CGV_UINT32 temp                 = packedEndpoints[subset * 2 + 0];
            packedEndpoints[subset * 2 + 0] = packedEndpoints[subset * 2 + 1];
            packedEndpoints[subset * 2 + 1] = temp;
        }
    }

    // ...next flip the indices where necessary

    for (CGV_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
    {
        CGV_UINT8 partsub = get_partition_subset(bestPartition, maxSubsets, k);

        if (flipColours[partsub] == 1)
        {
            blockindex[k] = ((1 << indexBitsV) - 1) - blockindex[k];
        }
    }

    // Endpoints are stored in the following order RRRR GGGG BBBB (AAAA) (PPPP)
    // i.e. components are packed together
    CGV_UINT32 unpackedColours[MAX_SUBSETS * 2 * MAX_CHANNELS];
    CGV_UINT8  parityBits[MAX_SUBSETS][2];

    // Unpack the colour values for the subsets
    for (CGU_INT subset = 0; subset < maxSubsets; subset++)
    {
        CGV_UINT32 packedColours[2] = {packedEndpoints[subset * 2 + 0], packedEndpoints[subset * 2 + 1]};

        if (blockMode == 0 || blockMode == 3 || blockMode == 7)
        {  // TWO_PBIT
            parityBits[subset][0] = packedColours[0] & 1;
            parityBits[subset][1] = packedColours[1] & 1;
            packedColours[0] >>= 1;
            packedColours[1] >>= 1;
        }
        else if (blockMode == 1)
        {  // ONE_PBIT
            parityBits[subset][0] = packedColours[1] & 1;
            parityBits[subset][1] = packedColours[1] & 1;
            packedColours[0] >>= 1;
            packedColours[1] >>= 1;
        }
        else if (blockMode == 2)
        {
            parityBits[subset][0] = 0;
            parityBits[subset][1] = 0;
        }

        for (CGU_INT ch = 0; ch < channels; ch++)
        {
            unpackedColours[(subset * 2 + 0) * MAX_CHANNELS + ch] = packedColours[0] & ((1 << componentBits) - 1);
            unpackedColours[(subset * 2 + 1) * MAX_CHANNELS + ch] = packedColours[1] & ((1 << componentBits) - 1);
            packedColours[0] >>= componentBits;
            packedColours[1] >>= componentBits;
        }
    }

    // Loop over component
    for (CGU_INT ch = 0; ch < channels; ch++)
    {
        // loop over subsets
        for (CGU_INT subset = 0; subset < maxSubsets; subset++)
        {
            cmp_Write8Bit(cmp_out, &bitPosition, componentBits, unpackedColours[(subset * 2 + 0) * MAX_CHANNELS + ch] & 0xFF);
            cmp_Write8Bit(cmp_out, &bitPosition, componentBits, unpackedColours[(subset * 2 + 1) * MAX_CHANNELS + ch] & 0xFF);
        }
    }

    // write parity bits
    if (blockMode != 2)
    {
        for (CGV_INT subset = 0; subset < maxSubsets; subset++)
        {
            if (blockMode == 1)
            {  // ONE_PBIT
                cmp_Write8Bit(cmp_out, &bitPosition, 1, parityBits[subset][0] & 0x01);
            }
            else
            {  // TWO_PBIT
                cmp_Write8Bit(cmp_out, &bitPosition, 1, parityBits[subset][0] & 0x01);
                cmp_Write8Bit(cmp_out, &bitPosition, 1, parityBits[subset][1] & 0x01);
            }
        }
    }

    // Encode the index bits
    CGV_INT bitPositionV = bitPosition;
    for (CGV_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
    {
        CGV_UINT8 partsub = get_partition_subset(bestPartition, maxSubsets, k);

        // If this is a fixup index then drop the MSB which is implicitly 0
        if (k == fixup[partsub])
        {
            cmp_Write8BitV(cmp_out, bitPositionV, indexBits - 1, blockindex[k] & 0x07F);
            bitPositionV += indexBits - 1;
        }
        else
        {
            cmp_Write8BitV(cmp_out, bitPositionV, indexBits, blockindex[k]);
            bitPositionV += indexBits;
        }
    }
}

void Encode_mode4(CGV_UINT8 cmp_out[COMPRESSED_BLOCK_SIZE], varying cmp_mode_parameters* uniform params)
{
    CGU_INT bitPosition = 4;  // Position the pointer at the LSB

    for (CGU_INT k = 0; k < COMPRESSED_BLOCK_SIZE; k++)
        cmp_out[k] = 0;

    // mode 4 (5 bits) 00001
    cmp_Write8Bit(cmp_out, &bitPosition, 1, 1);

    // rotation 2 bits
    cmp_Write8Bit(cmp_out, &bitPosition, 2, CMP_STATIC_CAST(CGV_UINT8, params->rotated_channel));

    // idxMode 1 bit
    cmp_Write8Bit(cmp_out, &bitPosition, 1, CMP_STATIC_CAST(CGV_UINT8, params->idxMode));

    CGU_INT idxBits[2] = {2, 3};

    if (params->idxMode)
    {
        idxBits[0] = 3;
        idxBits[1] = 2;
        // Indicate if we need to fixup the index
        cmp_swap_index(params->color_index, params->alpha_index, 16);
        cmp_encode_swap(params->alpha_qendpoint, 4, params->color_index, 2);
        cmp_encode_swap(params->color_qendpoint, 4, params->alpha_index, 3);
    }
    else
    {
        cmp_encode_swap(params->color_qendpoint, 4, params->color_index, 2);
        cmp_encode_swap(params->alpha_qendpoint, 4, params->alpha_index, 3);
    }

    // color endpoints 5 bits each
    // R0 : R1
    // G0 : G1
    // B0 : B1
    for (CGU_INT component = 0; component < 3; component++)
    {
        cmp_Write8Bit(cmp_out, &bitPosition, 5, CMP_STATIC_CAST(CGV_UINT8, params->color_qendpoint[component]));
        cmp_Write8Bit(cmp_out, &bitPosition, 5, CMP_STATIC_CAST(CGV_UINT8, params->color_qendpoint[4 + component]));
    }

    // alpha endpoints (6 bits each)
    // A0 : A1
    cmp_Write8Bit(cmp_out, &bitPosition, 6, CMP_STATIC_CAST(CGV_UINT8, params->alpha_qendpoint[0]));
    cmp_Write8Bit(cmp_out, &bitPosition, 6, CMP_STATIC_CAST(CGV_UINT8, params->alpha_qendpoint[4]));

    // index 2 bits each  (31 bits total)
    cmp_encode_index(cmp_out, &bitPosition, params->color_index, 2);
    // index 3 bits each  (47 bits total)
    cmp_encode_index(cmp_out, &bitPosition, params->alpha_index, 3);
}

void Encode_mode5(CGV_UINT8 cmp_out[COMPRESSED_BLOCK_SIZE], varying cmp_mode_parameters* uniform params)
{
    for (CGU_INT k = 0; k < COMPRESSED_BLOCK_SIZE; k++)
        cmp_out[k] = 0;

    // mode 5 bits = 000001
    CGU_INT bitPosition = 5;  // Position the pointer at the LSB
    cmp_Write8Bit(cmp_out, &bitPosition, 1, 1);

    // Write 2 bit rotation
    cmp_Write8Bit(cmp_out, &bitPosition, 2, CMP_STATIC_CAST(CGV_UINT8, params->rotated_channel));

    cmp_encode_swap(params->color_qendpoint, 4, params->color_index, 2);
    cmp_encode_swap(params->alpha_qendpoint, 4, params->alpha_index, 2);

    // color endpoints (7 bits each)
    // R0 : R1
    // G0 : G1
    // B0 : B1
    for (CGU_INT component = 0; component < 3; component++)
    {
        cmp_Write8Bit(cmp_out, &bitPosition, 7, CMP_STATIC_CAST(CGV_UINT8, params->color_qendpoint[component]));
        cmp_Write8Bit(cmp_out, &bitPosition, 7, CMP_STATIC_CAST(CGV_UINT8, params->color_qendpoint[4 + component]));
    }

    // alpha endpoints (8 bits each)
    // A0 : A1
    cmp_Write8Bit(cmp_out, &bitPosition, 8, CMP_STATIC_CAST(CGV_UINT8, params->alpha_qendpoint[0]));
    cmp_Write8Bit(cmp_out, &bitPosition, 8, CMP_STATIC_CAST(CGV_UINT8, params->alpha_qendpoint[4]));

    // color index 2 bits each  (31 bits total)
    // alpha index 2 bits each  (31 bits total)
    cmp_encode_index(cmp_out, &bitPosition, params->color_index, 2);
    cmp_encode_index(cmp_out, &bitPosition, params->alpha_index, 2);
}

void Encode_mode6(CGV_UINT8 index[MAX_SUBSET_SIZE], CGV_INT epo_code[8], CGV_UINT8 cmp_out[COMPRESSED_BLOCK_SIZE])
{
    for (CGU_INT k = 0; k < COMPRESSED_BLOCK_SIZE; k++)
        cmp_out[k] = 0;

    cmp_encode_swap(epo_code, 4, index, 4);

    // Mode = 6  bits = 0000001
    CGU_INT bitPosition = 6;  // Position the pointer at the LSB
    cmp_Write8Bit(cmp_out, &bitPosition, 1, 1);

    // endpoints
    for (CGU_INT p = 0; p < 4; p++)
    {
        cmp_Write8Bit(cmp_out, &bitPosition, 7, CMP_STATIC_CAST(CGV_UINT8, epo_code[0 + p] >> 1));
        cmp_Write8Bit(cmp_out, &bitPosition, 7, CMP_STATIC_CAST(CGV_UINT8, epo_code[4 + p] >> 1));
    }

    // p bits
    cmp_Write8Bit(cmp_out, &bitPosition, 1, epo_code[0] & 1);
    cmp_Write8Bit(cmp_out, &bitPosition, 1, epo_code[4] & 1);

    // quantized values
    cmp_encode_index(cmp_out, &bitPosition, index, 4);
}

void Compress_mode01237(CGU_INT blockMode, BC7_EncodeState EncodeState[], uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
    CGV_UINT8 storedBestindex[MAX_PARTITIONS][MAX_SUBSETS][MAX_SUBSET_SIZE];
    CGV_FLOAT storedError[MAX_PARTITIONS];
    CGV_UINT8 sortedPartition[MAX_PARTITIONS];

    EncodeState->numPartitionModes = 64;
    EncodeState->maxSubSets        = 2;

    if (blockMode == 0)
    {
        EncodeState->numPartitionModes = 16;
        EncodeState->channels3or4      = 3;
        EncodeState->bits              = 26;
        EncodeState->clusters          = 8;
        EncodeState->componentBits     = 4;
        EncodeState->maxSubSets        = 3;
    }
    else if (blockMode == 2)
    {
        EncodeState->channels3or4  = 3;
        EncodeState->bits          = 30;
        EncodeState->clusters      = 4;
        EncodeState->componentBits = 5;
        EncodeState->maxSubSets    = 3;
    }
    else if (blockMode == 1)
    {
        EncodeState->channels3or4  = 3;
        EncodeState->bits          = 37;
        EncodeState->clusters      = 8;
        EncodeState->componentBits = 6;
    }
    else if (blockMode == 3)
    {
        EncodeState->channels3or4  = 3;
        EncodeState->bits          = 44;
        EncodeState->clusters      = 4;
        EncodeState->componentBits = 7;
    }
    else if (blockMode == 7)
    {
        EncodeState->channels3or4  = 4;
        EncodeState->bits          = 42;  // (2* (R 5 + G 5 + B 5 + A 5)) + 2 parity bits
        EncodeState->clusters      = 4;
        EncodeState->componentBits = 5;  // 5 bit components
    }

    CGV_FLOAT image_subsets[MAX_SUBSETS][MAX_SUBSET_SIZE][MAX_CHANNELS];
    CGV_INT   subset_entryCount[MAX_SUBSETS] = {0, 0, 0};

    // Loop over the available partitions for the block mode and quantize them
    // to figure out the best candidates for further refinement
    CGU_UINT8 mode_partitionsToTry;
    mode_partitionsToTry = get_partitionsToTry(u_BC7Encode, EncodeState->numPartitionModes);

    CGV_UINT8 bestPartition = 0;

    for (CGU_INT mode_blockPartition = 0; mode_blockPartition < mode_partitionsToTry; mode_blockPartition++)
    {
        GetPartitionSubSet_mode01237(
            image_subsets, subset_entryCount, CMP_STATIC_CAST(CGV_UINT8, mode_blockPartition), EncodeState->image_src, blockMode, EncodeState->channels3or4);

        CGV_FLOAT subset_image_src[SOURCE_BLOCK_SIZE * MAX_CHANNELS];
        CGV_UINT8 index_out1[SOURCE_BLOCK_SIZE];
        CGV_FLOAT err_quant = 0.0F;

        // Store the quntize error for this partition to be sorted and processed later
        for (CGU_INT subset = 0; subset < EncodeState->maxSubSets; subset++)
        {
            CGV_INT numEntries = subset_entryCount[subset];

            for (CGU_INT ii = 0; ii < SOURCE_BLOCK_SIZE; ii++)
            {
                subset_image_src[ii + COMP_RED * SOURCE_BLOCK_SIZE]   = image_subsets[subset][ii][0];
                subset_image_src[ii + COMP_GREEN * SOURCE_BLOCK_SIZE] = image_subsets[subset][ii][1];
                subset_image_src[ii + COMP_BLUE * SOURCE_BLOCK_SIZE]  = image_subsets[subset][ii][2];
                subset_image_src[ii + COMP_ALPHA * SOURCE_BLOCK_SIZE] = image_subsets[subset][ii][3];
            }

            CGV_UINT32 color_index2[2];

            err_quant += GetQuantizeIndex(color_index2, index_out1, subset_image_src, numEntries, EncodeState->clusters, EncodeState->channels3or4);

            for (CGV_INT idx = 0; idx < numEntries; idx++)
            {
                storedBestindex[mode_blockPartition][subset][idx] = index_out1[idx];
            }
        }

        storedError[mode_blockPartition] = err_quant;
    }

    // Sort the results
    sortPartitionProjection(storedError, sortedPartition, mode_partitionsToTry);

    CGV_INT   epo_code[MAX_SUBSETS * 2 * MAX_CHANNELS];
    CGV_INT   bestEndpoints[MAX_SUBSETS * 2 * MAX_CHANNELS];
    CGV_UINT8 bestindex[MAX_SUBSETS * MAX_SUBSET_SIZE];
    CGV_INT   bestEntryCount[MAX_SUBSETS];
    CGV_UINT8 bestindex16[MAX_SUBSET_SIZE];

    // Extensive shaking is most important when the ramp is short, and
    // when we have less index. On a long ramp the quality of the
    // initial quantizing is relatively more important
    // We modulate the shake size according to the number of ramp index
    // - the more index we have the less shaking should be required to find a near
    // optimal match

    CGU_UINT8 numShakeAttempts = max8(1, min8((CGU_UINT8)floor(8 * u_BC7Encode->quality + 0.5), mode_partitionsToTry));
    CGV_FLOAT err_best         = CMP_FLOAT_MAX;

    // Now do the endpoint shaking
    for (CGU_INT nSA = 0; nSA < numShakeAttempts; nSA++)
    {
        CGV_FLOAT err_optimized = 0.0F;
        CGV_UINT8 sortedBlockPartition;
        sortedBlockPartition = sortedPartition[nSA];

        //********************************************
        // Get the partition shape for the given mode
        //********************************************
        GetPartitionSubSet_mode01237(image_subsets, subset_entryCount, sortedBlockPartition, EncodeState->image_src, blockMode, EncodeState->channels3or4);

        //*****************************
        // Process the partition shape
        //*****************************
        for (CGU_INT subset = 0; subset < EncodeState->maxSubSets; subset++)
        {
            CGV_INT   numEntries = subset_entryCount[subset];
            CGV_FLOAT src_image_block[SOURCE_BLOCK_SIZE * MAX_CHANNELS];
            CGV_UINT8 index_io[MAX_SUBSET_SIZE];
            CGV_INT   tmp_epo_code[8];

            for (CGU_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
            {
                src_image_block[k + COMP_RED * SOURCE_BLOCK_SIZE]   = image_subsets[subset][k][0];
                src_image_block[k + COMP_GREEN * SOURCE_BLOCK_SIZE] = image_subsets[subset][k][1];
                src_image_block[k + COMP_BLUE * SOURCE_BLOCK_SIZE]  = image_subsets[subset][k][2];
                src_image_block[k + COMP_ALPHA * SOURCE_BLOCK_SIZE] = image_subsets[subset][k][3];
            }

            for (CGU_INT k = 0; k < MAX_SUBSET_SIZE; k++)
            {
                index_io[k] = storedBestindex[sortedBlockPartition][subset][k];
            }

            err_optimized += optimize_IndexAndEndPoints(index_io,
                                                        tmp_epo_code,
                                                        src_image_block,
                                                        numEntries,
                                                        CMP_STATIC_CAST(CGU_INT8, EncodeState->clusters),  // Mi_
                                                        EncodeState->bits,
                                                        EncodeState->channels3or4,
                                                        u_BC7Encode);

            for (CGU_INT k = 0; k < MAX_SUBSET_SIZE; k++)
            {
                storedBestindex[sortedBlockPartition][subset][k] = index_io[k];
            }

            for (CGU_INT ch = 0; ch < MAX_CHANNELS; ch++)
            {
                epo_code[(subset * 2 + 0) * 4 + ch] = tmp_epo_code[ch];
                epo_code[(subset * 2 + 1) * 4 + ch] = tmp_epo_code[4 + ch];
            }
        }

        //****************************************
        // Check if result is better than the last
        //****************************************
        if (err_optimized < err_best)
        {
            bestPartition          = sortedBlockPartition;
            CGV_INT bestIndexCount = 0;

            for (CGU_INT subset = 0; subset < EncodeState->maxSubSets; subset++)
            {
                CGV_INT numEntries     = subset_entryCount[subset];
                bestEntryCount[subset] = numEntries;

                if (numEntries)
                {
                    for (CGU_INT ch = 0; ch < EncodeState->channels3or4; ch++)
                    {
                        bestEndpoints[(subset * 2 + 0) * 4 + ch] = epo_code[(subset * 2 + 0) * 4 + ch];
                        bestEndpoints[(subset * 2 + 1) * 4 + ch] = epo_code[(subset * 2 + 1) * 4 + ch];
                    }

                    for (CGV_INT k = 0; k < numEntries; k++)
                    {
                        bestindex[subset * MAX_SUBSET_SIZE + k] = storedBestindex[sortedBlockPartition][subset][k];
                        bestindex16[bestIndexCount++]           = storedBestindex[sortedBlockPartition][subset][k];
                    }
                }
            }

            err_best = err_optimized;
            // Early out if we  found we can compress with error below the quality threshold
            if (err_best <= u_BC7Encode->errorThreshold)
            {
                break;
            }
        }
    }

    if (blockMode != 7)
        err_best += EncodeState->opaque_err;

    if (err_best > EncodeState->best_err)
        return;

    //**************************
    // Save the encoded block
    //**************************
    EncodeState->best_err = err_best;

    // Now we have all the data needed to encode the block
    // We need to pack the endpoints prior to encoding
    CGV_UINT32 packedEndpoints[MAX_SUBSETS * 2] = {0, 0, 0, 0, 0, 0};
    for (CGU_INT subset = 0; subset < EncodeState->maxSubSets; subset++)
    {
        packedEndpoints[(subset * 2) + 0] = 0;
        packedEndpoints[(subset * 2) + 1] = 0;

        if (bestEntryCount[subset])
        {
            CGU_UINT32 rightAlignment = 0;

            // Sort out parity bits
            if (blockMode != 2)
            {
                // Sort out BCC parity bits
                packedEndpoints[(subset * 2) + 0] = bestEndpoints[(subset * 2 + 0) * 4 + 0] & 1;
                packedEndpoints[(subset * 2) + 1] = bestEndpoints[(subset * 2 + 1) * 4 + 0] & 1;
                for (CGU_INT ch = 0; ch < EncodeState->channels3or4; ch++)
                {
                    bestEndpoints[(subset * 2 + 0) * 4 + ch] >>= 1;
                    bestEndpoints[(subset * 2 + 1) * 4 + ch] >>= 1;
                }
                rightAlignment++;
            }

            // Fixup endpoints
            for (CGU_INT ch = 0; ch < EncodeState->channels3or4; ch++)
            {
                packedEndpoints[(subset * 2) + 0] |= bestEndpoints[((subset * 2) + 0) * 4 + ch] << rightAlignment;
                packedEndpoints[(subset * 2) + 1] |= bestEndpoints[((subset * 2) + 1) * 4 + ch] << rightAlignment;
                rightAlignment += EncodeState->componentBits;
            }
        }
    }

    CGV_UINT8 idxCount[3] = {0, 0, 0};
    for (CGV_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
    {
        CGV_UINT8 partsub = get_partition_subset(bestPartition, EncodeState->maxSubSets, k);
        CGV_UINT8 idxC    = idxCount[partsub];
        bestindex16[k]    = bestindex[partsub * MAX_SUBSET_SIZE + idxC];
        idxCount[partsub] = idxC + 1;
    }

    Encode_mode01237(blockMode, bestPartition, packedEndpoints, bestindex16, EncodeState->cmp_out);
}

void Compress_mode45(CGU_INT blockMode, BC7_EncodeState EncodeState[], uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
    cmp_mode_parameters best_candidate;
    EncodeState->channels3or4 = 4;
    cmp_memsetBC7((CGV_UINT8*)&best_candidate, 0, sizeof(cmp_mode_parameters));

    if (blockMode == 4)
    {
        EncodeState->max_idxMode     = 2;
        EncodeState->modeBits[0]     = 30;  // bits = 2 * (Red 5+ Grn 5+ blu 5)
        EncodeState->modeBits[1]     = 36;  // bits = 2 * (Alpha 6+6+6)
        EncodeState->numClusters0[0] = 4;
        EncodeState->numClusters0[1] = 8;
        EncodeState->numClusters1[0] = 8;
        EncodeState->numClusters1[1] = 4;
    }
    else
    {
        EncodeState->max_idxMode     = 1;
        EncodeState->modeBits[0]     = 42;  // bits = 2 * (Red 7+ Grn 7+ blu 7)
        EncodeState->modeBits[1]     = 48;  // bits = 2 * (Alpha 8+8+8) = 48
        EncodeState->numClusters0[0] = 4;
        EncodeState->numClusters0[1] = 4;
        EncodeState->numClusters1[0] = 4;
        EncodeState->numClusters1[1] = 4;
    }

    CGV_FLOAT src_color_Block[SOURCE_BLOCK_SIZE * MAX_CHANNELS];
    CGV_FLOAT src_alpha_Block[SOURCE_BLOCK_SIZE * MAX_CHANNELS];

    // Go through each possible rotation and selection of index rotationBits)
    for (CGU_UINT8 rotated_channel = 0; rotated_channel < EncodeState->channels3or4; rotated_channel++)
    {
        // A

        for (CGU_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
        {
            for (CGU_INT p = 0; p < 3; p++)
            {
                src_color_Block[k + p * SOURCE_BLOCK_SIZE] = EncodeState->image_src[k + componentRotations[rotated_channel][p + 1] * SOURCE_BLOCK_SIZE];
                src_alpha_Block[k + p * SOURCE_BLOCK_SIZE] = EncodeState->image_src[k + componentRotations[rotated_channel][0] * SOURCE_BLOCK_SIZE];
            }
        }

        CGV_FLOAT err_quantizer;
        CGV_FLOAT err_bestQuantizer = CMP_FLOAT_MAX;

        for (CGU_INT idxMode = 0; idxMode < EncodeState->max_idxMode; idxMode++)
        {
            // B
            CGV_UINT32 color_index2[2];  // reserved .. Not used!

            err_quantizer =
                GetQuantizeIndex(color_index2, best_candidate.color_index, src_color_Block, SOURCE_BLOCK_SIZE, EncodeState->numClusters0[idxMode], 3);

            err_quantizer +=
                GetQuantizeIndex(color_index2, best_candidate.alpha_index, src_alpha_Block, SOURCE_BLOCK_SIZE, EncodeState->numClusters1[idxMode], 3) / 3.0F;

            // If quality is high then run the full shaking for this config and
            // store the result if it beats the best overall error
            // Otherwise only run the shaking if the error is better than the best
            // quantizer error
            if (err_quantizer <= err_bestQuantizer)
            {
                err_bestQuantizer = err_quantizer;

                // Shake size gives the size of the shake cube
                CGV_FLOAT err_overallError;

                err_overallError = optimize_IndexAndEndPoints(best_candidate.color_index,
                                                              best_candidate.color_qendpoint,
                                                              src_color_Block,
                                                              SOURCE_BLOCK_SIZE,
                                                              EncodeState->numClusters0[idxMode],
                                                              CMP_STATIC_CAST(CGU_UINT8, EncodeState->modeBits[0]),
                                                              3,
                                                              u_BC7Encode);

                // Alpha scalar block
                err_overallError += optimize_IndexAndEndPoints(best_candidate.alpha_index,
                                                               best_candidate.alpha_qendpoint,
                                                               src_alpha_Block,
                                                               SOURCE_BLOCK_SIZE,
                                                               EncodeState->numClusters1[idxMode],
                                                               CMP_STATIC_CAST(CGU_UINT8, EncodeState->modeBits[1]),
                                                               3,
                                                               u_BC7Encode) / 3.0f;

                // If we beat the previous best then encode the block
                if (err_overallError < EncodeState->best_err)
                {
                    best_candidate.idxMode         = idxMode;
                    best_candidate.rotated_channel = rotated_channel;
                    if (blockMode == 4)
                        Encode_mode4(EncodeState->cmp_out, &best_candidate);
                    else
                        Encode_mode5(EncodeState->cmp_out, &best_candidate);
                    EncodeState->best_err = err_overallError;
                }
            }
        }  // B
    }      // A
}

void Compress_mode6(BC7_EncodeState EncodeState[], uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
    CGV_FLOAT err;

    CGV_INT    epo_code_out[8] = {0};
    CGV_UINT8  best_index_out[MAX_SUBSET_SIZE];
    CGV_UINT32 best_packedindex_out[2];

    // CGV_FLOAT        block_endpoints[8];
    // icmp_get_block_endpoints(block_endpoints,  EncodeState->image_src, -1, 4);
    // icmp_GetQuantizedEpoCode(epo_code_out, block_endpoints, 6,4);
    // err = icmp_GetQuantizeIndex(best_packedindex_out, best_index_out, EncodeState->image_src, 4, block_endpoints, 0,4);

    err = GetQuantizeIndex(best_packedindex_out,
                           best_index_out,
                           EncodeState->image_src,
                           16,  // numEntries
                           16,  // clusters
                           4);  // channels3or4

    //*****************************
    // Process the partition shape
    //*****************************
    err = optimize_IndexAndEndPoints(best_index_out,
                                     epo_code_out,
                                     EncodeState->image_src,
                                     16,  //numEntries
                                     16,  // Mi_ = clusters
                                     58,  // bits
                                     4,   // channels3or4
                                     u_BC7Encode);

    //**************************
    // Save the encoded block
    //**************************

    if (err < EncodeState->best_err)
    {
        EncodeState->best_err = err;
        Encode_mode6(best_index_out, epo_code_out, EncodeState->cmp_out);
    }
}

void copy_BC7_Encode_settings(BC7_EncodeState EncodeState[], uniform CMP_GLOBAL BC7_Encode settings[])
{
    EncodeState->best_err      = CMP_FLOAT_MAX;
    EncodeState->validModeMask = settings->validModeMask;
#ifdef USE_ICMP
    EncodeState->part_count = settings->part_count;
    EncodeState->channels   = settings->channels;
#endif
}

//===================================== COMPRESS CODE =============================================
#ifdef USE_ICMP
#include "external/bc7_icmp.h"
#endif

bool notValidBlockForMode(CGU_UINT32 blockMode, CGU_BOOL blockNeedsAlpha, CGU_BOOL blockAlphaZeroOne, uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
    // Do we need to skip alpha processing blocks
    if ((blockNeedsAlpha == FALSE) && (blockMode > 3))
    {
        return TRUE;
    }

    // Optional restriction for colour-only blocks so that they
    // don't use modes that have combined colour+alpha - this
    // avoids the possibility that the encoder might choose an
    // alpha other than 1.0 (due to parity) and cause something to
    // become accidentally slightly transparent (it's possible that
    // when encoding 3-component texture applications will assume that
    // the 4th component can safely be assumed to be 1.0 all the time)
    if ((blockNeedsAlpha == FALSE) && (u_BC7Encode->colourRestrict == TRUE) && ((blockMode == 6) || (blockMode == 7)))
    {  // COMBINED_ALPHA
        return TRUE;
    }

    // Optional restriction for blocks with alpha to avoid issues with
    // punch-through or thresholded alpha encoding
    if ((blockNeedsAlpha == TRUE) && (u_BC7Encode->alphaRestrict == TRUE) && (blockAlphaZeroOne == TRUE) && ((blockMode == 6) || (blockMode == 7)))
    {  // COMBINED_ALPHA
        return TRUE;
    }

    return FALSE;
}

void BC7_CompressBlock(BC7_EncodeState EncodeState[], uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
#ifdef USE_NEW_SINGLE_HEADER_INTERFACES
    CGV_Vec4f image_src[16];
    //int       px = 0;
    for (int i = 0; i < 16; i++)
    {
        image_src[i].x = EncodeState->image_src[i];
        image_src[i].y = EncodeState->image_src[i + 16];
        image_src[i].z = EncodeState->image_src[i + 32];
        image_src[i].w = EncodeState->image_src[i + 48];
    }

    CGU_Vec4ui cmp                = CompressBlockBC7_UNORM(image_src, u_BC7Encode->quality);

    //EncodeState->cmp_isout16Bytes = true;
    //EncodeState->cmp_out[0]       = cmp.x & 0xFF;
    //EncodeState->cmp_out[1]       = (cmp.x >> 8) & 0xFF;
    //EncodeState->cmp_out[2]       = (cmp.x >> 16) & 0xFF;
    //EncodeState->cmp_out[3]       = (cmp.x >> 24) & 0xFF;
    //EncodeState->cmp_out[4]       = cmp.y & 0xFF;
    //EncodeState->cmp_out[5]       = (cmp.y >> 8) & 0xFF;
    //EncodeState->cmp_out[6]       = (cmp.y >> 16) & 0xFF;
    //EncodeState->cmp_out[7]       = (cmp.y >> 24) & 0xFF;
    //EncodeState->cmp_out[8]       = cmp.z & 0xFF;
    //EncodeState->cmp_out[9]       = (cmp.z >> 8) & 0xFF;
    //EncodeState->cmp_out[10]      = (cmp.z >> 16) & 0xFF;
    //EncodeState->cmp_out[11]      = (cmp.z >> 24) & 0xFF;
    //EncodeState->cmp_out[12]      = cmp.w & 0xFF;
    //EncodeState->cmp_out[13]      = (cmp.w >> 8) & 0xFF;
    //EncodeState->cmp_out[14]      = (cmp.w >> 16) & 0xFF;
    //EncodeState->cmp_out[15]      = (cmp.w >> 24) & 0xFF;

    EncodeState->cmp_isout16Bytes = false;
    EncodeState->best_cmp_out[0]    = cmp.x;
    EncodeState->best_cmp_out[1]    = cmp.y;
    EncodeState->best_cmp_out[2]    = cmp.z;
    EncodeState->best_cmp_out[3]    = cmp.w;
    return;
#else
    CGU_BOOL blockNeedsAlpha   = FALSE;
    CGU_BOOL blockAlphaZeroOne = FALSE;

    CGV_FLOAT alpha_err = 0.0f;
    CGV_FLOAT alpha_min = 255.0F;

    for (CGU_INT k = 0; k < SOURCE_BLOCK_SIZE; k++)
    {
        if (EncodeState->image_src[k + COMP_ALPHA * SOURCE_BLOCK_SIZE] < alpha_min)
            alpha_min = EncodeState->image_src[k + COMP_ALPHA * SOURCE_BLOCK_SIZE];

        alpha_err += sq_image(EncodeState->image_src[k + COMP_ALPHA * SOURCE_BLOCK_SIZE] - 255.0F);

        if (blockAlphaZeroOne == FALSE)
        {
            if ((EncodeState->image_src[k + COMP_ALPHA * SOURCE_BLOCK_SIZE] == 255.0F) || (EncodeState->image_src[k + COMP_ALPHA * SOURCE_BLOCK_SIZE] == 0.0F))
            {
                blockAlphaZeroOne = TRUE;
            }
        }
    }

    if (alpha_min != 255.0F)
    {
        blockNeedsAlpha = TRUE;
    }

    EncodeState->best_err         = CMP_FLOAT_MAX;
    EncodeState->opaque_err       = alpha_err;

#ifdef USE_ICMP
    EncodeState->refineIterations = 4;
    EncodeState->fastSkipTreshold = 4;
    EncodeState->channels         = 4;
    EncodeState->part_count       = 64;
    EncodeState->cmp_isout16Bytes = FALSE;
#else
    EncodeState->cmp_isout16Bytes = TRUE;
#endif

    // We change the order in which we visit the block modes to try to maximize the chance
    // that we manage to early out as quickly as possible.
    // This is a significant performance optimization for the lower quality modes where the
    // exit threshold is higher, and also tends to improve quality (as the generally higher quality
    // modes are now enumerated earlier, so the first encoding that passes the threshold will
    // tend to pass by a greater margin than if we used a dumb ordering, and thus overall error will
    // be improved)
    CGU_INT blockModeOrder[NUM_BLOCK_TYPES] = {4, 6, 1, 3, 0, 2, 7, 5};

    // used for debugging and mode tests
    //                              76543210
    // u_BC7Encode->validModeMask  = 0b01000000;

    for (CGU_INT block = 0; block < NUM_BLOCK_TYPES; block++)
    {
        CGU_INT blockMode = blockModeOrder[block];

        if (u_BC7Encode->quality < BC7_qFAST_THRESHOLD)
        {
            if (notValidBlockForMode(blockMode, blockNeedsAlpha, blockAlphaZeroOne, u_BC7Encode))
                continue;
        }

        CGU_INT Mode = 0x0001 << blockMode;
        if (!(u_BC7Encode->validModeMask & Mode))
            continue;

        switch (blockMode)
        {
            // image processing with no alpha
        case 0:
#ifdef USE_ICMP
            icmp_mode02(EncodeState);
#else
            Compress_mode01237(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        case 1:
#ifdef USE_ICMP
            icmp_mode13(EncodeState);
#else
            Compress_mode01237(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        case 2:
#ifdef USE_ICMP
            icmp_mode13(EncodeState);
#else
            Compress_mode01237(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        case 3:
#ifdef USE_ICMP
            icmp_mode13(EncodeState);
#else
            Compress_mode01237(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        // image processing with alpha
        case 4:
#ifdef USE_ICMP
            icmp_mode4(EncodeState);
#else
            Compress_mode45(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        case 5:
#ifdef USE_ICMP
            icmp_mode5(EncodeState);
#else
            Compress_mode45(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        case 6:
#ifdef USE_ICMP
            icmp_mode6(EncodeState);
#else
            Compress_mode6(EncodeState, u_BC7Encode);
#endif
            break;
        case 7:
#ifdef USE_ICMP
            icmp_mode7(EncodeState);
#else
            Compress_mode01237(blockMode, EncodeState, u_BC7Encode);
#endif
            break;
        }

        // Early out if we  found we can compress with error below the quality threshold
        if (EncodeState->best_err <= u_BC7Encode->errorThreshold)
        {
            break;
        }
    }
#endif
}

//====================================== BC7_ENCODECLASS END =============================================

#ifndef ASPM_GPU

INLINE void load_block_interleaved_rgba2(CGV_FLOAT image_src[64], uniform texture_surface* uniform src, CGV_INT block_xx, CGU_INT block_yy)
{
    for (CGU_INT y = 0; y < 4; y++)
        for (CGU_INT x = 0; x < 4; x++)
        {
            CGU_UINT32* uniform src_ptr = (CGV_UINT32*)&src->ptr[(block_yy * 4 + y) * src->stride];
#ifdef USE_VARYING
            CGV_UINT32 rgba               = gather_partid(src_ptr, block_xx * 4 + x);
            image_src[16 * 0 + y * 4 + x] = (CGV_FLOAT)((rgba >> 0) & 255);
            image_src[16 * 1 + y * 4 + x] = (CGV_FLOAT)((rgba >> 8) & 255);
            image_src[16 * 2 + y * 4 + x] = (CGV_FLOAT)((rgba >> 16) & 255);
            image_src[16 * 3 + y * 4 + x] = (CGV_FLOAT)((rgba >> 24) & 255);
#else
            CGV_UINT32 rgba               = src_ptr[block_xx * 4 + x];
            image_src[16 * 0 + y * 4 + x] = (CGU_FLOAT)((rgba >> 0) & 255);
            image_src[16 * 1 + y * 4 + x] = (CGU_FLOAT)((rgba >> 8) & 255);
            image_src[16 * 2 + y * 4 + x] = (CGU_FLOAT)((rgba >> 16) & 255);
            image_src[16 * 3 + y * 4 + x] = (CGU_FLOAT)((rgba >> 24) & 255);
#endif
        }
}

#if defined(CMP_USE_FOREACH_ASPM) || defined(USE_VARYING)
INLINE void scatter_uint2(CGU_UINT32* ptr, CGV_INT idx, CGV_UINT32 value)
{
    ptr[idx] = value;  // (perf warning expected)
}
#endif

INLINE void store_data_uint32(CGU_UINT8 dst[], CGU_INT width, CGV_INT v_xx, CGU_INT yy, CGV_UINT32 data[], CGU_INT data_size)
{
    for (CGU_INT k = 0; k < data_size; k++)
    {
        CGU_UINT32* dst_ptr = (CGV_UINT32*)&dst[(yy)*width * data_size];
#ifdef USE_VARYING
        scatter_uint2(dst_ptr, v_xx * data_size + k, data[k]);
#else
        dst_ptr[v_xx * data_size + k]                                   = data[k];
#endif
    }
}

#ifdef USE_VARYING
INLINE void scatter_uint8(CGU_UINT8* ptr, CGV_UINT32 idx, CGV_UINT8 value)
{
    ptr[idx] = value;  // (perf warning expected)
}
#endif

INLINE void store_data_uint8(CGU_UINT8 u_dstptr[], CGU_INT src_width, CGU_INT block_x, CGU_INT block_y, CGV_UINT8 data[], CGU_INT data_size)
{
    for (CGU_INT k = 0; k < data_size; k++)
    {
#ifdef USE_VARYING
        CGU_UINT8* dst_blockptr = (CGU_UINT8*)&u_dstptr[(block_y * src_width * 4)];
        scatter_uint8(dst_blockptr, k + (block_x * data_size), data[k]);
#else
        u_dstptr[(block_y * src_width * 4) + k + (block_x * data_size)] = data[k];
#endif
    }
}

INLINE void store_data_uint32(CGU_UINT8 dst[], CGV_UINT32 width, CGU_INT v_xx, CGU_INT yy, CGV_UINT8 data[], CGU_INT data_size)
{
    for (CGU_INT k = 0; k < data_size; k++)
    {
#if defined(CMP_USE_FOREACH_ASPM) || defined(USE_VARYING)
        CGU_UINT32* dst_ptr = (CGV_UINT32*)&dst[(yy)*width * data_size];
        scatter_uint2(dst_ptr, v_xx * data_size + k, data[k]);
#else
        dst[((yy)*width * data_size) + v_xx * data_size + k]            = data[k];
#endif
    }
}

void CompressBlockBC7_XY(uniform texture_surface u_srcptr[], CGU_INT block_x, CGU_INT block_y, CGU_UINT8 u_dst[], uniform BC7_Encode u_settings[])
{
    BC7_EncodeState _state;
    varying BC7_EncodeState* uniform state = &_state;

    copy_BC7_Encode_settings(state, u_settings);

    load_block_interleaved_rgba2(state->image_src, u_srcptr, block_x, block_y);

    BC7_CompressBlock(state, u_settings);

    if (state->cmp_isout16Bytes)
        store_data_uint8(u_dst, u_srcptr->width, block_x, block_y, state->cmp_out, 16);
    else
        store_data_uint32(u_dst, u_srcptr->width, block_x, block_y, state->best_cmp_out, 4);
}

CMP_EXPORT void CompressBlockBC7_encode(uniform texture_surface src[], CGU_UINT8 dst[], uniform BC7_Encode settings[])
{
    // bc7_isa(); ASPM_PRINT(("ASPM encode [%d,%d]\n",bc7_isa(),src->width,src->height));

    for (CGU_INT u_yy = 0; u_yy < src->height / 4; u_yy++)
#ifdef CMP_USE_FOREACH_ASPM
        foreach (v_xx = 0 ... src->width / 4)
        {
#else
        for (CGV_INT v_xx = 0; v_xx < src->width / 4; v_xx++)
        {
#endif
            CompressBlockBC7_XY(src, v_xx, u_yy, dst, settings);
        }
}

#endif

#ifndef ASPM_GPU
#ifndef ASPM
//======================= DECOMPRESS =========================================
#ifndef USE_HIGH_PRECISION_INTERPOLATION_BC7
CGU_UINT16 aWeight2[] = {0, 21, 43, 64};
CGU_UINT16 aWeight3[] = {0, 9, 18, 27, 37, 46, 55, 64};
CGU_UINT16 aWeight4[] = {0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};

CGU_UINT8 interpolate(CGU_UINT8 e0, CGU_UINT8 e1, CGU_UINT8 index, CGU_UINT8 indexprecision)
{
    if (indexprecision == 2)
        return (CGU_UINT8)(((64 - aWeight2[index]) * CGU_UINT16(e0) + aWeight2[index] * CGU_UINT16(e1) + 32) >> 6);
    else if (indexprecision == 3)
        return (CGU_UINT8)(((64 - aWeight3[index]) * CGU_UINT16(e0) + aWeight3[index] * CGU_UINT16(e1) + 32) >> 6);
    else  // indexprecision == 4
        return (CGU_UINT8)(((64 - aWeight4[index]) * CGU_UINT16(e0) + aWeight4[index] * CGU_UINT16(e1) + 32) >> 6);
}
#endif

void GetBC7Ramp(CGU_UINT32 endpoint[][MAX_DIMENSION_BIG],
                CGU_FLOAT  ramp[MAX_DIMENSION_BIG][(1 << MAX_INDEX_BITS)],
                CGU_UINT32 clusters[2],
                CGU_UINT32 componentBits[MAX_DIMENSION_BIG])
{
    CGU_UINT32 ep[2][MAX_DIMENSION_BIG];
    CGU_UINT32 i;

    // Expand each endpoint component to 8 bits by shifting the MSB to bit 7
    // and then replicating the high bits to the low bits revealed by
    // the shift
    for (i = 0; i < MAX_DIMENSION_BIG; i++)
    {
        ep[0][i] = 0;
        ep[1][i] = 0;
        if (componentBits[i])
        {
            ep[0][i] = (CGU_UINT32)(endpoint[0][i] << (8 - componentBits[i]));
            ep[1][i] = (CGU_UINT32)(endpoint[1][i] << (8 - componentBits[i]));
            ep[0][i] += (CGU_UINT32)(ep[0][i] >> componentBits[i]);
            ep[1][i] += (CGU_UINT32)(ep[1][i] >> componentBits[i]);

            ep[0][i] = min8(255, max8(0, CMP_STATIC_CAST(CGU_UINT8, ep[0][i])));
            ep[1][i] = min8(255, max8(0, CMP_STATIC_CAST(CGU_UINT8, ep[1][i])));
        }
    }

    // If this block type has no explicit alpha channel
    // then make sure alpha is 1.0 for all points on the ramp
    if (!componentBits[COMP_ALPHA])
    {
        ep[0][COMP_ALPHA] = ep[1][COMP_ALPHA] = 255;
    }

    CGU_UINT32 rampIndex = clusters[0];

    rampIndex = (CGU_UINT32)(log((double)rampIndex) / log(2.0));

    // Generate colours for the RGB ramp
    for (i = 0; i < clusters[0]; i++)
    {
#ifdef USE_HIGH_PRECISION_INTERPOLATION_BC7
        ramp[COMP_RED][i] =
            (CGU_FLOAT)floor((ep[0][COMP_RED] * (1.0 - rampLerpWeightsBC7[rampIndex][i])) + (ep[1][COMP_RED] * rampLerpWeightsBC7[rampIndex][i]) + 0.5);
        ramp[COMP_RED][i] = bc7_minf(255.0, bc7_maxf(0., ramp[COMP_RED][i]));
        ramp[COMP_GREEN][i] =
            (CGU_FLOAT)floor((ep[0][COMP_GREEN] * (1.0 - rampLerpWeightsBC7[rampIndex][i])) + (ep[1][COMP_GREEN] * rampLerpWeightsBC7[rampIndex][i]) + 0.5);
        ramp[COMP_GREEN][i] = bc7_minf(255.0, bc7_maxf(0., ramp[COMP_GREEN][i]));
        ramp[COMP_BLUE][i] =
            (CGU_FLOAT)floor((ep[0][COMP_BLUE] * (1.0 - rampLerpWeightsBC7[rampIndex][i])) + (ep[1][COMP_BLUE] * rampLerpWeightsBC7[rampIndex][i]) + 0.5);
        ramp[COMP_BLUE][i] = bc7_minf(255.0, bc7_maxf(0., ramp[COMP_BLUE][i]));
#else
        ramp[COMP_RED][i]   = interpolate(ep[0][COMP_RED], ep[1][COMP_RED], i, rampIndex);
        ramp[COMP_GREEN][i] = interpolate(ep[0][COMP_GREEN], ep[1][COMP_GREEN], i, rampIndex);
        ramp[COMP_BLUE][i]  = interpolate(ep[0][COMP_BLUE], ep[1][COMP_BLUE], i, rampIndex);
#endif
    }

    rampIndex = clusters[1];
    rampIndex = (CGU_UINT32)(log((CGU_FLOAT)rampIndex) / log(2.0));

    if (!componentBits[COMP_ALPHA])
    {
        for (i = 0; i < clusters[1]; i++)
        {
            ramp[COMP_ALPHA][i] = 255.;
        }
    }
    else
    {
        // Generate alphas
        for (i = 0; i < clusters[1]; i++)
        {
#ifdef USE_HIGH_PRECISION_INTERPOLATION_BC7
            ramp[COMP_ALPHA][i] =
                (CGU_FLOAT)floor((ep[0][COMP_ALPHA] * (1.0 - rampLerpWeightsBC7[rampIndex][i])) + (ep[1][COMP_ALPHA] * rampLerpWeightsBC7[rampIndex][i]) + 0.5);
            ramp[COMP_ALPHA][i] = bc7_minf(255.0, bc7_maxf(0., ramp[COMP_ALPHA][i]));
#else
            ramp[COMP_ALPHA][i] = interpolate(ep[0][COMP_ALPHA], ep[1][COMP_ALPHA], i, rampIndex);
#endif
        }
    }
}

//
// Bit reader - reads one bit from a buffer at the current bit offset
//              and increments the offset
//

CGU_UINT32 ReadBit(const CGU_UINT8 base[], CGU_UINT32& m_bitPosition)
{
    int        byteLocation;
    int        remainder;
    CGU_UINT32 bit = 0;
    byteLocation   = m_bitPosition / 8;
    remainder      = m_bitPosition % 8;

    bit = base[byteLocation];
    bit >>= remainder;
    bit &= 0x1;
    // Increment bit position
    m_bitPosition++;
    return (bit);
}

void DecompressDualIndexBlock(CGU_UINT8       out[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG],
                              const CGU_UINT8 in[COMPRESSED_BLOCK_SIZE],
                              CGU_UINT32      endpoint[2][MAX_DIMENSION_BIG],
                              CGU_UINT32&     m_bitPosition,
                              CGU_UINT32      m_rotation,
                              CGU_UINT32      m_blockMode,
                              CGU_UINT32      m_indexSwap,
                              CGU_UINT32      m_componentBits[MAX_DIMENSION_BIG])
{
    CGU_UINT32 i, j, k;
    CGU_FLOAT  ramp[MAX_DIMENSION_BIG][1 << MAX_INDEX_BITS];
    CGU_UINT32 blockIndices[2][MAX_SUBSET_SIZE];

    CGU_UINT32 clusters[2];
    clusters[0] = 1 << bti[m_blockMode].indexBits[0];
    clusters[1] = 1 << bti[m_blockMode].indexBits[1];
    if (m_indexSwap)
    {
        CGU_UINT32 temp = clusters[0];
        clusters[0]     = clusters[1];
        clusters[1]     = temp;
    }

    GetBC7Ramp(endpoint, ramp, clusters, m_componentBits);

    // Extract the indices
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < MAX_SUBSET_SIZE; j++)
        {
            blockIndices[i][j] = 0;
            // If this is a fixup index then clear the implicit bit
            if (j == 0)
            {
                blockIndices[i][j] &= ~(1 << (bti[m_blockMode].indexBits[i] - 1U));
                for (k = 0; k < static_cast<CGU_UINT32>(bti[m_blockMode].indexBits[i] - 1); k++)
                {
                    blockIndices[i][j] |= (CGU_UINT32)ReadBit(in, m_bitPosition) << k;
                }
            }
            else
            {
                for (k = 0; k < bti[m_blockMode].indexBits[i]; k++)
                {
                    blockIndices[i][j] |= (CGU_UINT32)ReadBit(in, m_bitPosition) << k;
                }
            }
        }
    }

    // Generate block colours
    for (i = 0; i < MAX_SUBSET_SIZE; i++)
    {
        out[i][COMP_ALPHA] = (CGU_UINT8)ramp[COMP_ALPHA][blockIndices[m_indexSwap ^ 1][i]];
        out[i][COMP_RED]   = (CGU_UINT8)ramp[COMP_RED][blockIndices[m_indexSwap][i]];
        out[i][COMP_GREEN] = (CGU_UINT8)ramp[COMP_GREEN][blockIndices[m_indexSwap][i]];
        out[i][COMP_BLUE]  = (CGU_UINT8)ramp[COMP_BLUE][blockIndices[m_indexSwap][i]];
    }

    // Resolve the component rotation
    CGU_INT8 swap;
    for (i = 0; i < MAX_SUBSET_SIZE; i++)
    {
        switch (m_rotation)
        {
        case 0:
            // Do nothing
            break;
        case 1:
            // Swap A and R
            swap               = out[i][COMP_ALPHA];
            out[i][COMP_ALPHA] = out[i][COMP_RED];
            out[i][COMP_RED]   = swap;
            break;
        case 2:
            // Swap A and G
            swap               = out[i][COMP_ALPHA];
            out[i][COMP_ALPHA] = out[i][COMP_GREEN];
            out[i][COMP_GREEN] = swap;
            break;
        case 3:
            // Swap A and B
            swap               = out[i][COMP_ALPHA];
            out[i][COMP_ALPHA] = out[i][COMP_BLUE];
            out[i][COMP_BLUE]  = swap;
            break;
        }
    }
}

void DecompressBC7_internal(CGU_UINT8 out[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG], const CGU_UINT8 in[COMPRESSED_BLOCK_SIZE], const BC7_Encode* u_BC7Encode)
{
    if (u_BC7Encode)
    {
    }
    CGU_UINT32 i, j;
    CGU_UINT32 blockIndices[MAX_SUBSET_SIZE];
    CGU_UINT32 endpoint[MAX_SUBSETS][2][MAX_DIMENSION_BIG];

    CGU_UINT32 m_blockMode;
    CGU_UINT32 m_partition;
    CGU_UINT32 m_rotation;
    CGU_UINT32 m_indexSwap;

    CGU_UINT32 m_bitPosition;
    CGU_UINT32 m_componentBits[MAX_DIMENSION_BIG];

    m_blockMode = 0;
    m_partition = 0;
    m_rotation  = 0;
    m_indexSwap = 0;

    // Position the read pointer at the LSB of the block
    m_bitPosition = 0;

    while (!ReadBit(in, m_bitPosition) && (m_blockMode < 8))
    {
        m_blockMode++;
    }

    if (m_blockMode > 7)
    {
        // Something really bad happened...
        return;
    }

    for (i = 0; i < bti[m_blockMode].rotationBits; i++)
    {
        m_rotation |= ReadBit(in, m_bitPosition) << i;
    }
    for (i = 0; i < bti[m_blockMode].indexModeBits; i++)
    {
        m_indexSwap |= ReadBit(in, m_bitPosition) << i;
    }

    for (i = 0; i < bti[m_blockMode].partitionBits; i++)
    {
        m_partition |= ReadBit(in, m_bitPosition) << i;
    }

    if (bti[m_blockMode].encodingType == NO_ALPHA)
    {
        m_componentBits[COMP_ALPHA] = 0;
        m_componentBits[COMP_RED] = m_componentBits[COMP_GREEN] = m_componentBits[COMP_BLUE] = bti[m_blockMode].vectorBits / 3;
    }
    else if (bti[m_blockMode].encodingType == COMBINED_ALPHA)
    {
        m_componentBits[COMP_ALPHA] = m_componentBits[COMP_RED] = m_componentBits[COMP_GREEN] = m_componentBits[COMP_BLUE] = bti[m_blockMode].vectorBits / 4;
    }
    else if (bti[m_blockMode].encodingType == SEPARATE_ALPHA)
    {
        m_componentBits[COMP_ALPHA] = bti[m_blockMode].scalarBits;
        m_componentBits[COMP_RED] = m_componentBits[COMP_GREEN] = m_componentBits[COMP_BLUE] = bti[m_blockMode].vectorBits / 3;
    }

    CGU_UINT32 subset, ep, component;
    // Endpoints are stored in the following order RRRR GGGG BBBB (AAAA) (PPPP)
    // i.e. components are packed together
    // Loop over components
    for (component = 0; component < MAX_DIMENSION_BIG; component++)
    {
        // loop over subsets
        for (subset = 0; subset < (int)bti[m_blockMode].subsetCount; subset++)
        {
            // Loop over endpoints
            for (ep = 0; ep < 2; ep++)
            {
                endpoint[subset][ep][component] = 0;
                for (j = 0; j < m_componentBits[component]; j++)
                {
                    endpoint[subset][ep][component] |= ReadBit(in, m_bitPosition) << j;
                }
            }
        }
    }

    // Now get any parity bits
    if (bti[m_blockMode].pBitType != NO_PBIT)
    {
        for (subset = 0; subset < (int)bti[m_blockMode].subsetCount; subset++)
        {
            CGU_UINT32 pBit[2];
            if (bti[m_blockMode].pBitType == ONE_PBIT)
            {
                pBit[0] = ReadBit(in, m_bitPosition);
                pBit[1] = pBit[0];
            }
            else if (bti[m_blockMode].pBitType == TWO_PBIT)
            {
                pBit[0] = ReadBit(in, m_bitPosition);
                pBit[1] = ReadBit(in, m_bitPosition);
            }

            for (component = 0; component < MAX_DIMENSION_BIG; component++)
            {
                if (m_componentBits[component])
                {
                    endpoint[subset][0][component] <<= 1;
                    endpoint[subset][1][component] <<= 1;
                    endpoint[subset][0][component] |= pBit[0];
                    endpoint[subset][1][component] |= pBit[1];
                }
            }
        }
    }

    if (bti[m_blockMode].pBitType != NO_PBIT)
    {
        // Now that we've unpacked the parity bits, update the component size information
        // for the ramp generator
        for (j = 0; j < MAX_DIMENSION_BIG; j++)
        {
            if (m_componentBits[j])
            {
                m_componentBits[j] += 1;
            }
        }
    }

    // If this block has two independent sets of indices then put it to that decoder
    if (bti[m_blockMode].encodingType == SEPARATE_ALPHA)
    {
        DecompressDualIndexBlock(out, in, endpoint[0], m_bitPosition, m_rotation, m_blockMode, m_indexSwap, m_componentBits);
        return;
    }

    CGU_UINT32 fixup[MAX_SUBSETS] = {0, 0, 0};
    switch (bti[m_blockMode].subsetCount)
    {
    case 3:
        fixup[1] = BC7_FIXUPINDICES_LOCAL[2][m_partition][1];
        fixup[2] = BC7_FIXUPINDICES_LOCAL[2][m_partition][2];
        break;
    case 2:
        fixup[1] = BC7_FIXUPINDICES_LOCAL[1][m_partition][1];
        break;
    default:
        break;
    }

    //--------------------------------------------------------------------
    // New Code : Possible replacement for BC7_PARTITIONS for CPU code
    //--------------------------------------------------------------------
    // Extract index bits
    // for (i = 0; i < MAX_SUBSET_SIZE; i++)
    // {
    //     CGV_UINT8   p = get_partition_subset(m_partition, bti[m_blockMode].subsetCount - 1, i);
    //     //CGU_UINT32   p = partitionTable[i];
    //     blockIndices[i] = 0;
    //     CGU_UINT32   bitsToRead = bti[m_blockMode].indexBits[0];
    //
    //     // If this is a fixup index then set the implicit bit
    //     if (i == fixup[p])
    //     {
    //         blockIndices[i] &= ~(1 << (bitsToRead - 1));
    //         bitsToRead--;
    //     }
    //
    //     for (j = 0; j < bitsToRead; j++)
    //     {
    //         blockIndices[i] |= ReadBit(in, m_bitPosition) << j;
    //     }
    // }
    CGU_UINT8* partitionTable = (CGU_UINT8*)BC7_PARTITIONS[bti[m_blockMode].subsetCount - 1][m_partition];

    // Extract index bits
    for (i = 0; i < MAX_SUBSET_SIZE; i++)
    {
        CGU_UINT8 p          = partitionTable[i];
        blockIndices[i]      = 0;
        CGU_UINT8 bitsToRead = bti[m_blockMode].indexBits[0];

        // If this is a fixup index then set the implicit bit
        if (i == fixup[p])
        {
            blockIndices[i] &= ~(1 << (bitsToRead - 1));
            bitsToRead--;
        }

        for (j = 0; j < bitsToRead; j++)
        {
            blockIndices[i] |= ReadBit(in, m_bitPosition) << j;
        }
    }

    // Get the ramps
    CGU_UINT32 clusters[2];
    clusters[0] = clusters[1] = 1 << bti[m_blockMode].indexBits[0];

    // Colour Ramps
    CGU_FLOAT c[MAX_SUBSETS][MAX_DIMENSION_BIG][1 << MAX_INDEX_BITS];

    for (i = 0; i < (int)bti[m_blockMode].subsetCount; i++)
    {
        // Unpack the colours
        GetBC7Ramp(endpoint[i], c[i], clusters, m_componentBits);
    }

    //--------------------------------------------------------------------
    // New Code : Possible replacement for BC7_PARTITIONS for CPU code
    //--------------------------------------------------------------------
    // Generate the block colours.
    // for (i = 0; i < MAX_SUBSET_SIZE; i++)
    // {
    //     CGV_UINT8   p = get_partition_subset(m_partition, bti[m_blockMode].subsetCount - 1, i);
    //     out[i][0] = c[p][0][blockIndices[i]];
    //     out[i][1] = c[p][1][blockIndices[i]];
    //     out[i][2] = c[p][2][blockIndices[i]];
    //     out[i][3] = c[p][3][blockIndices[i]];
    // }

    // Generate the block colours.
    for (i = 0; i < MAX_SUBSET_SIZE; i++)
    {
        for (j = 0; j < MAX_DIMENSION_BIG; j++)
        {
            out[i][j] = (CGU_UINT8)c[partitionTable[i]][j][blockIndices[i]];
        }
    }
}

void CompressBlockBC7_Internal(CGU_UINT8  image_src[SOURCE_BLOCK_SIZE][4],
                               CMP_GLOBAL CGV_UINT8 cmp_out[COMPRESSED_BLOCK_SIZE],
                               uniform CMP_GLOBAL BC7_Encode u_BC7Encode[])
{
    BC7_EncodeState _state                 = {0};
    varying BC7_EncodeState* uniform state = &_state;

    copy_BC7_Encode_settings(state, u_BC7Encode);

    CGU_UINT8 offsetR = 0;
    CGU_UINT8 offsetG = 16;
    CGU_UINT8 offsetB = 32;
    CGU_UINT8 offsetA = 48;
    for (CGU_UINT8 i = 0; i < SOURCE_BLOCK_SIZE; i++)
    {
        state->image_src[offsetR++] = (CGV_FLOAT)image_src[i][0];
        state->image_src[offsetG++] = (CGV_FLOAT)image_src[i][1];
        state->image_src[offsetB++] = (CGV_FLOAT)image_src[i][2];
        state->image_src[offsetA++] = (CGV_FLOAT)image_src[i][3];
    }

    BC7_CompressBlock(state, u_BC7Encode);

    if (state->cmp_isout16Bytes)
    {
        for (CGU_UINT8 i = 0; i < COMPRESSED_BLOCK_SIZE; i++)
        {
            cmp_out[i] = state->cmp_out[i];
        }
    }
    else
    {
#ifdef ASPM_GPU
        cmp_memcpy(cmp_out, (CGU_UINT8*)state->best_cmp_out, 16);
#else
        memcpy(cmp_out, state->best_cmp_out, 16);
#endif
    }
}

//======================= CPU USER INTERFACES ====================================

int CMP_CDECL CreateOptionsBC7(void** options)
{
    (*options) = new BC7_Encode;
    if (!options)
        return CGU_CORE_ERR_NEWMEM;
    init_BC7ramps();
    SetDefaultBC7Options((BC7_Encode*)(*options));
    return CGU_CORE_OK;
}

int CMP_CDECL DestroyOptionsBC7(void* options)
{
    if (!options)
        return CGU_CORE_ERR_INVALIDPTR;
    BC7_Encode* BCOptions = reinterpret_cast<BC7_Encode*>(options);
    delete BCOptions;
    return CGU_CORE_OK;
}

int CMP_CDECL SetErrorThresholdBC7(void* options, CGU_FLOAT minThreshold, CGU_FLOAT maxThreshold)
{
    if (!options)
        return CGU_CORE_ERR_INVALIDPTR;
    BC7_Encode* BC7optionsDefault = (BC7_Encode*)options;

    if (minThreshold < 0.0f)
        minThreshold = 0.0f;
    if (maxThreshold < 0.0f)
        maxThreshold = 0.0f;

    BC7optionsDefault->minThreshold = minThreshold;
    BC7optionsDefault->maxThreshold = maxThreshold;
    return CGU_CORE_OK;
}

int CMP_CDECL SetQualityBC7(void* options, CGU_FLOAT fquality)
{
    if (!options)
        return CGU_CORE_ERR_INVALIDPTR;

    BC7_Encode* BC7optionsDefault = (BC7_Encode*)options;
    if (fquality < 0.0f)
        fquality = 0.0f;
    else if (fquality > 1.0f)
        fquality = 1.0f;
    BC7optionsDefault->quality = fquality;

    // Set Error Thresholds
    BC7optionsDefault->errorThreshold = BC7optionsDefault->maxThreshold * (1.0f - fquality);
    if (fquality > BC7_qFAST_THRESHOLD)
        BC7optionsDefault->errorThreshold += BC7optionsDefault->minThreshold;

    return CGU_CORE_OK;
}

int CMP_CDECL SetMaskBC7(void* options, CGU_UINT8 mask)
{
    if (!options)
        return CGU_CORE_ERR_INVALIDPTR;
    BC7_Encode* BC7options    = (BC7_Encode*)options;
    BC7options->validModeMask = mask;
    return CGU_CORE_OK;
}

int CMP_CDECL SetAlphaOptionsBC7(void* options, CGU_BOOL imageNeedsAlpha, CGU_BOOL colourRestrict, CGU_BOOL alphaRestrict)
{
    if (!options)
        return CGU_CORE_ERR_INVALIDPTR;
    BC7_Encode* u_BC7Encode      = (BC7_Encode*)options;
    u_BC7Encode->imageNeedsAlpha = imageNeedsAlpha;
    u_BC7Encode->colourRestrict  = colourRestrict;
    u_BC7Encode->alphaRestrict   = alphaRestrict;
    return CGU_CORE_OK;
}

int CMP_CDECL CompressBlockBC7(const unsigned char* srcBlock, unsigned int srcStrideInBytes, CMP_GLOBAL unsigned char cmpBlock[16], const void* options = NULL)
{
    CMP_Vec4uc inBlock[SOURCE_BLOCK_SIZE];

    //----------------------------------
    // Fill the inBlock with source data
    //----------------------------------
    CGU_INT srcpos = 0;
    CGU_INT dstptr = 0;
    for (CGU_UINT8 row = 0; row < 4; row++)
    {
        srcpos = row * srcStrideInBytes;
        for (CGU_UINT8 col = 0; col < 4; col++)
        {
            inBlock[dstptr].x = CGU_UINT8(srcBlock[srcpos++]);
            inBlock[dstptr].y = CGU_UINT8(srcBlock[srcpos++]);
            inBlock[dstptr].z = CGU_UINT8(srcBlock[srcpos++]);
            inBlock[dstptr].w = CGU_UINT8(srcBlock[srcpos++]);
            dstptr++;
        }
    }

    BC7_Encode* u_BC7Encode      = (BC7_Encode*)options;
    BC7_Encode  BC7EncodeDefault = {0};
    if (u_BC7Encode == NULL)
    {
        u_BC7Encode = &BC7EncodeDefault;
        SetDefaultBC7Options(u_BC7Encode);
        init_BC7ramps();
    }

    BC7_EncodeState EncodeState
#ifndef ASPM
        = { 0 }
#endif
    ;
    EncodeState.best_err      = CMP_FLOAT_MAX;
    EncodeState.validModeMask = u_BC7Encode->validModeMask;
    EncodeState.part_count    = u_BC7Encode->part_count;
    EncodeState.channels      = CMP_STATIC_CAST(CGU_UINT8, u_BC7Encode->channels);

    CGU_UINT8  offsetR   = 0;
    CGU_UINT8  offsetG   = 16;
    CGU_UINT8  offsetB   = 32;
    CGU_UINT8  offsetA   = 48;
    CGU_UINT32 offsetSRC = 0;
    for (CGU_UINT8 i = 0; i < SOURCE_BLOCK_SIZE; i++)
    {
        EncodeState.image_src[offsetR++] = (CGV_FLOAT)inBlock[offsetSRC].x;
        EncodeState.image_src[offsetG++] = (CGV_FLOAT)inBlock[offsetSRC].y;
        EncodeState.image_src[offsetB++] = (CGV_FLOAT)inBlock[offsetSRC].z;
        EncodeState.image_src[offsetA++] = (CGV_FLOAT)inBlock[offsetSRC].w;
        offsetSRC++;
    }

    BC7_CompressBlock(&EncodeState, u_BC7Encode);

    if (EncodeState.cmp_isout16Bytes)
    {
        for (CGU_UINT8 i = 0; i < COMPRESSED_BLOCK_SIZE; i++)
        {
            cmpBlock[i] = EncodeState.cmp_out[i];
        }
    }
    else
    {
        memcpy(cmpBlock, EncodeState.best_cmp_out, 16);
    }

    return CGU_CORE_OK;
}

int CMP_CDECL DecompressBlockBC7(const unsigned char cmpBlock[16], unsigned char srcBlock[64], const void* options = NULL)
{
    BC7_Encode* u_BC7Encode      = (BC7_Encode*)options;
    BC7_Encode  BC7EncodeDefault = {0};  // for q = 0.05
    if (u_BC7Encode == NULL)
    {
        // set for q = 1.0
        u_BC7Encode = &BC7EncodeDefault;
        SetDefaultBC7Options(u_BC7Encode);
        init_BC7ramps();
    }
    DecompressBC7_internal((CGU_UINT8(*)[4])srcBlock, (CGU_UINT8*)cmpBlock, u_BC7Encode);
    return CGU_CORE_OK;
}
#endif
#endif

//============================================== OpenCL USER INTERFACE ====================================================
#ifdef ASPM_OPENCL
CMP_STATIC CMP_KERNEL void CMP_GPUEncoder(uniform CMP_GLOBAL const CGU_Vec4uc ImageSource[],
                                          CMP_GLOBAL CGV_UINT8 ImageDestination[],
                                          uniform CMP_GLOBAL Source_Info SourceInfo[],
                                          uniform CMP_GLOBAL BC7_Encode BC7Encode[])
{
    CGU_INT xID = 0;
    CGU_INT yID = 0;

    xID               = get_global_id(0);  // ToDo: Define a size_t 32 bit and 64 bit based on clGetDeviceInfo
    yID               = get_global_id(1);
    CGU_INT srcWidth  = SourceInfo->m_src_width;
    CGU_INT srcHeight = SourceInfo->m_src_height;
    if (xID >= (srcWidth / BlockX))
        return;
    if (yID >= (srcHeight / BlockY))
        return;

    //ASPM_PRINT(("[ASPM_OCL] %d %d  size %d\n",xID,yID,sizeof(BC7_Encode)));

    CGU_INT         destI    = (xID * COMPRESSED_BLOCK_SIZE) + (yID * (srcWidth / BlockX) * COMPRESSED_BLOCK_SIZE);
    CGU_INT         srcindex = 4 * (yID * srcWidth + xID);
    CGU_INT         blkindex = 0;
    BC7_EncodeState EncodeState;
    cmp_memsetBC7((CGV_UINT8*)&EncodeState, 0, sizeof(EncodeState));
    copy_BC7_Encode_settings(&EncodeState, BC7Encode);

    //Check if it is a complete 4X4 block
    if (((xID + 1) * BlockX <= srcWidth) && ((yID + 1) * BlockY <= srcHeight))
    {
        srcWidth = srcWidth - 4;
        for (CGU_INT j = 0; j < 4; j++)
        {
            for (CGU_INT i = 0; i < 4; i++)
            {
                EncodeState.image_src[blkindex + 0 * SOURCE_BLOCK_SIZE] = ImageSource[srcindex].x;
                EncodeState.image_src[blkindex + 1 * SOURCE_BLOCK_SIZE] = ImageSource[srcindex].y;
                EncodeState.image_src[blkindex + 2 * SOURCE_BLOCK_SIZE] = ImageSource[srcindex].z;
                EncodeState.image_src[blkindex + 3 * SOURCE_BLOCK_SIZE] = ImageSource[srcindex].w;
                blkindex++;
                srcindex++;
            }

            srcindex += srcWidth;
        }

        BC7_CompressBlock(&EncodeState, BC7Encode);

        //printf("CMP %x %x %x %x %x %x %x %x\n",
        //        EncodeState.cmp_out[0],
        //        EncodeState.cmp_out[1],
        //        EncodeState.cmp_out[2],
        //        EncodeState.cmp_out[3],
        //        EncodeState.cmp_out[4],
        //        EncodeState.cmp_out[5],
        //        EncodeState.cmp_out[6],
        //        EncodeState.cmp_out[7]
        //        );

        for (CGU_INT i = 0; i < COMPRESSED_BLOCK_SIZE; i++)
        {
            ImageDestination[destI + i] = EncodeState.cmp_out[i];
        }
    }
    else
    {
        ASPM_PRINT(("[ASPM_GPU] Unable to process, make sure image size is divisible by 4"));
    }
}
#endif
