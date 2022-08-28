//============================================================================== 
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
//===============================================================================
#include "common_def.h"

//============================================== BC1 INTERFACES  =======================================================
#ifndef ASPM_OPENCL
#define USE_NEW_SINGLE_HEADER_INTERFACES // Else use HPC v4.1 codec if using OpenCL

#ifdef USE_NEW_SINGLE_HEADER_INTERFACES
#define USE_CMP
//#define USE_RGBCX
//#define USE_ICBC
//#define USE_BETSY
//#define USE_INT
//#define USE_STB
//#define USE_SQUISH
//#define USE_HUMUS
#endif
#endif

#include "bc1_encode_kernel.h"      // new header for testing common encoders

// Heat Mapping
// This is code that compares quality of two similar or equal codecs with varying quality settings
// A resulting compressed codec data block is colored according to three colors conditions
// The base codec, lowest quality is colored green and the varying quality code is colored red.
// If the quality of the base matches that of the varying codec then the color is set to blue
// Base codecs can be local to CMP_Core or imported using a external set of files, the base codec

#ifndef TEST_HEATMAP
//#define TEST_HEATMAP   // Enable this to run heat map tests on BC1 codec
#endif

#ifdef TEST_HEATMAP
#include "externcodec.h"  // Use external codec for testing
#endif

#ifndef ASPM_HLSL
void  CompressBlockBC1_Internal(const       CMP_Vec4uc      srcBlockTemp[16],
                                CMP_GLOBAL  CGU_UINT32      compressedBlock[2],
                                CMP_GLOBAL  CMP_BC15Options *BC15options) 
{

#ifdef USE_NEW_SINGLE_HEADER_INTERFACES
    CGU_Vec2ui cmpBlock2 = {0,0};
    CGU_Vec4f  image_src[16];
    //int        px = 0;
    for (int i = 0; i < 16; i++)
    {
        image_src[i].x = srcBlockTemp[i].x / 255.0f;
        image_src[i].y = srcBlockTemp[i].y / 255.0f;
        image_src[i].z = srcBlockTemp[i].z / 255.0f;
        image_src[i].w = srcBlockTemp[i].w / 255.0f;
    }

    cmpBlock2          = CompressBlockBC1_UNORM2(image_src, *BC15options);
    compressedBlock[0] = cmpBlock2.x;
    compressedBlock[1] = cmpBlock2.y;
#else
    CGU_UINT8    srcindex = 0;
    CGU_FLOAT    BlockA[16];
    CGU_Vec3f    rgbBlockUV[16];
    for ( CGU_INT32 j = 0; j < 4; j++) {
        for ( CGU_INT32 i = 0; i < 4; i++) {
            rgbBlockUV[srcindex].x = (CGU_FLOAT)(srcBlockTemp[srcindex].x & 0xFF)/ 255.0f;  // R
            rgbBlockUV[srcindex].y = (CGU_FLOAT)(srcBlockTemp[srcindex].y & 0xFF)/ 255.0f;  // G
            rgbBlockUV[srcindex].z = (CGU_FLOAT)(srcBlockTemp[srcindex].z & 0xFF)/ 255.0f;  // B
            srcindex++;
        }
    }

    CMP_BC15Options internalOptions = *BC15options;
    internalOptions = CalculateColourWeightings3f(rgbBlockUV,internalOptions);
    CGU_Vec3f  channelWeights     = {internalOptions.m_fChannelWeights[0],internalOptions.m_fChannelWeights[1],internalOptions.m_fChannelWeights[2]};
    CGU_BOOL   isSRGB             = internalOptions.m_bIsSRGB; // feature not supported in this section of code until v4.1
    CGU_Vec2ui  cmpBlock    = 0;

//#define CMP_PRINTRESULTS
#ifdef TEST_HEATMAP

#ifdef CMP_PRINTRESULTS
    static int q1= 0,q2= 0,same = 0;
    static int testnum = 0;
    printf("%4d ",testnum);
#endif
    {

        // Heatmap test: See BCn_Common_Kernel for details
        CGU_Vec2ui red   = {0xf800f800,0};
        CGU_Vec2ui green = {0x07e007e0,0};
        CGU_Vec2ui blue  = {0x001f001f,0};

        CGU_Vec2ui comp1;
        CGU_Vec2ui comp2;
        float err ;

        comp1 =  (BC15options->m_fquality < 0.3)?CompressBC1Block_SRGB(rgbBlockUV):CompressBC1Block(rgbBlockUV);
        comp2 =  CompressBlockBC1_UNORM(rgbBlockUV, BC15options->m_fquality,BC15options->m_fquality < 0.3?true:false);

        if ((comp1.x == comp2.x)&&(comp1.y == comp2.y)) err = 0.0f;
        else {
            float err1 = CMP_RGBBlockError(rgbBlockUV,comp1,(BC15options->m_fquality < 0.3)?true:false);
            float err2 = CMP_RGBBlockError(rgbBlockUV,comp2,(BC15options->m_fquality < 0.3)?true:false);
            err = err1-err2;
        }

        if (err > 0.0f) {
            cmpBlock = red;
        } else if (err < 0.0f) {
            cmpBlock = green;
        } else {
            cmpBlock = blue;
        }
    }
#ifdef CMP_PRINTRESULTS
    printf("Q1 [%4X:%4X]  %.3f, ",cmpBlockQ1.x,cmpBlockQ1.y,err1);
    printf("Q2 [%4X:%4X]  %.3f, ",cmpBlock.x,cmpBlock.y,err2);
    testnum++;
#endif
#else

    // printf("q = %f\n",internalOptions.m_fquality);
    cmpBlock = CompressBlockBC1_RGBA_Internal(
                   rgbBlockUV,
                   BlockA,
                   channelWeights,
                   0, //internalOptions.m_nAlphaThreshold, bug to investigate in debug is ok release has issue!
                   1,
                   internalOptions.m_fquality,
                   isSRGB
               );
#endif
    compressedBlock[0] = cmpBlock.x;
    compressedBlock[1] = cmpBlock.y;

    union {
        unsigned char buf[8];
        uint32        blocks[2];
    } cmp;

    cmp.blocks[0] = compressedBlock[0];
    cmp.blocks[1] = compressedBlock[1];

//   printf("[%3d,%3d,%3d,%3d:%3d,%3d,%3d,%3d]\n",
//       cmp.buf[0], cmp.buf[1], cmp.buf[2], cmp.buf[3],
//       cmp.buf[4], cmp.buf[5], cmp.buf[6], cmp.buf[7]);
#endif
}
#endif // ASPM_HLSL

//============================================== CPU INTERFACES  ========================================================
#ifndef ASPM_GPU
int CMP_CDECL CreateOptionsBC1(void **options) {
    CMP_BC15Options *BC15optionsDefault = new CMP_BC15Options;
    if (BC15optionsDefault) {
        SetDefaultBC15Options(BC15optionsDefault);
        (*options) = BC15optionsDefault;
    } else {
        (*options) = NULL;
        return CGU_CORE_ERR_NEWMEM;
    }
    return CGU_CORE_OK;
}

int CMP_CDECL DestroyOptionsBC1(void *options) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BCOptions = reinterpret_cast <CMP_BC15Options *>(options);
    delete BCOptions;
    return CGU_CORE_OK;
}

int CMP_CDECL SetQualityBC1(void *options, CGU_FLOAT fquality) {
    if (!options) return CGU_CORE_ERR_NEWMEM;
    CMP_BC15Options *BC15optionsDefault =  reinterpret_cast <CMP_BC15Options *>(options);
    if (fquality < 0.0f) fquality = 0.0f;
    else if (fquality > 1.0f) fquality = 1.0f;
    BC15optionsDefault->m_fquality = fquality;
    return CGU_CORE_OK;
}

int CMP_CDECL SetRefineStepsBC1(void *options, CGU_UINT32 steps) {
    if (!options) return CGU_CORE_ERR_NEWMEM;
    CMP_BC15Options *BC15optionsDefault =  reinterpret_cast <CMP_BC15Options *>(options);
    if (steps < 0) steps = 1;
    else if (steps > 1) steps = 1;
    BC15optionsDefault->m_nRefinementSteps = steps;
    return CGU_CORE_OK;
}


int CMP_CDECL SetAlphaThresholdBC1(void *options, CGU_UINT8 alphaThreshold) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault =  reinterpret_cast <CMP_BC15Options *>(options);
    BC15optionsDefault->m_nAlphaThreshold = alphaThreshold;
    return CGU_CORE_OK;
}

int CMP_CDECL SetDecodeChannelMapping(void *options, CGU_BOOL mapRGBA) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault =  reinterpret_cast <CMP_BC15Options *>(options);
    BC15optionsDefault->m_mapDecodeRGBA = mapRGBA;
    return CGU_CORE_OK;
}

int CMP_CDECL SetChannelWeightsBC1(void *options,
                                   CGU_FLOAT WeightRed,
                                   CGU_FLOAT WeightGreen,
                                   CGU_FLOAT WeightBlue) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault = (CMP_BC15Options *)options;

    if ((WeightRed < 0.0f)   || (WeightRed > 1.0f))      return CGU_CORE_ERR_RANGERED;
    if ((WeightGreen < 0.0f) || (WeightGreen > 1.0f))    return CGU_CORE_ERR_RANGEGREEN;
    if ((WeightBlue < 0.0f)  || (WeightBlue > 1.0f))     return CGU_CORE_ERR_RANGEBLUE;

    BC15optionsDefault->m_bUseChannelWeighting = true;
    BC15optionsDefault->m_fChannelWeights[0] = WeightRed;
    BC15optionsDefault->m_fChannelWeights[1] = WeightGreen;
    BC15optionsDefault->m_fChannelWeights[2] = WeightBlue;
    return CGU_CORE_OK;
}

int CMP_CDECL SetGammaBC1(void *options, CGU_BOOL sRGB) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault = (CMP_BC15Options *)options;

    BC15optionsDefault->m_bIsSRGB = sRGB;
    return CGU_CORE_OK;
}

int CMP_CDECL CompressBlockBC1(const unsigned char *srcBlock,
                               unsigned int srcStrideInBytes,
                               CMP_GLOBAL unsigned char cmpBlock[8],
                               const void *options = NULL) {
    CMP_Vec4uc inBlock[16];

    //----------------------------------
    // Fill the inBlock with source data
    //----------------------------------
    CGU_INT srcpos = 0;
    CGU_INT dstptr = 0;
    for (CGU_UINT8 row=0; row < 4; row++) {
        srcpos = row * srcStrideInBytes;
        for (CGU_UINT8 col = 0; col < 4; col++) {
            inBlock[dstptr].x = CGU_UINT8(srcBlock[srcpos++]);
            inBlock[dstptr].y = CGU_UINT8(srcBlock[srcpos++]);
            inBlock[dstptr].z = CGU_UINT8(srcBlock[srcpos++]);
            inBlock[dstptr].w = CGU_UINT8(srcBlock[srcpos++]);
            dstptr++;
        }
    }

    CMP_BC15Options *BC15options = (CMP_BC15Options *)options;
    CMP_BC15Options BC15optionsDefault;
    if (BC15options == NULL) {
        BC15options     = &BC15optionsDefault;
        SetDefaultBC15Options(BC15options);
    }

    CompressBlockBC1_Internal(inBlock, (CMP_GLOBAL  CGU_UINT32 *)cmpBlock, BC15options);
    return CGU_CORE_OK;
}

int CMP_CDECL DecompressBlockBC1(const unsigned char cmpBlock[8],
                                 CMP_GLOBAL unsigned char srcBlock[64],
                                 const void *options = NULL) {
    CMP_BC15Options *BC15options = (CMP_BC15Options *)options;
    CMP_BC15Options BC15optionsDefault;
    if (BC15options == NULL) {
        BC15options     = &BC15optionsDefault;
        SetDefaultBC15Options(BC15options);
    }

    CGU_Vec2ui compBlock;

    compBlock.x = (CGU_UINT32)cmpBlock[3] << 24 |
                  (CGU_UINT32)cmpBlock[2] << 16 |
                  (CGU_UINT32)cmpBlock[1] << 8  |
                  (CGU_UINT32)cmpBlock[0];

    compBlock.y = (CGU_UINT32)cmpBlock[7] << 24 |
                  (CGU_UINT32)cmpBlock[6] << 16 |
                  (CGU_UINT32)cmpBlock[5] << 8  |
                  (CGU_UINT32)cmpBlock[4];

    cmp_decompressDXTRGBA_Internal(srcBlock, compBlock, BC15options->m_mapDecodeRGBA);

    return CGU_CORE_OK;
}
#endif

//============================================== OpenCL USER INTERFACE ========================================================
#ifdef ASPM_OPENCL
CMP_STATIC CMP_KERNEL void CMP_GPUEncoder(
    CMP_GLOBAL  const CMP_Vec4uc*   ImageSource,
    CMP_GLOBAL  CGU_UINT8*          ImageDestination,
    CMP_GLOBAL  Source_Info*        SourceInfo,
    CMP_GLOBAL  CMP_BC15Options*    BC15options
) {
    CGU_UINT32 xID;
    CGU_UINT32 yID;

    //printf("SourceInfo: (H:%d,W:%d) Quality %1.2f \n", SourceInfo->m_src_height, SourceInfo->m_src_width, SourceInfo->m_fquality);
    xID = get_global_id(0);
    yID = get_global_id(1);


    if (xID >= (SourceInfo->m_src_width / BlockX)) return;
    if (yID >= (SourceInfo->m_src_height / BlockX)) return;
    int  srcWidth = SourceInfo->m_src_width;

    CGU_UINT32 destI = (xID*BC1CompBlockSize) + (yID*(srcWidth / BlockX)*BC1CompBlockSize);
    int srcindex = 4 * (yID * srcWidth + xID);
    int blkindex = 0;
    CMP_Vec4uc srcData[16];
    srcWidth = srcWidth - 4;

    for ( CGU_INT32 j = 0; j < 4; j++) {
        for ( CGU_INT32 i = 0; i < 4; i++) {
            srcData[blkindex++] = ImageSource[srcindex++];
        }
        srcindex += srcWidth;
    }
    CompressBlockBC1_Internal(srcData, (CMP_GLOBAL  CGU_UINT32 *)&ImageDestination[destI], BC15options);
}
#endif
