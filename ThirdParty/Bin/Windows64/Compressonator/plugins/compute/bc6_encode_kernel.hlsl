// 60354e86-----------------------------------------------------------------------------
//==============================================================================
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
//===============================================================================
#define ASPM_HLSL  // This is required
#define ASPM_GPU   // This is required

#define USE_MSC   // Use MSC Codec
//#define USE_BETSY  // Use Betsy Codec
//#define USE_CMP     // Use Compressonator Codec


#define CHAR_LENGTH 8
#define NCHANNELS 3
#define MAX_UINT 0xFFFFFFFF
#define MIN_UINT 0
#define BLOCK_SIZE_Y 4
#define BLOCK_SIZE_X 4
#define BLOCK_SIZE (BLOCK_SIZE_Y * BLOCK_SIZE_X)
#define THREAD_GROUP_SIZE 64  // 4 blocks where a block is (BLOCK_SIZE_X x BLOCK_SIZE_Y)

cbuffer cbCS : register(b0)
{
    uint  g_tex_width;
    uint  g_num_block_x;
    uint  g_format;
    uint  g_mode_id;
    uint  g_start_block_id;
    uint  g_num_total_blocks;
    float g_alpha_weight;
    float g_quality;
};

Texture2D<float4>       g_Input : register(t0);
StructuredBuffer<uint4> g_InBuff : register(t1);

RWStructuredBuffer<uint4> g_OutBuff : register(u0);

struct SharedData
{
    float3 pixel;
    int3   pixel_ph;
    float3 pixel_hr;
    float  pixel_lum;
    float  error;
    uint   best_mode;
    uint   best_partition;
    int3   endPoint_low;
    int3   endPoint_high;
    float  endPoint_lum_low;
    float  endPoint_lum_high;
};

#ifdef USE_MSC
groupshared SharedData shared_temp[THREAD_GROUP_SIZE];
#else
groupshared float3 shared_temp[THREAD_GROUP_SIZE];
#endif

#include "bc6_common_encoder.h"

#ifndef USE_MSC
[numthreads(THREAD_GROUP_SIZE, 1, 1)] void EncodeBlocks(CGU_UINT32 GI
                                                        : SV_GroupIndex, CGU_Vec3ui groupID
                                                        : SV_GroupID) {
    // we process 4 BC blocks per thread group
    const CGU_UINT32 MAX_USED_THREAD = 32;
    CGU_UINT32       BLOCK_IN_GROUP  = THREAD_GROUP_SIZE / MAX_USED_THREAD;

    CGU_UINT32 blockInGroup = GI / MAX_USED_THREAD;                                          // what BC block this thread is on within this thread group
    CGU_UINT32 blockID      = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;  // what global BC block this thread is on
    CGU_UINT32 pixelBase    = blockInGroup * MAX_USED_THREAD;                                // the first id of the pixel in this BC block in this thread group
    CGU_UINT32 pixelInBlock = GI - pixelBase;                                                // id of the pixel in this BC block

    CGU_UINT32 block_y = blockID / g_num_block_x;
    CGU_UINT32 block_x = blockID - block_y * g_num_block_x;
    CGU_UINT32 base_x  = block_x * BLOCK_SIZE_X;
    CGU_UINT32 base_y  = block_y * BLOCK_SIZE_Y;

    // Load up the pixels
    if (pixelInBlock < 16)
    {
        shared_temp[GI] = g_Input.Load(CGU_Vec3ui(base_x + pixelInBlock % 4, base_y + pixelInBlock / 4, 0)).rgb;
    }

    GroupMemoryBarrierWithGroupSync();

    // Process and save s
    if (pixelInBlock == 0)
    {
        CGU_Vec3f image_src[16];
        for (CGU_INT i = 0; i < 16; i++)
        {
            image_src[i].x = shared_temp[pixelBase + i].x;
            image_src[i].y = shared_temp[pixelBase + i].y;
            image_src[i].z = shared_temp[pixelBase + i].z;
        }
        g_OutBuff[blockID] = CompressBlockBC6H_UNORM(image_src, g_quality);
    }
}
#endif
