//=====================================================================
// Copyright (c) 2020    Advanced Micro Devices, Inc. All rights reserved.
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
//=====================================================================

#include "bc2_encode_kernel.h"

//============================================== BC2 INTERFACES =======================================================

void  CompressBlockBC2_Internal(const CMP_Vec4uc srcBlockTemp[16],
                                CMP_GLOBAL CGU_UINT32 compressedBlock[4],
                                CMP_GLOBAL const CMP_BC15Options *BC15options) {

    CGU_Vec2ui  cmpBlock;
    CGU_Vec3f   rgbBlock[16];
    CGU_FLOAT   BlockA[16];

    for (CGU_INT32 i = 0; i < 16; i++) {
        rgbBlock[i].x = (CGU_FLOAT)(srcBlockTemp[i].x & 0xFF)/255.0f;   // R
        rgbBlock[i].y = (CGU_FLOAT)(srcBlockTemp[i].y & 0xFF)/255.0f;   // G
        rgbBlock[i].z = (CGU_FLOAT)(srcBlockTemp[i].z & 0xFF)/255.0f;   // B
        BlockA[i]     = (CGU_FLOAT)(srcBlockTemp[i].w & 0xFF)/255.0f;   // A
    }

    cmpBlock = cmp_compressExplicitAlphaBlock(BlockA);
    compressedBlock[DXTC_OFFSET_ALPHA  ] = cmpBlock.x;
    compressedBlock[DXTC_OFFSET_ALPHA+1] = cmpBlock.y;

    // Need a copy, as CalculateColourWeightings sets variables in the BC15options
    CMP_BC15Options internalOptions = *BC15options;
    internalOptions             = CalculateColourWeightings3f(rgbBlock, internalOptions);
    internalOptions.m_bUseAlpha = false;
    CGU_Vec3f  channelWeights   = {internalOptions.m_fChannelWeights[0],internalOptions.m_fChannelWeights[1],internalOptions.m_fChannelWeights[2]};
    CGU_Vec3f MinColor = {0,0,0}, MaxColor= {0,0,0};

    cmpBlock = CompressBlockBC1_RGBA_Internal(
                   rgbBlock,
                   BlockA,
                   channelWeights,
                   internalOptions.m_nAlphaThreshold,
                   internalOptions.m_nRefinementSteps,
                   internalOptions.m_fquality,
                   FALSE);

    compressedBlock[DXTC_OFFSET_RGB]   = cmpBlock.x;
    compressedBlock[DXTC_OFFSET_RGB+1] = cmpBlock.y;

}

//============================================== USER INTERFACES ========================================================
#ifndef ASPM_GPU

int CMP_CDECL CreateOptionsBC2(void **options) {
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

int CMP_CDECL DestroyOptionsBC2(void *options) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BCOptions = reinterpret_cast <CMP_BC15Options *>(options);
    delete BCOptions;
    return CGU_CORE_OK;
}

int CMP_CDECL SetQualityBC2(void *options,
                            CGU_FLOAT fquality) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault = reinterpret_cast <CMP_BC15Options *>(options);
    if (fquality < 0.0f) fquality = 0.0f;
    else if (fquality > 1.0f) fquality = 1.0f;
    BC15optionsDefault->m_fquality = fquality;
    return CGU_CORE_OK;
}

int CMP_CDECL SetChannelWeightsBC2(void *options,
                                   CGU_FLOAT WeightRed,
                                   CGU_FLOAT WeightGreen,
                                   CGU_FLOAT WeightBlue) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault = (CMP_BC15Options *)options;

    if ((WeightRed < 0.0f) || (WeightRed > 1.0f))       return CGU_CORE_ERR_RANGERED;
    if ((WeightGreen < 0.0f) || (WeightGreen > 1.0f))   return CGU_CORE_ERR_RANGEGREEN;
    if ((WeightBlue < 0.0f) || (WeightBlue > 1.0f))     return CGU_CORE_ERR_RANGEBLUE;

    BC15optionsDefault->m_bUseChannelWeighting = true;
    BC15optionsDefault->m_fChannelWeights[0] = WeightRed;
    BC15optionsDefault->m_fChannelWeights[1] = WeightGreen;
    BC15optionsDefault->m_fChannelWeights[2] = WeightBlue;
    return CGU_CORE_OK;
}

int CMP_CDECL SetSrgbBC2(void* options,
                          CGU_BOOL sRGB) {
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault = (CMP_BC15Options *)options;

    BC15optionsDefault->m_bIsSRGB = sRGB;
    return CGU_CORE_OK;
}

#define EXPLICIT_ALPHA_PIXEL_MASK 0xf
#define EXPLICIT_ALPHA_PIXEL_BPP  4

// Decompresses an explicit alpha block (DXT3)
void DecompressExplicitAlphaBlock(CGU_UINT8 alphaBlock[BLOCK_SIZE_4X4],
                                  const CGU_UINT32 compressedBlock[2]) {
    for (int i = 0; i < 16; i++) {
        int nBlock = i < 8 ? 0 : 1;
        CGU_UINT8 cAlpha = (CGU_UINT8)((compressedBlock[nBlock] >> ((i % 8) * EXPLICIT_ALPHA_PIXEL_BPP)) & EXPLICIT_ALPHA_PIXEL_MASK);
        alphaBlock[i] = (CGU_UINT8)((cAlpha << EXPLICIT_ALPHA_PIXEL_BPP) | cAlpha);
    }
}

void DecompressBC2_Internal(CMP_GLOBAL CGU_UINT8 rgbaBlock[BLOCK_SIZE_4X4X4],
                            const CGU_UINT32 compressedBlock[4],
                            const CMP_BC15Options *BC15options) {
    CGU_UINT8 alphaBlock[BLOCK_SIZE_4X4];

    DecompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);


    CGU_Vec2ui compBlock;
    compBlock.x = compressedBlock[DXTC_OFFSET_RGB];
    compBlock.y = compressedBlock[DXTC_OFFSET_RGB+1];

    cmp_decompressDXTRGBA_Internal(rgbaBlock, compBlock,BC15options->m_mapDecodeRGBA);

    for (CGU_UINT32 i = 0; i < 16; i++)
        ((CMP_GLOBAL CGU_UINT32*)rgbaBlock)[i] = (alphaBlock[i] << RGBA8888_OFFSET_A) | (((CMP_GLOBAL CGU_UINT32*)rgbaBlock)[i] & ~(BYTE_MASK << RGBA8888_OFFSET_A));
}

int CMP_CDECL CompressBlockBC2(const unsigned char *srcBlock,
                               unsigned int srcStrideInBytes,
                               CMP_GLOBAL unsigned char cmpBlock[16],
                               CMP_GLOBAL const void *options = NULL) {

    CMP_Vec4uc inBlock[16];

    //----------------------------------
    // Fill the inBlock with source data
    //----------------------------------
    CGU_INT srcpos = 0;
    CGU_INT dstptr = 0;
    for (CGU_UINT8 row = 0; row < 4; row++) {
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
        BC15options = &BC15optionsDefault;
        SetDefaultBC15Options(BC15options);
    }
    CompressBlockBC2_Internal(inBlock, (CMP_GLOBAL CGU_UINT32 *)cmpBlock, BC15options);
    return CGU_CORE_OK;
}

int CMP_CDECL DecompressBlockBC2(const unsigned char cmpBlock[16],
                                 CMP_GLOBAL unsigned char srcBlock[64],
                                 const void *options = NULL) {
    CMP_BC15Options *BC15options = (CMP_BC15Options *)options;
    CMP_BC15Options BC15optionsDefault;
    if (BC15options == NULL) {
        BC15options = &BC15optionsDefault;
        SetDefaultBC15Options(BC15options);
    }
    DecompressBC2_Internal(srcBlock, (CGU_UINT32 *)cmpBlock,BC15options);

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

#ifdef ASPM_GPU
    xID = get_global_id(0);
    yID = get_global_id(1);
#else
    xID = 0;
    yID = 0;
#endif

    if (xID >= (SourceInfo->m_src_width / BlockX)) return;
    if (yID >= (SourceInfo->m_src_height / BlockX)) return;
    int  srcWidth = SourceInfo->m_src_width;

    CGU_UINT32 destI = (xID*BC2CompBlockSize) + (yID*(srcWidth / BlockX)*BC2CompBlockSize);
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

    CompressBlockBC2_Internal(srcData,(CMP_GLOBAL CGU_UINT32 *)&ImageDestination[destI], BC15options);
}
#endif

