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
// File: BC1Encode.hlsl
//--------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------
#ifndef ASPM_HLSL
#define ASPM_HLSL
#endif

cbuffer cbCS : register( b0 )
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

#include "bcn_common_kernel.h"


// Source Data
Texture2D g_Input : register( t0 ); 
StructuredBuffer<uint4> g_InBuff : register( t1 );

// Compressed Output Data
RWStructuredBuffer<uint4> g_OutBuff : register( u0 );

// Processing multiple blocks at a time
#define MAX_USED_THREAD     16  // pixels in a BC (block compressed) block
#define BLOCK_IN_GROUP      4   // the number of BC blocks a thread group processes = 64 / 16 = 4
#define THREAD_GROUP_SIZE   64  // 4 blocks where a block is (BLOCK_SIZE_X x BLOCK_SIZE_Y)
#define BLOCK_SIZE_Y        4
#define BLOCK_SIZE_X        4

groupshared float4 shared_temp[THREAD_GROUP_SIZE];

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void EncodeBlocks(uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID)
{
    // we process 4 BC blocks per thread group
    uint blockInGroup   = GI / MAX_USED_THREAD;                                         // what BC block this thread is on within this thread group
    uint blockID        = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup; // what global BC block this thread is on
    uint pixelBase      = blockInGroup * MAX_USED_THREAD;                               // the first id of the pixel in this BC block in this thread group
    uint pixelInBlock   = GI - pixelBase;                                               // id of the pixel in this BC block


    uint block_y        = blockID / g_num_block_x;
    uint block_x        = blockID - block_y * g_num_block_x;
    uint base_x         = block_x * BLOCK_SIZE_X;
    uint base_y         = block_y * BLOCK_SIZE_Y;


    // Load up the pixels
    if (pixelInBlock < 16)
    {
           // load pixels (0..1)
           shared_temp[GI] = float4(g_Input.Load( uint3( base_x + pixelInBlock % 4, base_y + pixelInBlock / 4, 0 ) ));
    }

    GroupMemoryBarrierWithGroupSync();

    // Process and save s
    if (pixelInBlock == 0)
    {
        float3 blockRGB[16];
        float  blockA[16];
        for (int i = 0; i < 16; i++ )
        {
                blockRGB[i].x   = shared_temp[pixelBase + i].x;
                blockRGB[i].y   = shared_temp[pixelBase + i].y;
                blockRGB[i].z   = shared_temp[pixelBase + i].z;
                blockA[i]       = shared_temp[pixelBase + i].w;
        }

        g_OutBuff[blockID] = CompressBlockBC3_UNORM(blockRGB,blockA, g_quality,false); 
    }
}
