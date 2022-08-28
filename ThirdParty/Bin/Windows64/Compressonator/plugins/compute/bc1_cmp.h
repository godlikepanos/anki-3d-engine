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
// File: BC1_Encode_kernel.hlsl
//--------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------
#include "common_def.h"
#include "bcn_common_kernel.h"
#include "bcn_common_api.h"

//-----------------------------------------------------------------------
// When build is for CPU, we have some missing API calls common to GPU
// Use CPU CMP_Core replacements
//-----------------------------------------------------------------------
#if defined(ASPM_GPU) || defined(ASPM_HLSL) || defined(ASPM_OPENCL)
#define ALIGN_16
#else
#include INC_cmp_math_func
#if defined(WIN32) || defined(_WIN64)
#define ALIGN_16 __declspec(align(16))
#else  // !WIN32 && !_WIN64
#define ALIGN_16
#endif  // !WIN32 && !_WIN64
#endif

#define USE_REFINE3D
#define USE_REFINE
 
#ifndef MAX_ERROR
#define MAX_ERROR 128000.f
#endif

#define NUM_CHANNELS 4
#define NUM_ENDPOINTS 2

#ifndef CMP_QUALITY0
#define CMP_QUALITY0 0.25f
#endif

#ifndef CMP_QUALITY1
#define CMP_QUALITY1 0.50f
#endif

#ifndef CMP_QUALITY2
#define CMP_QUALITY2 0.75f
#endif

#define EPS (2.f / 255.f) * (2.f / 255.f)
#define EPS2 3.f * (2.f / 255.f) * (2.f / 255.f)



static CGU_FLOAT cgu_getRampErr(CGU_FLOAT  Prj[BLOCK_SIZE_4X4],
                                CGU_FLOAT  PrjErr[BLOCK_SIZE_4X4],
                                CGU_FLOAT  PreMRep[BLOCK_SIZE_4X4],
                                CGU_FLOAT  StepErr,
                                CGU_FLOAT  lowPosStep,
                                CGU_FLOAT  highPosStep,
                                CGU_UINT32 dwUniqueColors)
{
    CGU_FLOAT error  = 0;
    CGU_FLOAT step   = (highPosStep - lowPosStep) / 3;  // using (dwNumChannels=4 - 1);
    CGU_FLOAT step_h = step * (CGU_FLOAT)0.5;
    CGU_FLOAT rstep  = (CGU_FLOAT)1.0f / step;

    for (CGU_UINT32 i = 0; i < dwUniqueColors; i++)
    {
        CGU_FLOAT v;
        // Work out which value in the block this select
        CGU_FLOAT del;

        if ((del = Prj[i] - lowPosStep) <= 0)
            v = lowPosStep;
        else if (Prj[i] - highPosStep >= 0)
            v = highPosStep;
        else
            v = floor((del + step_h) * rstep) * step + lowPosStep;

        // And accumulate the error
        CGU_FLOAT d = (Prj[i] - v);
        d *= d;
        CGU_FLOAT err = PreMRep[i] * d + PrjErr[i];
        error += err;
        if (StepErr < error)
        {
            error = StepErr;
            break;
        }
    }
    return error;
}


CMP_STATIC CMP_EndPoints cgu_CompressRGBBlockX( CMP_IN CGU_Vec3f   BlkInBGRf_UV[BLOCK_SIZE_4X4],
                                                CMP_IN CGU_FLOAT   Rpt[BLOCK_SIZE_4X4],
                                                CMP_IN CGU_UINT32  dwUniqueColors,
                                                CMP_IN CGU_Vec3f   channelWeightsBGR,
                                                CMP_IN CGU_BOOL    b3DRefinement
)
{
    CMP_UNUSED(channelWeightsBGR);
    CMP_UNUSED(b3DRefinement);
    ALIGN_16 CGU_FLOAT Prj0[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT Prj[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT PrjErr[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT RmpIndxs[BLOCK_SIZE_4X4];

    CGU_Vec3f LineDirG;
    CGU_Vec3f LineDir;
    CGU_FLOAT LineDir0[NUM_CHANNELS];
    CGU_Vec3f BlkUV[BLOCK_SIZE_4X4];
    CGU_Vec3f BlkSh[BLOCK_SIZE_4X4];
    CGU_Vec3f Mdl;

    CGU_Vec3f  rsltC0;
    CGU_Vec3f  rsltC1;
    CGU_Vec3f  PosG0 = {0.0f, 0.0f, 0.0f};
    CGU_Vec3f  PosG1 = {0.0f, 0.0f, 0.0f};
    CGU_UINT32 i;

    for (i = 0; i < dwUniqueColors; i++)
    {
        BlkUV[i] = BlkInBGRf_UV[i];
    }

    // if not more then 2 different colors, we've done
    if (dwUniqueColors <= 2)
    {
        rsltC0 = BlkInBGRf_UV[0] * 255.0f;
        rsltC1 = BlkInBGRf_UV[dwUniqueColors - 1] * 255.0f;
    }
    else
    {
        //    This is our first attempt to find an axis we will go along.
        //    The cumulation is done to find a line minimizing the MSE from the
        //    input 3D points.

        //    While trying to find the axis we found that the diameter of the input
        //    set is quite small. Do not bother.

        // FindAxisIsSmall(BlkSh, LineDir0, Mdl, Blk, Rpt,dwUniqueColors);
        {
            CGU_UINT32 ii;
            CGU_UINT32 jj;
            CGU_UINT32 kk;

            // These vars cannot be Vec3 as index to them are varying
            CGU_FLOAT Crrl[NUM_CHANNELS];
            CGU_FLOAT RGB2[NUM_CHANNELS];

            LineDir0[0] = LineDir0[1] = LineDir0[2] = RGB2[0] = RGB2[1] = RGB2[2] = Crrl[0] = Crrl[1] = Crrl[2] = Mdl.x = Mdl.y = Mdl.z = 0.f;

            // sum position of all points
            CGU_FLOAT fNumPoints = 0.0f;
            for (ii = 0; ii < dwUniqueColors; ii++)
            {
                Mdl.x += BlkUV[ii].x * Rpt[ii];
                Mdl.y += BlkUV[ii].y * Rpt[ii];
                Mdl.z += BlkUV[ii].z * Rpt[ii];
                fNumPoints += Rpt[ii];
            }

            // and then average to calculate center coordinate of block
            Mdl /= fNumPoints;

            for (ii = 0; ii < dwUniqueColors; ii++)
            {
                // calculate output block as offsets around block center
                BlkSh[ii] = BlkUV[ii] - Mdl;

                // compute correlation matrix
                // RGB2 = sum of ((distance from point from center) squared)
                RGB2[0] += BlkSh[ii].x * BlkSh[ii].x * Rpt[ii];
                RGB2[1] += BlkSh[ii].y * BlkSh[ii].y * Rpt[ii];
                RGB2[2] += BlkSh[ii].z * BlkSh[ii].z * Rpt[ii];

                Crrl[0] += BlkSh[ii].x * BlkSh[ii].y * Rpt[ii];
                Crrl[1] += BlkSh[ii].y * BlkSh[ii].z * Rpt[ii];
                Crrl[2] += BlkSh[ii].z * BlkSh[ii].x * Rpt[ii];
            }

            // if set's diameter is small
            CGU_UINT32 i0 = 0, i1 = 1;
            CGU_FLOAT  mxRGB2 = 0.0f;

            CGU_FLOAT fEPS = fNumPoints * EPS;
            for (kk = 0, jj = 0; jj < 3; jj++)
            {
                if (RGB2[jj] >= fEPS)
                    kk++;
                else
                    RGB2[jj] = 0.0f;

                if (mxRGB2 < RGB2[jj])
                {
                    mxRGB2 = RGB2[jj];
                    i0     = jj;
                }
            }

            CGU_FLOAT fEPS2 = fNumPoints * EPS2;
            CGU_BOOL  AxisIsSmall;

            AxisIsSmall = (RGB2[0] < fEPS2);
            AxisIsSmall = AxisIsSmall && (RGB2[1] < fEPS2);
            AxisIsSmall = AxisIsSmall && (RGB2[2] < fEPS2);

            // all are very small to avoid division on the small determinant
            if (AxisIsSmall)
            {
                rsltC0 = BlkInBGRf_UV[0] * 255.0f;
                rsltC1 = BlkInBGRf_UV[dwUniqueColors - 1] * 255.0f;
            }
            else
            {
                // !AxisIsSmall
                if (kk == 1)  // really only 1 dimension
                    LineDir0[i0] = 1.;
                else if (kk == 2)
                {  // really only 2 dimensions
                    i1            = (RGB2[(i0 + 1) % 3] > 0.f) ? (i0 + 1) % 3 : (i0 + 2) % 3;
                    CGU_FLOAT Crl = (i1 == (i0 + 1) % 3) ? Crrl[i0] : Crrl[(i0 + 2) % 3];
                    LineDir0[i1]  = Crl / RGB2[i0];
                    LineDir0[i0]  = 1.;
                }
                else
                {
                    CGU_FLOAT maxDet = 100000.f;
                    CGU_FLOAT Cs[3];
                    // select max det for precision
                    for (jj = 0; jj < 3; jj++)
                    {
                        // 3 = nDimensions
                        CGU_FLOAT Det = RGB2[jj] * RGB2[(jj + 1) % 3] - Crrl[jj] * Crrl[jj];
                        Cs[jj]        = cmp_fabs(Crrl[jj] / sqrt(RGB2[jj] * RGB2[(jj + 1) % 3]));
                        if (maxDet < Det)
                        {
                            maxDet = Det;
                            i0     = jj;
                        }
                    }

                    // inverse correl matrix
                    //  --      --       --      --
                    //  |  A   B |       |  C  -B |
                    //  |  B   C |  =>   | -B   A |
                    //  --      --       --     --
                    CGU_FLOAT mtrx1[2][2];
                    CGU_FLOAT vc1[2];
                    CGU_FLOAT vc[2];
                    vc1[0] = Crrl[(i0 + 2) % 3];
                    vc1[1] = Crrl[(i0 + 1) % 3];
                    // C
                    mtrx1[0][0] = RGB2[(i0 + 1) % 3];
                    // A
                    mtrx1[1][1] = RGB2[i0];
                    // -B
                    mtrx1[1][0] = mtrx1[0][1] = -Crrl[i0];
                    // find a solution
                    vc[0] = mtrx1[0][0] * vc1[0] + mtrx1[0][1] * vc1[1];
                    vc[1] = mtrx1[1][0] * vc1[0] + mtrx1[1][1] * vc1[1];
                    // normalize
                    vc[0] /= maxDet;
                    vc[1] /= maxDet;
                    // find a line direction vector
                    LineDir0[i0]           = 1.;
                    LineDir0[(i0 + 1) % 3] = 1.;
                    LineDir0[(i0 + 2) % 3] = vc[0] + vc[1];
                }

                // normalize direction vector
                CGU_FLOAT Len = LineDir0[0] * LineDir0[0] + LineDir0[1] * LineDir0[1] + LineDir0[2] * LineDir0[2];
                Len           = sqrt(Len);

                LineDir0[0] = (Len > 0.f) ? LineDir0[0] / Len : 0.0f;
                LineDir0[1] = (Len > 0.f) ? LineDir0[1] / Len : 0.0f;
                LineDir0[2] = (Len > 0.f) ? LineDir0[2] / Len : 0.0f;
            }
        }  // FindAxisIsSmall

        // GCC is being an awful being when it comes to goto-jumps.
        // So please bear with this.
        CGU_FLOAT ErrG = 10000000.f;
        CGU_FLOAT PrjBnd0;
        CGU_FLOAT PrjBnd1;
        ALIGN_16 CGU_FLOAT PreMRep[BLOCK_SIZE_4X4];

        LineDir.x = LineDir0[0];
        LineDir.y = LineDir0[1];
        LineDir.z = LineDir0[2];

        //    Here is the main loop.
        //    1. Project input set on the axis in consideration.
        //    2. Run 1 dimensional search (see scalar case) to find an (sub) optimal pair of end points.
        //    3. Compute the vector of indexes (or clusters) for the current approximate ramp.
        //    4. Present our color channels as 3 16DIM vectors.
        //    5. Find closest approximation of each of 16DIM color vector with the projection of the 16DIM index vector.
        //    6. Plug the projections as a new directional vector for the axis.
        //    7. Goto 1.
        //    D - is 16 dim "index" vector (or 16 DIM vector of indexes - {0, 1/3,2/3, 0, ...,}, but shifted and normalized).
        //    Ci - is a 16 dim vector of color i. for each Ci find a scalar Ai such that (Ai * D - Ci) (Ai * D - Ci) -> min ,
        //         i.e distance between vector AiD and C is min. You can think of D as a unit interval(vector) "clusterizer", and Ai is a scale
        //         you need to apply to the clusterizer to approximate the Ci vector instead of the unit vector.
        //    Solution is
        //    Ai = (D . Ci) / (D . D); . - is a dot product.
        //    in 3 dim space Ai(s) represent a line direction, along which
        //    we again try to find (sub)optimal quantizer.
        //    That's what our for(;;) loop is about.
        for (;;)
        {
            //  1. Project input set on the axis in consideration.
            // From Foley & Van Dam: Closest point of approach of a line (P + v) to a
            // point (R) is
            //                            P + ((R-P).v) / (v.v))v
            // The distance along v is therefore (R-P).v / (v.v)
            // (v.v) is 1 if v is a unit vector.
            //
            PrjBnd0 = 1000.0f;
            PrjBnd1 = -1000.0f;
            for (i = 0; i < BLOCK_SIZE_4X4; i++)
                Prj0[i] = Prj[i] = PrjErr[i] = PreMRep[i] = 0.f;

            for (i = 0; i < dwUniqueColors; i++)
            {
                Prj0[i] = Prj[i] = dot(BlkSh[i], LineDir);
                PrjErr[i]        = dot(BlkSh[i] - LineDir * Prj[i], BlkSh[i] - LineDir * Prj[i]);
                PrjBnd0          = min(PrjBnd0, Prj[i]);
                PrjBnd1          = max(PrjBnd1, Prj[i]);
            }

            //  2. Run 1 dimensional search (see scalar case) to find an (sub) optimal
            //  pair of end points.

            // min and max of the search interval
            CGU_FLOAT Scl0;
            CGU_FLOAT Scl1;
            Scl0 = PrjBnd0 - (PrjBnd1 - PrjBnd0) * 0.125f;
            Scl1 = PrjBnd1 + (PrjBnd1 - PrjBnd0) * 0.125f;

            // compute scaling factor to scale down the search interval to [0.,1]
            const CGU_FLOAT Scl2    = (Scl1 - Scl0) * (Scl1 - Scl0);
            const CGU_FLOAT overScl = 1.f / (Scl1 - Scl0);

            for (i = 0; i < dwUniqueColors; i++)
            {
                // scale them
                Prj[i] = (Prj[i] - Scl0) * overScl;
                // premultiply the scale square to plug into error computation later
                PreMRep[i] = Rpt[i] * Scl2;
            }

            // scale first approximation of end points
            PrjBnd0 = (PrjBnd0 - Scl0) * overScl;
            PrjBnd1 = (PrjBnd1 - Scl0) * overScl;

            CGU_FLOAT StepErr = MAX_ERROR;

            // search step
            CGU_FLOAT searchStep = 0.025f;

            // low Start/End; high Start/End
            const CGU_FLOAT lowStartEnd  = (PrjBnd0 - 2.f * searchStep > 0.f) ? PrjBnd0 - 2.f * searchStep : 0.f;
            const CGU_FLOAT highStartEnd = (PrjBnd1 + 2.f * searchStep < 1.f) ? PrjBnd1 + 2.f * searchStep : 1.f;

            // find the best endpoints
            CGU_FLOAT Pos0 = 0;
            CGU_FLOAT Pos1 = 0;
            CGU_FLOAT lowPosStep, highPosStep;
            CGU_FLOAT err;

            int l, h;
            for (l = 0, lowPosStep = lowStartEnd; l < 8; l++, lowPosStep += searchStep)
            {
                for (h = 0, highPosStep = highStartEnd; h < 8; h++, highPosStep -= searchStep)
                {
                    // compute an error for the current pair of end points.
                    err = cgu_getRampErr(Prj, PrjErr, PreMRep, StepErr, lowPosStep, highPosStep, dwUniqueColors);

                    if (err < StepErr)
                    {
                        // save better result
                        StepErr = err;
                        Pos0    = lowPosStep;
                        Pos1    = highPosStep;
                    }
                }
            }

            // inverse the scaling
            Pos0 = Pos0 * (Scl1 - Scl0) + Scl0;
            Pos1 = Pos1 * (Scl1 - Scl0) + Scl0;

            // did we find somthing better from the previous run?
            if (StepErr + 0.001 < ErrG)
            {
                // yes, remember it
                ErrG     = StepErr;
                LineDirG = LineDir;

                PosG0.x = Pos0;
                PosG0.y = Pos0;
                PosG0.z = Pos0;
                PosG1.x = Pos1;
                PosG1.y = Pos1;
                PosG1.z = Pos1;

                //  3. Compute the vector of indexes (or clusters) for the current
                //  approximate ramp.
                // indexes
                const CGU_FLOAT step      = (Pos1 - Pos0) / 3.0f;  // (dwNumChannels=4 - 1);
                const CGU_FLOAT step_h    = step * (CGU_FLOAT)0.5;
                const CGU_FLOAT rstep     = (CGU_FLOAT)1.0f / step;
                const CGU_FLOAT overBlkTp = 1.f / 3.0f;  // (dwNumChannels=4 - 1);

                // here the index vector is computed,
                // shifted and normalized
                CGU_FLOAT indxAvrg = 3.0f / 2.0f;  // (dwNumChannels=4 - 1);

                for (i = 0; i < dwUniqueColors; i++)
                {
                    CGU_FLOAT del;
                    // CGU_UINT32 n = (CGU_UINT32)((b - _min_ex + (step*0.5f)) * rstep);
                    if ((del = Prj0[i] - Pos0) <= 0)
                        RmpIndxs[i] = 0.f;
                    else if (Prj0[i] - Pos1 >= 0)
                        RmpIndxs[i] = 3.0f;  // (dwNumChannels=4 - 1);
                    else
                        RmpIndxs[i] = floor((del + step_h) * rstep);
                    // shift and normalization
                    RmpIndxs[i] = (RmpIndxs[i] - indxAvrg) * overBlkTp;
                }

                //  4. Present our color channels as 3 16 DIM vectors.
                //  5. Find closest aproximation of each of 16DIM color vector with the
                //  pojection of the 16DIM index vector.
                CGU_Vec3f Crs = {0.0f, 0.0f, 0.0f};
                CGU_FLOAT Len = 0.0f;

                for (i = 0; i < dwUniqueColors; i++)
                {
                    const CGU_FLOAT PreMlt = RmpIndxs[i] * Rpt[i];
                    Len += RmpIndxs[i] * PreMlt;
                    Crs.x += BlkSh[i].x * PreMlt;
                    Crs.y += BlkSh[i].y * PreMlt;
                    Crs.z += BlkSh[i].z * PreMlt;
                }

                LineDir.x = LineDir.y = LineDir.z = 0.0f;
                if (Len > 0.0f)
                {
                    CGU_FLOAT Len2;
                    LineDir = Crs / Len;
                    //  6. Plug the projections as a new directional vector for the axis.
                    //  7. Goto 1.
                    Len2 = dot(LineDir, LineDir);  // LineDir.x * LineDir.x + LineDir.y * LineDir.y + LineDir.z * LineDir.z;
                    Len2 = sqrt(Len2);
                    LineDir /= Len2;
                }
            }
            else  // We was not able to find anything better.  Drop out.
                break;
        }

        // inverse transform to find end-points of 3-color ramp
        rsltC0 = (PosG0 * LineDirG + Mdl) * 255.f;
        rsltC1 = (PosG1 * LineDirG + Mdl) * 255.f;
    }  // !isDone

    // We've dealt with (almost) unrestricted full precision realm.
    // Now back digital world.

    // round the end points to make them look like compressed ones
    CGU_Vec3f inpRmpEndPts0 = {0.0f, 255.0f, 0.0f};
    CGU_Vec3f inpRmpEndPts1 = {0.0f, 255.0f, 0.0f};
    CGU_Vec3f Fctrs0        = {8.0f, 4.0f, 8.0f};     //(1 << (PIX_GRID - BG)); x (1 << (PIX_GRID - GG)); y (1 << (PIX_GRID - RG)); z
    CGU_Vec3f Fctrs1        = {32.0f, 64.0f, 32.0f};  //(CGU_FLOAT)(1 << RG); z (CGU_FLOAT)(1 << GG); y (CGU_FLOAT)(1 << BG); x
    CGU_FLOAT _Min          = 0.0f;
    CGU_FLOAT _Max          = 255.0f;

    {
        // MkRmpOnGrid(inpRmpEndPts, rsltC, _Min, _Max);

        inpRmpEndPts0 = floor(rsltC0);

        if (inpRmpEndPts0.x <= _Min)
            inpRmpEndPts0.x = _Min;
        else
        {
            inpRmpEndPts0.x += floor(128.f / Fctrs1.x) - floor(inpRmpEndPts0.x / Fctrs1.x);
            inpRmpEndPts0.x = min(inpRmpEndPts0.x, _Max);
        }
        if (inpRmpEndPts0.y <= _Min)
            inpRmpEndPts0.y = _Min;
        else
        {
            inpRmpEndPts0.y += floor(128.f / Fctrs1.y) - floor(inpRmpEndPts0.y / Fctrs1.y);
            inpRmpEndPts0.y = min(inpRmpEndPts0.y, _Max);
        }
        if (inpRmpEndPts0.z <= _Min)
            inpRmpEndPts0.z = _Min;
        else
        {
            inpRmpEndPts0.z += floor(128.f / Fctrs1.z) - floor(inpRmpEndPts0.z / Fctrs1.z);
            inpRmpEndPts0.z = min(inpRmpEndPts0.z, _Max);
        }

        inpRmpEndPts0 = floor(inpRmpEndPts0 / Fctrs0) * Fctrs0;

        inpRmpEndPts1 = floor(rsltC1);
        if (inpRmpEndPts1.x <= _Min)
            inpRmpEndPts1.x = _Min;
        else
        {
            inpRmpEndPts1.x += floor(128.f / Fctrs1.x) - floor(inpRmpEndPts1.x / Fctrs1.x);
            inpRmpEndPts1.x = min(inpRmpEndPts1.x, _Max);
        }
        if (inpRmpEndPts1.y <= _Min)
            inpRmpEndPts1.y = _Min;
        else
        {
            inpRmpEndPts1.y += floor(128.f / Fctrs1.y) - floor(inpRmpEndPts1.y / Fctrs1.y);
            inpRmpEndPts1.y = min(inpRmpEndPts1.y, _Max);
        }
        if (inpRmpEndPts1.z <= _Min)
            inpRmpEndPts1.z = _Min;
        else
        {
            inpRmpEndPts1.z += floor(128.f / Fctrs1.z) - floor(inpRmpEndPts1.z / Fctrs1.z);
            inpRmpEndPts1.z = min(inpRmpEndPts1.z, _Max);
        }

        inpRmpEndPts1 = floor(inpRmpEndPts1 / Fctrs0) * Fctrs0;
    }  // MkRmpOnGrid

    CMP_EndPoints EndPoints;
    EndPoints.Color0 = inpRmpEndPts0;
    EndPoints.Color1 = inpRmpEndPts1;

    return EndPoints;
}

CMP_STATIC CMP_EndPoints  cgu_MkRmpOnGridBGR(CMP_IN CGU_Vec3f rsltC0, 
                                             CMP_IN CGU_Vec3f rsltC1,
                                             CMP_IN CGU_UINT32  nRedBits, 
                                             CMP_IN CGU_UINT32  nGreenBits, 
                                             CMP_IN CGU_UINT32  nBlueBits)
{
    CGU_Vec3f inpRmpEndPts0 = {0.0f, 255.0f, 0.0f};
    CGU_Vec3f inpRmpEndPts1 = {0.0f, 255.0f, 0.0f};
    CGU_Vec3f Fctrs0        = {8.0f, 4.0f, 8.0f};
    CGU_Vec3f Fctrs1        = {32.0f, 64.0f, 32.0f};
    CGU_FLOAT _Min          = 0.0f;
    CGU_FLOAT _Max          = 255.0f;

    // user override 565 default setting
    if ((nRedBits!=5)||(nGreenBits!=6)||(nBlueBits!=5)) {
        Fctrs1[RC] = (CGU_FLOAT)(1 << nRedBits);
        Fctrs1[GC] = (CGU_FLOAT)(1 << nGreenBits);
        Fctrs1[BC] = (CGU_FLOAT)(1 << nBlueBits);
        Fctrs0[RC] = (CGU_FLOAT)(1 << (PIX_GRID-nRedBits));
        Fctrs0[GC] = (CGU_FLOAT)(1 << (PIX_GRID-nGreenBits));
        Fctrs0[BC] = (CGU_FLOAT)(1 << (PIX_GRID-nBlueBits));
    }

    inpRmpEndPts0 = cmp_floorVec3f(rsltC0);

    if (inpRmpEndPts0.x <= _Min)
        inpRmpEndPts0.x = _Min;
    else
    {
        inpRmpEndPts0.x += cmp_floor(128.f / Fctrs1.x) - cmp_floor(inpRmpEndPts0.x / Fctrs1.x);
        inpRmpEndPts0.x = cmp_minf(inpRmpEndPts0.x, _Max);
    }
    if (inpRmpEndPts0.y <= _Min)
        inpRmpEndPts0.y = _Min;
    else
    {
        inpRmpEndPts0.y += cmp_floor(128.f / Fctrs1.y) - cmp_floor(inpRmpEndPts0.y / Fctrs1.y);
        inpRmpEndPts0.y = cmp_minf(inpRmpEndPts0.y, _Max);
    }
    if (inpRmpEndPts0.z <= _Min)
        inpRmpEndPts0.z = _Min;
    else
    {
        inpRmpEndPts0.z += cmp_floor(128.f / Fctrs1.z) - cmp_floor(inpRmpEndPts0.z / Fctrs1.z);
        inpRmpEndPts0.z = cmp_minf(inpRmpEndPts0.z, _Max);
    }

    inpRmpEndPts0 = cmp_floorVec3f(inpRmpEndPts0 / Fctrs0) * Fctrs0;

    inpRmpEndPts1 = cmp_floorVec3f(rsltC1);
    if (inpRmpEndPts1.x <= _Min)
        inpRmpEndPts1.x = _Min;
    else
    {
        inpRmpEndPts1.x += cmp_floor(128.f / Fctrs1.x) - cmp_floor(inpRmpEndPts1.x / Fctrs1.x);
        inpRmpEndPts1.x = cmp_minf(inpRmpEndPts1.x, _Max);
    }
    if (inpRmpEndPts1.y <= _Min)
        inpRmpEndPts1.y = _Min;
    else
    {
        inpRmpEndPts1.y += cmp_floor(128.f / Fctrs1.y) - cmp_floor(inpRmpEndPts1.y / Fctrs1.y);
        inpRmpEndPts1.y = cmp_minf(inpRmpEndPts1.y, _Max);
    }
    if (inpRmpEndPts1.z <= _Min)
        inpRmpEndPts1.z = _Min;
    else
    {
        inpRmpEndPts1.z += cmp_floor(128.f / Fctrs1.z) - cmp_floor(inpRmpEndPts1.z / Fctrs1.z);
        inpRmpEndPts1.z = cmp_minf(inpRmpEndPts1.z, _Max);
    }

    inpRmpEndPts1 = cmp_floorVec3f(inpRmpEndPts1 / Fctrs0) * Fctrs0;

    CMP_EndPoints EndPoints;
    EndPoints.Color0 = inpRmpEndPts0;
    EndPoints.Color1 = inpRmpEndPts1;

     return EndPoints;

}  // MkRmpOnGrid


//===================================================================
// Replaces CompressBlockBC1_RGBA_Internal()
// if ((errLQ > 0.0f) && (fquality > CMP_QUALITY2)) code block
//===================================================================
CMP_STATIC CGU_Vec2ui cgu_CompRGBBlock(CMP_IN CGU_Vec4f src_imageNorm[BLOCK_SIZE_4X4],
                                       CMP_IN CMP_BC15Options BC15Options)
{
    //CGU_FLOAT  errLQ    = 1e6f;
    CGU_UINT32 m_nRefinementSteps = BC15Options.m_nRefinementSteps;
    CGU_UINT32 dwAlphaThreshold   = BC15Options.m_nAlphaThreshold;
    CGU_Vec3f  channelWeights     = {BC15Options.m_fChannelWeights[0],BC15Options.m_fChannelWeights[1],BC15Options.m_fChannelWeights[2]};
    CGU_BOOL   isSRGB             = BC15Options.m_bIsSRGB;

    CGU_Vec3f  rgbBlock_normal[BLOCK_SIZE_4X4];
    CGU_UINT32 nCmpIndices = 0;
    CGU_UINT32 c0, c1;
    // High Quality
    CMP_EndPoints EndPoints = {{0, 0, 0xFF}, {0, 0, 0xFF}};
    CGU_UINT32 i;
    
    ALIGN_16 CGU_FLOAT Rpt[BLOCK_SIZE_4X4];
    CGU_UINT32         pcIndices = 0;
    
    m_nRefinementSteps = 0;
    
    CGU_Vec3f BlkInBGRf_UV[BLOCK_SIZE_4X4];  // Normalized Block Input (0..1) in BGR channel format
    // Default inidices & endpoints for Transparent Block
    CGU_Vec3ui nEndpoints0 = {0, 0, 0};           // Endpoints are stored BGR as x,y,z
    CGU_Vec3ui nEndpoints1 = {0xFF, 0xFF, 0xFF};  // Endpoints are stored BGR as x,y,z
    
    for (i = 0; i < BLOCK_SIZE_4X4; i++)
    {
        Rpt[i] = 0.0f;
    }
    
    //===============================================================
    // Check if we have more then 2 colors and process Alpha block
    CGU_UINT32 dwColors = 0;
    CGU_UINT32 dwBlk[BLOCK_SIZE_4X4];
    CGU_UINT32 R, G, B, A;
    for (i = 0; i < BLOCK_SIZE_4X4; i++)
    {
        // Do any color conversion prior to processing the block
        rgbBlock_normal[i] = isSRGB ? cmp_linearToSrgb(src_imageNorm[i].rgb) : src_imageNorm[i].rgb;
    
        R = (CGU_UINT32)(rgbBlock_normal[i].x * 255.0f);
        G = (CGU_UINT32)(rgbBlock_normal[i].y * 255.0f);
        B = (CGU_UINT32)(rgbBlock_normal[i].z * 255.0f);
    
        //if (dwAlphaThreshold > 0)
        //    A = (CGU_UINT32)src_imageNorm[i].w * 255.0f;
        //else
            A = 255;
    
        // Punch Through Alpha in BC1 Codec (1 bit alpha)
        //if ((dwAlphaThreshold == 0) || (A >= dwAlphaThreshold))
        //{
            // copy to local RGB data and have alpha set to 0xFF
            dwBlk[dwColors++] = A << 24 | R << 16 | G << 8 | B;
        //}
    }
    
    if (!dwColors)
    {
        // All are colors transparent
        EndPoints.Color0.x = EndPoints.Color0.y = EndPoints.Color0.z = 0.0f;
        EndPoints.Color1.x = EndPoints.Color1.y = EndPoints.Color0.z = 255.0f;
        nCmpIndices                                                  = 0xFFFFFFFF;
    }
    else
    {
        // We have colors to process
        nCmpIndices = 0;
        // Punch Through Alpha Support ToDo
        // CGU_BOOL bHasAlpha = (dwColors != BLOCK_SIZE_4X4);
        // bHasAlpha = bHasAlpha && (dwAlphaThreshold > 0); // valid for  (dwNumChannels=4);
        // if (bHasAlpha) {
        //      CGU_Vec2ui  compBlock = {0xf800f800,0};
        //     return compBlock;
        // }
        
        // Here we are computing an unique number of sorted colors.
        // For each unique value we compute the number of it appearences.
        // qsort((void *)dwBlk, (size_t)dwColors, sizeof(CGU_UINT32), QSortIntCmp);
        {
            CGU_UINT32 j;
            CMP_di     what[BLOCK_SIZE_4X4];
    
            for (i = 0; i < dwColors; i++)
            {
                what[i].index = i;
                what[i].data  = dwBlk[i];
            }
    
            CGU_UINT32 tmp_index;
            CGU_UINT32 tmp_data;
    
            for (i = 1; i < dwColors; i++)
            {
                for (j = i; j > 0; j--)
                {
                    if (what[j - 1].data > what[j].data)
                    {
                        tmp_index         = what[j].index;
                        tmp_data          = what[j].data;
                        what[j].index     = what[j - 1].index;
                        what[j].data      = what[j - 1].data;
                        what[j - 1].index = tmp_index;
                        what[j - 1].data  = tmp_data;
                    }
                }
            }
            for (i = 0; i < dwColors; i++)
                dwBlk[i] = what[i].data;
        }
        CGU_UINT32 new_p;
        CGU_UINT32 dwBlkU[BLOCK_SIZE_4X4];
        CGU_UINT32 dwUniqueColors = 0;
        new_p = dwBlkU[0]   = dwBlk[0];
        Rpt[dwUniqueColors] = 1.f;
        for (i = 1; i < dwColors; i++)
        {
            if (new_p != dwBlk[i])
            {
                dwUniqueColors++;
                new_p = dwBlkU[dwUniqueColors] = dwBlk[i];
                Rpt[dwUniqueColors]            = 1.f;
            }
            else
                Rpt[dwUniqueColors] += 1.f;
        }
        dwUniqueColors++;
    
        // Simple case of only 2 colors to process
        // no need for futher processing as lowest quality methods work best for this case
        if (dwUniqueColors <= 2)
        {
            CGU_Vec3f  rsltC0;
            CGU_Vec3f  rsltC1;
            rsltC0.r = rgbBlock_normal[0].b * 255.0f;
            rsltC0.g = rgbBlock_normal[0].g * 255.0f;
            rsltC0.b = rgbBlock_normal[0].r * 255.0f;
            rsltC1.r = rgbBlock_normal[dwUniqueColors - 1].b * 255.0f;
            rsltC1.g = rgbBlock_normal[dwUniqueColors - 1].g * 255.0f;
            rsltC1.b = rgbBlock_normal[dwUniqueColors - 1].r * 255.0f;
            EndPoints = cgu_MkRmpOnGridBGR(rsltC0, rsltC1,5, 6, 5);
        }
        else
            {
                // switch from int range back to UV floats
                for (i = 0; i < dwUniqueColors; i++)
                {
                    R                 = (dwBlkU[i] >> 16) & 0xff;
                    G                 = (dwBlkU[i] >> 8) & 0xff;
                    B                 = (dwBlkU[i] >> 0) & 0xff;
                    BlkInBGRf_UV[i].z = (CGU_FLOAT)R / 255.0f;
                    BlkInBGRf_UV[i].y = (CGU_FLOAT)G / 255.0f;
                    BlkInBGRf_UV[i].x = (CGU_FLOAT)B / 255.0f;
                }

                CGU_Vec3f channelWeightsBGR;
                channelWeightsBGR.x = channelWeights.z;
                channelWeightsBGR.y = channelWeights.y;
                channelWeightsBGR.z = channelWeights.x;

                EndPoints = cgu_CompressRGBBlockX(BlkInBGRf_UV, Rpt, dwUniqueColors, channelWeightsBGR, m_nRefinementSteps);
            }
    }  // colors
    
    //===================================================================
    // Process Cluster INPUT is constant EndPointsf OUTPUT is pcIndices
    //===================================================================
    if (nCmpIndices == 0)
        {
            R                  = (CGU_UINT32)(EndPoints.Color0.z);
            G                  = (CGU_UINT32)(EndPoints.Color0.y);
            B                  = (CGU_UINT32)(EndPoints.Color0.x);
            CGU_INT32 cluster0 = cmp_constructColor(R, G, B);

            R                  = (CGU_UINT32)(EndPoints.Color1.z);
            G                  = (CGU_UINT32)(EndPoints.Color1.y);
            B                  = (CGU_UINT32)(EndPoints.Color1.x);
            CGU_INT32 cluster1 = cmp_constructColor(R, G, B);

            CGU_Vec3f InpRmp[NUM_ENDPOINTS];
            if ((cluster0 <= cluster1)  // valid for 4 channels
                                        // || (cluster0 > cluster1)    // valid for 3 channels
            )
            {
                // inverse endpoints
                InpRmp[0] = EndPoints.Color1;
                InpRmp[1] = EndPoints.Color0;
            }
            else
            {
                InpRmp[0] = EndPoints.Color0;
                InpRmp[1] = EndPoints.Color1;
            }

            CGU_Vec3f srcblockBGR[BLOCK_SIZE_4X4];
            CGU_FLOAT srcblockA[BLOCK_SIZE_4X4];

            // Swizzle the source RGB to BGR for processing
            for (i = 0; i < BLOCK_SIZE_4X4; i++)
            {
                srcblockBGR[i].z = rgbBlock_normal[i].x * 255.0f;
                srcblockBGR[i].y = rgbBlock_normal[i].y * 255.0f;
                srcblockBGR[i].x = rgbBlock_normal[i].z * 255.0f;
                srcblockA[i]     = 255.0f;
                if (dwAlphaThreshold > 0)
                {
                    CGU_UINT32 alpha = (CGU_UINT32)src_imageNorm[i].w*255.0f;
                    if (alpha >= dwAlphaThreshold)
                        srcblockA[i] = alpha;
                }
            }

            // input ramp is on the coarse grid
            // make ramp endpoints the way they'll going to be decompressed
            CGU_Vec3f InpRmpL[NUM_ENDPOINTS];
            CGU_Vec3f Fctrs = {32.0F, 64.0F, 32.0F};  // 1 << RG,1 << GG,1 << BG

            {
                //   ConstantRamp = MkWkRmpPts(InpRmpL, InpRmp);
                InpRmpL[0] = InpRmp[0] + floor(InpRmp[0] / Fctrs);
                InpRmpL[0] = cmp_clampVec3f(InpRmpL[0], 0.0f, 255.0f);
                InpRmpL[1] = InpRmp[1] + floor(InpRmp[1] / Fctrs);
                InpRmpL[1] = cmp_clampVec3f(InpRmpL[1], 0.0f, 255.0f);
            }  // MkWkRmpPts

            // build ramp
            CGU_Vec3f LerpRmp[4];
            CGU_Vec3f offset = {1.0f, 1.0f, 1.0f};
            {
                //BldRmp(Rmp, InpRmpL, dwNumChannels);
                // linear interpolate end points to get the ramp
                LerpRmp[0] = InpRmpL[0];
                LerpRmp[3] = InpRmpL[1];
                LerpRmp[1] = floor((InpRmpL[0] * 2.0f + LerpRmp[3] + offset) / 3.0f);
                LerpRmp[2] = floor((InpRmpL[0] + LerpRmp[3] * 2.0f + offset) / 3.0f);
            }  // BldRmp

            //=========================================================================
            // Clusterize, Compute error and find DXTC indexes for the current cluster
            //=========================================================================
            {
                // Clusterize
                CGU_UINT32 alpha;

                // For each colour in the original block assign it
                // to the closest cluster and compute the cumulative error
                for (i = 0; i < BLOCK_SIZE_4X4; i++)
                {
                    alpha = (CGU_UINT32)srcblockA[i];
                    if ((dwAlphaThreshold > 0) && alpha == 0)
                    {                                      //*((CGU_DWORD *)&_Blk[i][AC]) == 0)
                        pcIndices |= cmp_set2Bit32(4, i);  // dwNumChannels 3 or 4 (default is 4)
                    }
                    else
                    {
                        CGU_FLOAT shortest      = 99999999999.f;
                        CGU_UINT8 shortestIndex = 0;

                        CGU_Vec3f channelWeightsBGR;
                        channelWeightsBGR.x = channelWeights.z;
                        channelWeightsBGR.y = channelWeights.y;
                        channelWeightsBGR.z = channelWeights.x;

                        for (CGU_UINT8 rampindex = 0; rampindex < 4; rampindex++)
                        {
                            // r is either 1 or 4
                            // calculate the distance for each component
                            CGU_FLOAT distance =
                                dot(((srcblockBGR[i] - LerpRmp[rampindex]) * channelWeightsBGR), ((srcblockBGR[i] - LerpRmp[rampindex]) * channelWeightsBGR));
                            if (distance < shortest)
                            {
                                shortest      = distance;
                                shortestIndex = rampindex;
                            }
                        }

                        // The total is a sum of (error += shortest)
                        // We have the index of the best cluster, so assign this in the block
                        // Reorder indices to match correct DXTC ordering
                        if (shortestIndex == 3)  // dwNumChannels - 1
                            shortestIndex = 1;
                        else if (shortestIndex)
                            shortestIndex++;
                        pcIndices |= cmp_set2Bit32(shortestIndex, i);
                    }
                }  // BLOCK_SIZE_4X4
            }      // Clusterize
        }          // Process Cluster
    
    //==============================================================
    // Generate Compressed Result from nEndpoints & pcIndices
    //==============================================================
    c0 = cmp_constructColorBGR(EndPoints.Color0);
    c1 = cmp_constructColorBGR(EndPoints.Color1);
    
    // Get Processed indices if not set
    if (nCmpIndices == 0)
        nCmpIndices = pcIndices;
    
    CGU_Vec2ui cmpBlock;
    if (c0 <= c1)
    {
        cmpBlock.x = c1 | (c0 << 16);
    }
    else
        cmpBlock.x = c0 | (c1 << 16);
    
    cmpBlock.y = nCmpIndices;
    
    return cmpBlock;
}

CMP_STATIC void  cgu_ProcessColors(CMP_INOUT CGU_Vec3f CMP_PTRINOUT colorMin,
                                   CMP_INOUT CGU_Vec3f CMP_PTRINOUT colorMax,
                                   CMP_INOUT CGU_UINT32 CMP_PTRINOUT c0,
                                   CMP_INOUT CGU_UINT32 CMP_PTRINOUT c1,
                                   CMP_IN    CGU_INT    setopt,
                                   CMP_IN    CGU_BOOL   isSRGB)
{
    // CGU_UINT32 srbMap[32] = {0,5,8,11,12,13,14,15,16,17,18,19,20,21,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31};
    // CGU_UINT32 sgMap[64]  = {0,10,14,16,19,20,22,24,25,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,42,43,43,44,45,45,
    //                          46,47,47,48,48,49,50,50,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,63};
    CGU_INT32 x, y, z;
    CGU_Vec3f scale = {31.0f, 63.0f, 31.0f};
    CGU_Vec3f MinColorScaled;
    CGU_Vec3f MaxColorScaled;

    // Clamp or Transform is needed, the transforms have built in clamps
    if (isSRGB)
    {
        MinColorScaled = cmp_linearToSrgb(CMP_PTRINOUT colorMin);
        MaxColorScaled = cmp_linearToSrgb(CMP_PTRINOUT colorMax);
    }
    else
    {
        MinColorScaled = cmp_clampVec3f(CMP_PTRINOUT colorMin, 0.0f, 1.0f);
        MaxColorScaled = cmp_clampVec3f(CMP_PTRINOUT colorMax, 0.0f, 1.0f);
    }

    switch (setopt)
    {
    case 0:  // Use Min Max processing
        MinColorScaled        = cmp_floorVec3f(MinColorScaled * scale);
        MaxColorScaled        = cmp_ceilVec3f(MaxColorScaled * scale);
        CMP_PTRINOUT colorMin = MinColorScaled / scale;
        CMP_PTRINOUT colorMax = MaxColorScaled / scale;
        break;
    default:  // Use round processing
        MinColorScaled = round(MinColorScaled * scale);
        MaxColorScaled = round(MaxColorScaled * scale);
        break;
    }

    x = (CGU_UINT32)(MinColorScaled.x);
    y = (CGU_UINT32)(MinColorScaled.y);
    z = (CGU_UINT32)(MinColorScaled.z);

    //if (isSRGB) {
    //    // scale RB
    //    x = srbMap[x]; // &0x1F];
    //    y = sgMap [y]; // &0x3F];
    //    z = srbMap[z]; // &0x1F];
    //    // scale G
    //}
    CMP_PTRINOUT c0 = (x << 11) | (y << 5) | z;

    x               = (CGU_UINT32)(MaxColorScaled.x);
    y               = (CGU_UINT32)(MaxColorScaled.y);
    z               = (CGU_UINT32)(MaxColorScaled.z);
    CMP_PTRINOUT c1 = (x << 11) | (y << 5) | z;
}

CMP_STATIC CGU_FLOAT cgu_getIndicesRGB(CMP_INOUT CGU_UINT32 CMP_PTRINOUT cmpindex,
                                       CMP_IN const CGU_Vec3f            block[16],
                                       CMP_IN CGU_Vec3f                  minColor,
                                       CMP_IN CGU_Vec3f                  maxColor,
                                       CMP_IN CGU_BOOL                   getErr)
{
    CGU_UINT32 PackedIndices = 0;
    CGU_FLOAT  err           = 0.0f;
    CGU_Vec3f  cn[4];
    CGU_FLOAT  minDistance;

    if (getErr)
    {
        // remap to BC1 spec for decoding offsets,
        // where cn[0] > cn[1] Max Color = index 0, 2/3 offset =index 2, 1/3 offset = index 3, Min Color = index 1
        cn[0] = maxColor;
        cn[1] = minColor;
        cn[2] = cn[0] * 2.0f / 3.0f + cn[1] * 1.0f / 3.0f;
        cn[3] = cn[0] * 1.0f / 3.0f + cn[1] * 2.0f / 3.0f;
    }

    CGU_FLOAT  Scale       = 3.f / cmp_dotVec3f(minColor - maxColor, minColor - maxColor);
    CGU_Vec3f  ScaledRange = (minColor - maxColor) * Scale;
    CGU_FLOAT  Bias        = (cmp_dotVec3f(maxColor, maxColor) - cmp_dotVec3f(maxColor, minColor)) * Scale;
    CGU_INT    indexMap[4] = {0, 2, 3, 1};  // mapping based on BC1 Spec for color0 > color1
    CGU_UINT32 index;
    CGU_FLOAT  diff;

    for (CGU_UINT32 i = 0; i < 16; i++)
    {
        // Get offset from base scale
        diff  = cmp_dotVec3f(block[i], ScaledRange) + Bias;
        index = ((CGU_UINT32)round(diff)) & 0x3;

        // remap linear offset to spec offset
        index = indexMap[index];

        // use err calc for use in higher quality code
        if (getErr)
        {
            minDistance = cmp_dotVec3f(block[i] - cn[index], block[i] - cn[index]);
            err += minDistance;
        }

        // Map the 2 bit index into compress 32 bit block
        if (index)
            PackedIndices |= (index << (2 * i));
    }

    if (getErr)
        err = err * 0.0208333f;

    CMP_PTRINOUT cmpindex = PackedIndices;
    return err;
}

//--------------------------------------------------------------------------------------------------------
// Decompress is RGB (0.0f..255.0f)
//--------------------------------------------------------------------------------------------------------
CMP_STATIC void  cgu_decompressRGBBlock(CMP_INOUT CGU_Vec3f rgbBlock[BLOCK_SIZE_4X4], const CGU_Vec2ui compressedBlock)
{
    CGU_UINT32 n0 = compressedBlock.x & 0xffff;
    CGU_UINT32 n1 = compressedBlock.x >> 16;
    CGU_UINT32 index;

    //-------------------------------------------------------
    // Decode the compressed block 0..255 color range
    //-------------------------------------------------------
    CGU_Vec3f c0 = cmp_565ToLinear(n0);  // max color
    CGU_Vec3f c1 = cmp_565ToLinear(n1);  // min color
    CGU_Vec3f c2;
    CGU_Vec3f c3;

    if (n0 > n1)
    {
        c2 = (c0 * 2.0f + c1) / 3.0f;
        c3 = (c1 * 2.0f + c0) / 3.0f;

        for (CGU_UINT32 i = 0; i < 16; i++)
        {
            index = (compressedBlock.y >> (2 * i)) & 3;
            switch (index)
            {
            case 0:
                rgbBlock[i] = c0;
                break;
            case 1:
                rgbBlock[i] = c1;
                break;
            case 2:
                rgbBlock[i] = c2;
                break;
            case 3:
                rgbBlock[i] = c3;
                break;
            }
        }
    }
    else
    {
        // Transparent decode
        c2 = (c0 + c1) / 2.0f;

        for (CGU_UINT32 i = 0; i < 16; i++)
        {
            index = (compressedBlock.y >> (2 * i)) & 3;
            switch (index)
            {
            case 0:
                rgbBlock[i] = c0;
                break;
            case 1:
                rgbBlock[i] = c1;
                break;
            case 2:
                rgbBlock[i] = c2;
                break;
            case 3:
                rgbBlock[i] = 0.0f;
                break;
            }
        }
    }
}

// The source is 0..255
CMP_STATIC float cgu_RGBABlockErrorLinear(const CGU_Vec4uc src_rgbBlock[BLOCK_SIZE_4X4], const CGU_Vec2ui compressedBlock)
{
    CGU_Vec3f rgbBlock[BLOCK_SIZE_4X4];

    // Decompressed block channels are 0..255
    cgu_decompressRGBBlock(rgbBlock, compressedBlock);

    //------------------------------------------------------------------
    // Calculate MSE of the block
    // Note : pow is used as Float type for the code to be usable on CPU
    //------------------------------------------------------------------
    CGU_Vec3f serr;
    serr = 0.0f;

    float sR, sG, sB, R, G, B;

    for (int j = 0; j < 16; j++)
    {
        sR = src_rgbBlock[j].x;
        sG = src_rgbBlock[j].y;
        sB = src_rgbBlock[j].z;

        R = rgbBlock[j].x;
        G = rgbBlock[j].y;
        B = rgbBlock[j].z;

        // Norm colors
        serr.x += pow(sR - R, 2.0f);
        serr.y += pow(sG - G, 2.0f);
        serr.z += pow(sB - B, 2.0f);
    }

    // MSE for 16 texels
    return (serr.x + serr.y + serr.z) / 48.0f;
}

// The source is 0..1, decompressed data using cmp_decompressRGBBlock2 is 0..255 which is converted down to 0..1
CMP_STATIC float cgu_RGBBlockError(const CGU_Vec3f src_rgbBlock[BLOCK_SIZE_4X4], const CGU_Vec2ui compressedBlock, CGU_BOOL isSRGB)
{
    CGU_Vec3f rgbBlock[BLOCK_SIZE_4X4];

    // Decompressed block channels are 0..255
    cgu_decompressRGBBlock(rgbBlock, compressedBlock);

    //------------------------------------------------------------------
    // Calculate MSE of the block
    // Note : pow is used as Float type for the code to be usable on CPU
    //------------------------------------------------------------------
    CGU_Vec3f serr;
    serr = 0.0f;

    float sR, sG, sB, R, G, B;

    for (int j = 0; j < 16; j++)
    {
        if (isSRGB)
        {
            sR = round(cmp_linearToSrgbf(src_rgbBlock[j].x) * 255.0f);
            sG = round(cmp_linearToSrgbf(src_rgbBlock[j].y) * 255.0f);
            sB = round(cmp_linearToSrgbf(src_rgbBlock[j].z) * 255.0f);
        }
        else
        {
            sR = round(src_rgbBlock[j].x * 255.0f);
            sG = round(src_rgbBlock[j].y * 255.0f);
            sB = round(src_rgbBlock[j].z * 255.0f);
        }

        R = rgbBlock[j].x;
        G = rgbBlock[j].y;
        B = rgbBlock[j].z;

        // Norm colors
        serr.x += pow(sR - R, 2.0f);
        serr.y += pow(sG - G, 2.0f);
        serr.z += pow(sB - B, 2.0f);
    }

    // MSE for 16 texels
    return (serr.x + serr.y + serr.z) / 48.0f;
}

CMP_STATIC CGU_Vec2ui cgu_CompressRGBBlock_MinMax(CMP_IN    const CGU_Vec3f src_imageRGB[16], 
                                                  CMP_IN    CGU_FLOAT fquality, 
                                                  CMP_IN    CGU_BOOL isSRGB, 
                                                  CMP_INOUT CGU_Vec3f srcRGB[16],                    // The list of source colors with blue channel altered
                                                  CMP_INOUT CGU_Vec3f CMP_REFINOUT  average_rgb,     // The centrepoint of the axis
                                                  CMP_INOUT CGU_FLOAT CMP_REFINOUT errout
)
{
    CGU_Vec2ui Q1CompData = {0,0};
    CGU_Vec3f  rgb = {0,0,0};

    // -------------------------------------------------------------------------------------
    // (1) Find the array of unique pixel values and sum them to find their average position
    // -------------------------------------------------------------------------------------
    CGU_FLOAT  errLQ             = 0.0f;
    CGU_BOOL   fastProcess       = (fquality <= CMP_QUALITY0); // Min Max only
    CGU_Vec3f  srcMin            = 1.0f;  // Min source color
    CGU_Vec3f  srcMax            = 0.0f;  // Max source color
    CGU_Vec2ui Q1compressedBlock = {0, 0};
    CGU_UINT32 c0 = 0;
    CGU_UINT32 c1 = 0;

    average_rgb = 0.0f;
    // Get average and modifed src
    // find average position and save list of pixels as 0F..255F range for processing
    // Note: z (blue) is average of blue+green channels
    for (CGU_INT32 i = 0; i < BLOCK_SIZE_4X4; i++)
    {
        srcMin = cmp_minVec3f(srcMin, src_imageRGB[i]);
        srcMax = cmp_maxVec3f(srcMax, src_imageRGB[i]);
        if (!fastProcess)
        {
            rgb         = isSRGB ? cmp_linearToSrgb(src_imageRGB[i]) : cmp_saturate(src_imageRGB[i]);
            rgb.z       = (rgb.y + rgb.z) * 0.5F;  // Z-axiz => (R+G)/2
            srcRGB[i]   = rgb;
            average_rgb = average_rgb + rgb;
        }
    }

    // Process two colors for saving in 565 format as C0 and C1
    cgu_ProcessColors(CMP_REFINOUT srcMin, CMP_REFINOUT srcMax, CMP_REFINOUT c0, CMP_REFINOUT c1, isSRGB ? 1 : 0, isSRGB);

    // Save simple min-max encoding
    if (c0 < c1)
    {
        Q1CompData.x = (c0 << 16) | c1;
        CGU_UINT32 index = 0;
        errLQ               = cgu_getIndicesRGB(CMP_REFINOUT index, src_imageRGB, srcMin, srcMax, false);
        Q1CompData.y        = index;
        errout  = cgu_RGBBlockError(src_imageRGB, Q1CompData, isSRGB);
    }
    else
    {
        // Most simple case all colors are equal or 0.0f
        Q1compressedBlock.x = (c1 << 16) | c0;
        Q1compressedBlock.y = 0;
        errout = 0.0f;
        return Q1compressedBlock;
    }
    // 0.0625F is (1/BLOCK_SIZE_4X4)
    average_rgb = average_rgb * 0.0625F;

    return Q1CompData;
}


CMP_STATIC CGU_Vec2ui cgu_CompressRGBBlock_Fast(CMP_IN const CGU_Vec3f src_imageRGB[16], 
                                                CMP_IN CGU_FLOAT fquality, 
                                                CMP_IN CGU_BOOL isSRGB, 
                                                CMP_IN CGU_Vec3f srcRGB[16],
                                                CMP_IN CGU_Vec3f CMP_REFINOUT  average_rgb, 
                                                CMP_INOUT CGU_FLOAT CMP_REFINOUT errout)
{
    CGU_Vec3f  axisVectorRGB = {0.0f, 0.0f, 0.0f};  // The axis vector for index projection
    CGU_FLOAT  pos_on_axis[16];                     // The distance each unique falls along the compression axis
    CGU_FLOAT  axisleft   = 0;                      // The extremities and centre (average of left/right) of srcRGB along the compression axis
    CGU_FLOAT  axisright  = 0;                      // The extremities and centre (average of left/right) of srcRGB along the compression axis
    CGU_FLOAT  axiscentre = 0;                      // The extremities and centre (average of left/right) of srcRGB along the compression axis
    CGU_INT32  swap       = 0;                      // Indicator if the RGB values need swapping to generate an opaque result
    CGU_Vec3f  srcBlock[16];                        // The list of source colors with any color space transforms and clipping
    CGU_UINT32 c0 = 0;
    CGU_UINT32 c1 = 0;
    CGU_Vec2ui compressedBlock = {0, 0};
    CGU_FLOAT  Q1CompErr;
    CGU_Vec2ui Q1CompData = {0,0};



    CGU_Vec3f  rgb = {0,0,0};

    // -------------------------------------------------------------------------------------
    // (4) For each component, reflect points about the average so all lie on the same side
    // of the average, and compute the new average - this gives a second point that defines the axis
    // To compute the sign of the axis sum the positive differences of G for each of R and B (the
    // G axis is always positive in this implementation
    // -------------------------------------------------------------------------------------
    // An interesting situation occurs if the G axis contains no information, in which case the RB
    // axis is also compared. I am not entirely sure if this is the correct implementation - should
    // the priority axis be determined by magnitude?
    {
        CGU_FLOAT rg_pos = 0.0f;
        CGU_FLOAT bg_pos = 0.0f;
        CGU_FLOAT rb_pos = 0.0f;

        for (CGU_INT32 i = 0; i < BLOCK_SIZE_4X4; i++)
        {
            rgb           = srcRGB[i] - average_rgb;
            axisVectorRGB = axisVectorRGB + cmp_fabsVec3f(rgb);
            if (rgb.x > 0)
            {
                rg_pos += rgb.y;
                rb_pos += rgb.z;
            }
            if (rgb.z > 0)
                bg_pos += rgb.y;
        }

        // Average over BLOCK_SIZE_4X4
        axisVectorRGB = axisVectorRGB * 0.0625F;

        // New average position
        if (rg_pos < 0)
            axisVectorRGB.x = -axisVectorRGB.x;
        if (bg_pos < 0)
            axisVectorRGB.z = -axisVectorRGB.z;
        if ((rg_pos == bg_pos) && (rg_pos == 0))
        {
            if (rb_pos < 0)
                axisVectorRGB.z = -axisVectorRGB.z;
        }
    }

    // -------------------------------------------------------------------------------------
    // (5) Axis projection and remapping
    // -------------------------------------------------------------------------------------
    {
        CGU_FLOAT v2_recip;
        // Normalize the axis for simplicity of future calculation
        v2_recip = cmp_dotVec3f(axisVectorRGB, axisVectorRGB);
        if (v2_recip > 0)
            v2_recip = 1.0f / (CGU_FLOAT)cmp_sqrt(v2_recip);
        else
            v2_recip = 1.0f;
        axisVectorRGB = axisVectorRGB * v2_recip;
    }

    // -------------------------------------------------------------------------------------
    // (6) Map the axis
    // -------------------------------------------------------------------------------------
    // the line joining (and extended on either side of) average and axis
    // defines the axis onto which the points will be projected
    // Project all the points onto the axis, calculate the distance along
    // the axis from the centre of the axis (average)
    // From Foley & Van Dam: Closest point of approach of a line (P + v) to a point (R) is
    //     P + ((R-P).v) / (v.v))v
    // The distance along v is therefore (R-P).v / (v.v) where (v.v) is 1 if v is a unit vector.
    //
    // Calculate the extremities at the same time - these need to be reasonably accurately
    // represented in all cases
    {
        axisleft  = CMP_FLOAT_MAX;
        axisright = -CMP_FLOAT_MAX;
        for (CGU_INT32 i = 0; i < BLOCK_SIZE_4X4; i++)
        {
            // Compute the distance along the axis of the point of closest approach
            CGU_Vec3f temp = (srcRGB[i] - average_rgb);
            pos_on_axis[i] = cmp_dotVec3f(temp, axisVectorRGB);

            // Work out the extremities
            if (pos_on_axis[i] < axisleft)
                axisleft = pos_on_axis[i];
            if (pos_on_axis[i] > axisright)
                axisright = pos_on_axis[i];
        }
    }

    // ---------------------------------------------------------------------------------------------
    // (7) Now we have a good axis and the basic information about how the points are mapped to it
    // Our initial guess is to represent the endpoints accurately, by moving the average
    // to the centre and recalculating the point positions along the line
    // ---------------------------------------------------------------------------------------------
    {
        axiscentre  = (axisleft + axisright) * 0.5F;
        average_rgb = average_rgb + (axisVectorRGB * axiscentre);
        for (CGU_INT32 i = 0; i < BLOCK_SIZE_4X4; i++)
            pos_on_axis[i] -= axiscentre;
        axisright -= axiscentre;
        axisleft -= axiscentre;
    }

    // -------------------------------------------------------------------------------------
    // (8) Calculate the high and low output colour values
    // Involved in this is a rounding procedure which is undoubtedly slightly twitchy. A
    // straight rounded average is not correct, as the decompressor 'unrounds' by replicating
    // the top bits to the bottom.
    // In order to take account of this process, we don't just apply a straight rounding correction,
    // but base our rounding on the input value (a straight rounding is actually pretty good in terms of
    // error measure, but creates a visual colour and/or brightness shift relative to the original image)
    // The method used here is to apply a centre-biased rounding dependent on the input value, which was
    // (mostly by experiment) found to give minimum MSE while preserving the visual characteristics of
    // the image.
    // rgb = (average_rgb + (left|right)*axisVectorRGB);
    // -------------------------------------------------------------------------------------
    {
        CGU_Vec3f MinColor, MaxColor;

        MinColor   = average_rgb + (axisVectorRGB * axisleft);
        MaxColor   = average_rgb + (axisVectorRGB * axisright);
        MinColor.z = (MinColor.z * 2) - MinColor.y;
        MaxColor.z = (MaxColor.z * 2) - MaxColor.y;

        cgu_ProcessColors(CMP_REFINOUT MinColor, CMP_REFINOUT MaxColor, CMP_REFINOUT c0, CMP_REFINOUT c1, 1, false);

        // Force to be a 4-colour opaque block - in which case, c0 is greater than c1
        swap = 0;
        if (c0 < c1)
        {
            CGU_UINT32 t;
            t    = c0;
            c0   = c1;
            c1   = t;
            swap = 1;
        }
        else if (c0 == c1)
        {
            // This block will always be encoded in 3-colour mode
            // Need to ensure that only one of the two points gets used,
            // avoiding accidentally setting some transparent pixels into the block
            for (CGU_INT32 i = 0; i < BLOCK_SIZE_4X4; i++)
                pos_on_axis[i] = axisleft;
        }

        compressedBlock.x = c0 | (c1 << 16);

        // -------------------------------------------------------------------------------------
        // (9) Final clustering, creating the 2-bit values that define the output
        // -------------------------------------------------------------------------------------

        CGU_UINT32 index;
        CGU_FLOAT  division;
        {
            compressedBlock.y = 0;
            division          = axisright * 2.0f / 3.0f;
            axiscentre        = (axisleft + axisright) / 2;  // Actually, this code only works if centre is 0 or approximately so

            CGU_FLOAT CompMinErr;

            // This feature is work in progress
            // remap to BC1 spec for decoding offsets,
            // where cn[0] > cn[1] Max Color = index 0, 2/3 offset =index 2, 1/3 offset = index 3, Min Color = index 1
            // CGU_Vec3f   cn[4];
            // cn[0] = MaxColor;
            // cn[1] = MinColor;
            // cn[2] = cn[0]*2.0f/3.0f + cn[1]*1.0f/3.0f;
            // cn[3] = cn[0]*1.0f/3.0f + cn[1]*2.0f/3.0f;

            for (CGU_INT32 i = 0; i < BLOCK_SIZE_4X4; i++)
            {
                // Endpoints (indicated by block > average) are 0 and 1, while
                // interpolants are 2 and 3
                if (cmp_fabs(pos_on_axis[i]) >= division)
                    index = 0;
                else
                    index = 2;
                // Positive is in the latter half of the block
                if (pos_on_axis[i] >= axiscentre)
                    index += 1;

                index = index ^ swap;
                // Set the output, taking swapping into account
                compressedBlock.y |= (index << (2 * i));

                // use err calc for use in higher quality code
                //CompMinErr += cmp_dotVec3f(srcRGBRef[i] - cn[index],srcRGBRef[i] - cn[index]);
            }

            //CompMinErr = CompMinErr * 0.0208333f;

            CompMinErr = cgu_RGBBlockError(src_imageRGB, compressedBlock, isSRGB);
            Q1CompErr  = cgu_RGBBlockError(src_imageRGB, Q1CompData, isSRGB);

            if (CompMinErr > Q1CompErr)
            {
                compressedBlock     = Q1CompData;
                errout = Q1CompErr;
            }
            else
                errout = CompMinErr;
        }
    }
    // done

    return compressedBlock;
}

CMP_STATIC CGU_UINT8 g_Match5Bit[256][2] = {
    { 0, 0},{ 0, 0},{ 1, 0},{ 1, 0},{ 0, 1},{ 0, 1},{ 0, 1},{ 1, 1},{ 1, 1},{ 1, 1},{ 0, 2},{ 4, 0},{ 1, 2},{ 1, 2},{ 1, 2},{ 2, 2},
    { 2, 2},{ 2, 2},{ 1, 3},{ 5, 1},{ 2, 3},{ 2, 3},{ 0, 4},{ 3, 3},{ 3, 3},{ 3, 3},{ 2, 4},{ 2, 4},{ 2, 4},{ 5, 3},{ 1, 5},{ 1, 5},
    { 2, 5},{ 4, 4},{ 4, 4},{ 3, 5},{ 3, 5},{ 2, 6},{ 2, 6},{ 2, 6},{ 3, 6},{ 5, 5},{ 5, 5},{ 4, 6},{ 8, 4},{ 3, 7},{ 3, 7},{ 3, 7},
    { 6, 6},{ 6, 6},{ 6, 6},{ 5, 7},{ 9, 5},{ 6, 7},{ 6, 7},{ 4, 8},{ 7, 7},{ 7, 7},{ 7, 7},{ 6, 8},{ 6, 8},{ 6, 8},{ 9, 7},{ 5, 9},
    { 5, 9},{ 6, 9},{ 8, 8},{ 8, 8},{ 7, 9},{ 7, 9},{ 6,10},{ 6,10},{ 6,10},{ 7,10},{ 9, 9},{ 9, 9},{ 8,10},{12, 8},{ 7,11},{ 7,11},
    { 7,11},{10,10},{10,10},{10,10},{ 9,11},{13, 9},{10,11},{10,11},{ 8,12},{11,11},{11,11},{11,11},{10,12},{10,12},{10,12},{13,11},
    { 9,13},{ 9,13},{10,13},{12,12},{12,12},{11,13},{11,13},{10,14},{10,14},{10,14},{11,14},{13,13},{13,13},{12,14},{16,12},{11,15},
    {11,15},{11,15},{14,14},{14,14},{14,14},{13,15},{17,13},{14,15},{14,15},{12,16},{15,15},{15,15},{15,15},{14,16},{14,16},{14,16},
    {17,15},{13,17},{13,17},{14,17},{16,16},{16,16},{15,17},{15,17},{14,18},{14,18},{14,18},{15,18},{17,17},{17,17},{16,18},{20,16},
    {15,19},{15,19},{15,19},{18,18},{18,18},{18,18},{17,19},{21,17},{18,19},{18,19},{16,20},{19,19},{19,19},{19,19},{18,20},{18,20},
    {18,20},{21,19},{17,21},{17,21},{18,21},{20,20},{20,20},{19,21},{19,21},{18,22},{18,22},{18,22},{19,22},{21,21},{21,21},{20,22},
    {24,20},{19,23},{19,23},{19,23},{22,22},{22,22},{22,22},{21,23},{25,21},{22,23},{22,23},{20,24},{23,23},{23,23},{23,23},{22,24},
    {22,24},{22,24},{25,23},{21,25},{21,25},{22,25},{24,24},{24,24},{23,25},{23,25},{22,26},{22,26},{22,26},{23,26},{25,25},{25,25},
    {24,26},{28,24},{23,27},{23,27},{23,27},{26,26},{26,26},{26,26},{25,27},{29,25},{26,27},{26,27},{24,28},{27,27},{27,27},{27,27},
    {26,28},{26,28},{26,28},{29,27},{25,29},{25,29},{26,29},{28,28},{28,28},{27,29},{27,29},{26,30},{26,30},{26,30},{27,30},{29,29},
    {29,29},{28,30},{28,30},{27,31},{27,31},{27,31},{30,30},{30,30},{30,30},{29,31},{29,31},{30,31},{30,31},{30,31},{31,31},{31,31}};

CMP_STATIC CGU_UINT8 g_Match6Bit[256][2] = {
    { 0, 0},{ 1, 0},{ 0, 1},{ 1, 1},{ 1, 1},{ 0, 2},{ 1, 2},{ 2, 2},{ 2, 2},{ 1, 3},{ 0, 4},{ 3, 3},{ 3, 3},{ 0, 5},{ 1, 5},{ 4, 4},
    { 4, 4},{ 1, 6},{ 0, 7},{ 5, 5},{ 5, 5},{ 0, 8},{ 1, 8},{ 6, 6},{ 6, 6},{ 1, 9},{ 2, 9},{ 7, 7},{ 7, 7},{ 2,10},{ 3,10},{ 8, 8},
    { 8, 8},{ 3,11},{ 4,11},{ 9, 9},{ 9, 9},{ 4,12},{ 5,12},{10,10},{10,10},{ 5,13},{ 6,13},{16, 8},{11,11},{ 6,14},{ 7,14},{17, 9},
    {12,12},{ 7,15},{ 8,15},{16,11},{13,13},{10,15},{ 8,16},{ 9,16},{14,14},{13,15},{ 9,17},{10,17},{15,15},{16,15},{10,18},{11,18},
    {12,18},{16,16},{11,19},{12,19},{13,19},{17,17},{12,20},{13,20},{14,20},{18,18},{13,21},{14,21},{15,21},{19,19},{14,22},{15,22},
    {20,20},{20,20},{15,23},{16,23},{21,21},{21,21},{16,24},{17,24},{22,22},{22,22},{17,25},{18,25},{23,23},{23,23},{18,26},{19,26},
    {24,24},{24,24},{19,27},{20,27},{25,25},{25,25},{20,28},{21,28},{26,26},{26,26},{21,29},{22,29},{32,24},{27,27},{22,30},{23,30},
    {33,25},{28,28},{23,31},{24,31},{32,27},{29,29},{26,31},{24,32},{25,32},{30,30},{29,31},{25,33},{26,33},{31,31},{32,31},{26,34},
    {27,34},{28,34},{32,32},{27,35},{28,35},{29,35},{33,33},{28,36},{29,36},{30,36},{34,34},{29,37},{30,37},{31,37},{35,35},{30,38},
    {31,38},{36,36},{36,36},{31,39},{32,39},{37,37},{37,37},{32,40},{33,40},{38,38},{38,38},{33,41},{34,41},{39,39},{39,39},{34,42},
    {35,42},{40,40},{40,40},{35,43},{36,43},{41,41},{41,41},{36,44},{37,44},{42,42},{42,42},{37,45},{38,45},{48,40},{43,43},{38,46},
    {39,46},{49,41},{44,44},{39,47},{40,47},{48,43},{45,45},{42,47},{40,48},{41,48},{46,46},{45,47},{41,49},{42,49},{47,47},{48,47},
    {42,50},{43,50},{44,50},{48,48},{43,51},{44,51},{45,51},{49,49},{44,52},{45,52},{46,52},{50,50},{45,53},{46,53},{47,53},{51,51},
    {46,54},{47,54},{52,52},{52,52},{47,55},{48,55},{53,53},{53,53},{48,56},{49,56},{54,54},{54,54},{49,57},{50,57},{55,55},{55,55},
    {50,58},{51,58},{56,56},{56,56},{51,59},{52,59},{57,57},{57,57},{52,60},{53,60},{58,58},{58,58},{53,61},{54,61},{59,59},{59,59},
    {54,62},{55,62},{60,60},{60,60},{55,63},{56,63},{61,61},{61,61},{58,63},{59,63},{62,62},{62,62},{61,63},{62,63},{63,63},{63,63}};

CMP_STATIC CGU_Vec2ui cgu_solidColorBlock(CMP_IN CGU_UINT8 Red, CMP_IN CGU_UINT8 Green, CMP_IN CGU_UINT8 Blue)
 {
         CGU_UINT32 maxEndp16;
         CGU_UINT32 minEndp16;
 
         CGU_UINT32 mask = 0xAAAAAAAAu;
 
         minEndp16 = g_Match5Bit[Red][0] * 2048U + g_Match6Bit[Green][0] * 32U + g_Match5Bit[Blue][0];
         maxEndp16 = g_Match5Bit[Red][1] * 2048U + g_Match6Bit[Green][1] * 32U + g_Match5Bit[Blue][1];
 
         // write the color block
         if( maxEndp16 < minEndp16 )
         {
             CGU_UINT32 tmpValue = minEndp16;
             minEndp16 = maxEndp16;
             maxEndp16 = tmpValue;
             mask ^= 0x55555555u;
         }
 
         CGU_Vec2ui outputBytes;
         outputBytes.x = CGU_UINT32(maxEndp16) | (CGU_UINT32(minEndp16) << 16u);
         outputBytes.y = mask;
 
         return outputBytes;
 }

CMP_STATIC void cmp_get_encode_data(CMP_IN CMP_EncodeData CMP_REFINOUT edata, CMP_IN CMP_CONSTANT CGU_Vec4uc src_image[16])
{
    CMP_CONSTANT CGU_UINT32 fr = src_image[0].r, fg = src_image[0].g, fb = src_image[0].b;
    
    edata.all_colors_equal = false;

    edata.total.r = fr;
    edata.total.g = fg;
    edata.total.b = fb;
    edata.max.r   = fr;
    edata.max.g   = fg;
    edata.max.b   = fb;
    edata.min.r   = fr;
    edata.min.g   = fg;
    edata.min.b   = fb;

    edata.grayscale_flag   = (fr == fg) && (fr == fb);
    edata.any_black_pixels = (fr | fg | fb) < 4;

    for (CGU_UINT32 i = 1; i < 16; i++)
    {
        CMP_CONSTANT CGU_INT r = src_image[i].r, g = src_image[i].g, b = src_image[i].b;

        edata.grayscale_flag &= ((r == g) && (r == b));
        edata.any_black_pixels |= ((r | g | b) < 4);

        edata.max.r = CMP_MAX(edata.max.r, r);
        edata.max.g = CMP_MAX(edata.max.g, g);
        edata.max.b = CMP_MAX(edata.max.b, b);
        edata.min.r = CMP_MIN(edata.min.r, r);
        edata.min.g = CMP_MIN(edata.min.g, g);
        edata.min.b = CMP_MIN(edata.min.b, b);
        edata.total.r += r;
        edata.total.g += g;
        edata.total.b += b;
    }

    edata.avg.r = (edata.total.r + 8) >> 4;
    edata.avg.g = (edata.total.g + 8) >> 4;
    edata.avg.b = (edata.total.b + 8) >> 4;
}

#ifndef ASPM_GPU
/*------------------------------------------------------------------------------------------------
1 DIM ramp
------------------------------------------------------------------------------------------------*/
CMP_STATIC inline void cpu_BldClrRmp(CGU_FLOAT _Rmp[MAX_POINTS], CGU_FLOAT _InpRmp[NUM_ENDPOINTS], CGU_UINT32 dwNumPoints) 
{
    CGU_UINT32 dwRndAmount[9] = {0, 0, 0, 0, 1, 1, 2, 2, 3};

    // linear interpolate end points to get the ramp
    _Rmp[0] = _InpRmp[0];
    _Rmp[dwNumPoints - 1] = _InpRmp[1];
    if(dwNumPoints % 2)
        _Rmp[dwNumPoints] = 1000000.f; // for 3 point ramp; not to select the 4th point as min
    for(CGU_UINT32 e = 1; e < dwNumPoints - 1; e++)
        _Rmp[e] = floor((_Rmp[0] * (dwNumPoints - 1 - e) + _Rmp[dwNumPoints - 1] * e + dwRndAmount[dwNumPoints])/ (CGU_FLOAT)(dwNumPoints - 1));
}

/*------------------------------------------------------------------------------------------------
// build 3D ramp
------------------------------------------------------------------------------------------------*/
CMP_STATIC inline void cpu_BldRmp(CGU_FLOAT _Rmp[NUM_CHANNELS][MAX_POINTS], CGU_FLOAT _InpRmp[NUM_CHANNELS][NUM_ENDPOINTS],CGU_UINT32 dwNumPoints) {
    for(CGU_UINT32 j = 0; j < 3; j++)
        cpu_BldClrRmp(_Rmp[j], _InpRmp[j], dwNumPoints);
}

/*------------------------------------------------------------------------------------------------
// this is how the end points is going to be look like when decompressed
------------------------------------------------------------------------------------------------*/
CMP_STATIC inline void cpu_MkWkRmpPts(CMP_INOUT CGU_UINT8  CMP_REFINOUT _bEq, 
                                  CGU_FLOAT _OutRmpPts[NUM_CHANNELS][NUM_ENDPOINTS],
                                  CGU_FLOAT _InpRmpPts[NUM_CHANNELS][NUM_ENDPOINTS], 
                                  CGU_UINT8 nRedBits, 
                                  CGU_UINT8 nGreenBits, 
                                  CGU_UINT8 nBlueBits) 
{
    CGU_FLOAT Fctrs[3];
    Fctrs[RC] = (CGU_FLOAT)(1 << nRedBits);
    Fctrs[GC] = (CGU_FLOAT)(1 << nGreenBits);
    Fctrs[BC] = (CGU_FLOAT)(1 << nBlueBits);

    CGU_BOOL bEq = true;
    // find whether input ramp is flat
    for(CGU_UINT32 j = 0; j < 3; j++)
        bEq  &= (_InpRmpPts[j][0] == _InpRmpPts[j][1]);

    _bEq = bEq?1:0;

    // end points on the integer grid
    for(CGU_UINT32 j = 0; j <3; j++) {
        for(CGU_UINT32 k = 0; k <2; k++) {
            // Apply the lower bit replication to give full dynamic range
            _OutRmpPts[j][k] = _InpRmpPts[j][k] + floor(_InpRmpPts[j][k] / Fctrs[j]);
            _OutRmpPts[j][k] = cmp_max(_OutRmpPts[j][k], 0.f);
            _OutRmpPts[j][k] = cmp_min(_OutRmpPts[j][k], 255.f);
        }
    }
}

// Compute error and find DXTC indexes for the current cluster
CMP_STATIC CGU_FLOAT cpu_ClstrIntnl(CGU_FLOAT _Blk[BLOCK_SIZE_4X4][NUM_CHANNELS], 
                                 CGU_UINT8 pcIndices[BLOCK_SIZE_4X4], 
                                 CGU_FLOAT _Rmp[NUM_CHANNELS][MAX_POINTS], 
                                 int dwBlockSize, 
                                 CGU_UINT8 dwNumPoints,
                                 bool _ConstRamp, 
                                 CGU_FLOAT _pfWeights[3], 
                                 bool _bUseAlpha) 
{
    CGU_FLOAT Err = 0.f;
    CGU_UINT8 rmp_l = (_ConstRamp) ? 1 : dwNumPoints;

    // For each colour in the original block assign it
    // to the closest cluster and compute the cumulative error
    for(int i=0; i< dwBlockSize; i++) {
        if(_bUseAlpha && *((CGU_UINT32*) &_Blk[i][AC]) == 0)
            pcIndices[i] = dwNumPoints;
        else {
            CGU_FLOAT shortest = 99999999999.f;
            CGU_UINT8 shortestIndex = 0;
            CGU_UINT8 r;
            if ((_pfWeights[0] != 1.0f)||(_pfWeights[1] != 1.0f)||(_pfWeights[2] != 1.0f))
                for(r=0; r < rmp_l; r++) {
                    // calculate the distance for each component
                    CGU_FLOAT distance =     (_Blk[i][RC] - _Rmp[RC][r]) * (_Blk[i][RC] - _Rmp[RC][r]) * _pfWeights[0] +
                                             (_Blk[i][GC] - _Rmp[GC][r]) * (_Blk[i][GC] - _Rmp[GC][r]) * _pfWeights[1] +
                                             (_Blk[i][BC] - _Rmp[BC][r]) * (_Blk[i][BC] - _Rmp[BC][r]) * _pfWeights[2];

                    if(distance < shortest) {
                        shortest = distance;
                        shortestIndex = r;
                    }
                } else
                for(r=0; r < rmp_l; r++) {
                    // calculate the distance for each component
                    CGU_FLOAT distance =     (_Blk[i][RC] - _Rmp[RC][r]) * (_Blk[i][RC] - _Rmp[RC][r]) +
                                             (_Blk[i][GC] - _Rmp[GC][r]) * (_Blk[i][GC] - _Rmp[GC][r]) +
                                             (_Blk[i][BC] - _Rmp[BC][r]) * (_Blk[i][BC] - _Rmp[BC][r]);

                    if(distance < shortest) {
                        shortest = distance;
                        shortestIndex = r;
                    }
                }

            Err += shortest;

            // We have the index of the best cluster, so assign this in the block
            // Reorder indices to match correct DXTC ordering
            if(shortestIndex == dwNumPoints - 1)
                shortestIndex = 1;
            else if(shortestIndex)
                shortestIndex++;
            pcIndices[i] = shortestIndex;
        }
    }

    return Err;
}

/*------------------------------------------------------------------------------------------------
// input ramp is on the coarse grid
------------------------------------------------------------------------------------------------*/
CMP_STATIC CGU_FLOAT cpu_ClstrBas( CGU_UINT8 pcIndices[BLOCK_SIZE_4X4], 
                                CGU_FLOAT _Blk[BLOCK_SIZE_4X4][NUM_CHANNELS],
                                CGU_FLOAT _InpRmp[NUM_CHANNELS][NUM_ENDPOINTS], 
                                int dwBlockSize, 
                                CGU_UINT8 dwNumPoints, 
                                CGU_FLOAT _pfWeights[3],
                                bool _bUseAlpha, 
                                CGU_UINT8 nRedBits, 
                                CGU_UINT8 nGreenBits, 
                                CGU_UINT8 nBlueBits) 
{
    // make ramp endpoints the way they'll going to be decompressed
    CGU_UINT8  Eq = 1;
    CGU_FLOAT InpRmp[NUM_CHANNELS][NUM_ENDPOINTS];
    cpu_MkWkRmpPts(Eq, InpRmp, _InpRmp, nRedBits, nGreenBits, nBlueBits);

    // build ramp as it would be built by decompressor
    CGU_FLOAT Rmp[NUM_CHANNELS][MAX_POINTS];
    cpu_BldRmp(Rmp, InpRmp, dwNumPoints);

    // clusterize and find a cumulative error
    return cpu_ClstrIntnl(_Blk, pcIndices, Rmp, dwBlockSize, dwNumPoints, Eq,  _pfWeights, _bUseAlpha);
}

CMP_STATIC CGU_UINT8 nByteBitsMask2[9] = {0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

CMP_STATIC CGU_UINT32 cpu_ConstructColor2(CGU_UINT8 R, CGU_UINT8 nRedBits, CGU_UINT8 G, CGU_UINT8 nGreenBits, CGU_UINT8 B, CGU_UINT8 nBlueBits) {
    return (    ((R & nByteBitsMask2[nRedBits])    << (nGreenBits + nBlueBits - (PIX_GRID - nRedBits))) |
                ((G & nByteBitsMask2[nGreenBits])<< (nBlueBits - (PIX_GRID - nGreenBits))) |
                ((B & nByteBitsMask2[nBlueBits]) >> ((PIX_GRID - nBlueBits))));
}

CMP_STATIC CGU_FLOAT cpu_Clstr( CGU_UINT32 block_32[BLOCK_SIZE_4X4], 
                             CGU_UINT32 dwBlockSize,
                             CGU_UINT8  nEndpoints[3][NUM_ENDPOINTS],
                             CGU_UINT8  pcIndices[BLOCK_SIZE_4X4], 
                             CGU_UINT8  dwNumPoints,
                             CGU_FLOAT  _pfWeights[3], 
                             bool       _bUseAlpha, 
                             CGU_UINT8 _nAlphaThreshold,
                             CGU_UINT8 nRedBits, 
                             CGU_UINT8 nGreenBits, 
                             CGU_UINT8 nBlueBits) 
{
    CGU_UINT32 c0 = cpu_ConstructColor2(nEndpoints[RC][0], nRedBits, nEndpoints[GC][0], nGreenBits, nEndpoints[BC][0], nBlueBits);
    CGU_UINT32 c1 = cpu_ConstructColor2(nEndpoints[RC][1], nRedBits, nEndpoints[GC][1], nGreenBits, nEndpoints[BC][1], nBlueBits);
    CGU_UINT32 nEndpointIndex0 = 0;
    CGU_UINT32 nEndpointIndex1 = 1;
    if((!(dwNumPoints & 0x1) && c0 <= c1) || ((dwNumPoints & 0x1) && c0 > c1)) {
        nEndpointIndex0 = 1;
        nEndpointIndex1 = 0;
    }

    CGU_FLOAT InpRmp[NUM_CHANNELS][NUM_ENDPOINTS];
    InpRmp[RC][0] = (CGU_FLOAT)nEndpoints[RC][nEndpointIndex0];
    InpRmp[RC][1] = (CGU_FLOAT)nEndpoints[RC][nEndpointIndex1];
    InpRmp[GC][0] = (CGU_FLOAT)nEndpoints[GC][nEndpointIndex0];
    InpRmp[GC][1] = (CGU_FLOAT)nEndpoints[GC][nEndpointIndex1];
    InpRmp[BC][0] = (CGU_FLOAT)nEndpoints[BC][nEndpointIndex0];
    InpRmp[BC][1] = (CGU_FLOAT)nEndpoints[BC][nEndpointIndex1];

    CGU_UINT32 dwAlphaThreshold = _nAlphaThreshold << 24;
    CGU_FLOAT Blk[BLOCK_SIZE_4X4][NUM_CHANNELS];
    for(CGU_UINT32 i = 0; i < dwBlockSize; i++) {
        Blk[i][RC] = (CGU_FLOAT)((block_32[i] & 0xff0000) >> 16);
        Blk[i][GC] = (CGU_FLOAT)((block_32[i] & 0xff00) >> 8);
        Blk[i][BC] = (CGU_FLOAT)(block_32[i] & 0xff);
        if(_bUseAlpha)
            Blk[i][AC] = ((block_32[i] & 0xff000000) >= dwAlphaThreshold) ? 1.f : 0.f;
    }

    return cpu_ClstrBas(pcIndices, Blk, InpRmp, dwBlockSize, dwNumPoints, _pfWeights, _bUseAlpha, nRedBits, nGreenBits, nBlueBits);
}

/*------------------------------------------------------------------------------------------------
Compute cumulative error for the current cluster
------------------------------------------------------------------------------------------------*/
CMP_STATIC CGU_FLOAT cpu_ClstrErr(CGU_FLOAT _Blk[BLOCK_SIZE_4X4][NUM_CHANNELS], 
                              CGU_FLOAT _Rpt[BLOCK_SIZE_4X4],
                              CGU_FLOAT _Rmp[NUM_CHANNELS][MAX_POINTS], 
                              CGU_UINT32 _NmbClrs, 
                              CGU_UINT32 _blcktp,
                              bool _ConstRamp, 
                              CGU_Vec3f channelWeights) 
{
    CGU_FLOAT fError = 0.f;
    CGU_UINT32 rmp_l = (_ConstRamp) ? 1 : _blcktp;

    CGU_BOOL useWeights = ((channelWeights[0] != 1.0f) || (channelWeights[1] != 1.0f) || (channelWeights[2] != 1.0f));

    // For each colour in the original block, find the closest cluster
    // and compute the comulative error
    for(CGU_UINT32 i=0; i<_NmbClrs; i++) {
        CGU_FLOAT fShortest = 99999999999.f;

        if(useWeights)
            for(CGU_UINT32 r=0; r < rmp_l; r++) {
                // calculate the distance for each component
                CGU_FLOAT fDistance =    (_Blk[i][RC] - _Rmp[RC][r]) * (_Blk[i][RC] - _Rmp[RC][r])  * channelWeights[0] +
                                          (_Blk[i][GC] - _Rmp[GC][r]) * (_Blk[i][GC] - _Rmp[GC][r]) * channelWeights[1] +
                                          (_Blk[i][BC] - _Rmp[BC][r]) * (_Blk[i][BC] - _Rmp[BC][r]) * channelWeights[2];

                if(fDistance < fShortest)
                    fShortest = fDistance;
            } else
            for(CGU_UINT32 r=0; r < rmp_l; r++) {
                // calculate the distance for each component
                CGU_FLOAT fDistance =    (_Blk[i][RC] - _Rmp[RC][r]) * (_Blk[i][RC] - _Rmp[RC][r]) +
                                          (_Blk[i][GC] - _Rmp[GC][r]) * (_Blk[i][GC] - _Rmp[GC][r]) +
                                          (_Blk[i][BC] - _Rmp[BC][r]) * (_Blk[i][BC] - _Rmp[BC][r]);

                if(fDistance < fShortest)
                    fShortest = fDistance;
            }

        // accumulate the error
        fError += fShortest * _Rpt[i];
    }

    return fError;
}


#if defined(USE_REFINE3D)

CMP_STATIC CGU_FLOAT cmp_Refine3D(  CGU_FLOAT _OutRmpPnts[NUM_CHANNELS][NUM_ENDPOINTS],
                                CGU_FLOAT  _InpRmpPnts[NUM_CHANNELS][NUM_ENDPOINTS],
                                CGU_FLOAT  _Blk[BLOCK_SIZE_4X4][NUM_CHANNELS], 
                                CGU_FLOAT  _Rpt[BLOCK_SIZE_4X4],
                                CGU_UINT32 _NmrClrs, 
                                CGU_UINT32 dwNumPoints, 
                                CGU_Vec3f channelWeights,
                                CGU_UINT8 nRedBits, 
                                CGU_UINT8 nGreenBits, 
                                CGU_UINT8 nBlueBits, 
                                CGU_UINT32 nRefineSteps) 
{
    ALIGN_16 CGU_FLOAT Rmp[NUM_CHANNELS][MAX_POINTS];

    CGU_FLOAT Blk[BLOCK_SIZE_4X4][NUM_CHANNELS];
    for(CGU_UINT32 i = 0; i < _NmrClrs; i++)
        for(CGU_UINT32 j = 0; j < 3; j++)
            Blk[i][j] = _Blk[i][j];

    CGU_FLOAT fWeightRed    = channelWeights.r;
    CGU_FLOAT fWeightGreen  = channelWeights.g;
    CGU_FLOAT fWeightBlue   = channelWeights.b;

    // here is our grid
    CGU_FLOAT Fctrs[3];
    Fctrs[RC] = (CGU_FLOAT)(1 << (PIX_GRID-nRedBits));
    Fctrs[GC] = (CGU_FLOAT)(1 << (PIX_GRID-nGreenBits));
    Fctrs[BC] = (CGU_FLOAT)(1 << (PIX_GRID-nBlueBits));

    CGU_FLOAT InpRmp0[NUM_CHANNELS][NUM_ENDPOINTS];
    CGU_FLOAT InpRmp[NUM_CHANNELS][NUM_ENDPOINTS];
    for(CGU_UINT32 k = 0; k < 2; k++)
        for(CGU_UINT32 j = 0; j < 3; j++)
            InpRmp0[j][k] = InpRmp[j][k] = _OutRmpPnts[j][k] = _InpRmpPnts[j][k];

    // make ramp endpoints the way they'll going to be decompressed
    // plus check whether the ramp is flat
    CGU_UINT8 Eq;
    CGU_FLOAT WkRmpPts[NUM_CHANNELS][NUM_ENDPOINTS];
    cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);

    // build ramp for all 3 colors
    cpu_BldRmp(Rmp, WkRmpPts, dwNumPoints);

    // clusterize for the current ramp
    CGU_FLOAT bestE = cpu_ClstrErr(Blk, _Rpt, Rmp, _NmrClrs, dwNumPoints, Eq, channelWeights);
    if(bestE == 0.f)    // if exact, we've done
        return bestE;

    // Jitter endpoints in each direction
    CGU_INT nRefineStart = 0 - (cmp_min(nRefineSteps, (CGU_UINT8)8));
    CGU_INT nRefineEnd   = cmp_min(nRefineSteps, (CGU_UINT8)8);
    for(CGU_INT nJitterG0 = nRefineStart; nJitterG0 <= nRefineEnd; nJitterG0++) {
        InpRmp[GC][0] = cmp_min(cmp_max(InpRmp0[GC][0] + nJitterG0 * Fctrs[GC], 0.f), 255.f);
        for(CGU_INT nJitterG1 = nRefineStart; nJitterG1 <= nRefineEnd; nJitterG1++) {
            InpRmp[GC][1] = cmp_min(cmp_max(InpRmp0[GC][1] + nJitterG1 * Fctrs[GC], 0.f), 255.f);
            cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);
            cpu_BldClrRmp(Rmp[GC], WkRmpPts[GC], dwNumPoints);

            CGU_FLOAT RmpErrG[MAX_POINTS][BLOCK_SIZE_4X4];
            for(CGU_UINT32 i=0; i < _NmrClrs; i++) {
                for(CGU_UINT32 r = 0; r < dwNumPoints; r++) {
                    CGU_FLOAT DistG = (Rmp[GC][r] - Blk[i][GC]);
                    RmpErrG[r][i] = DistG * DistG * fWeightGreen;
                }
            }

            for(CGU_INT nJitterB0 = nRefineStart; nJitterB0 <= nRefineEnd; nJitterB0++) {
                InpRmp[BC][0] = cmp_min(cmp_max(InpRmp0[BC][0] + nJitterB0 * Fctrs[BC], 0.f), 255.f);
                for(CGU_INT nJitterB1 = nRefineStart; nJitterB1 <= nRefineEnd; nJitterB1++) {
                    InpRmp[BC][1] = cmp_min(cmp_max(InpRmp0[BC][1] + nJitterB1 * Fctrs[BC], 0.f), 255.f);
                    cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);
                    cpu_BldClrRmp(Rmp[BC], WkRmpPts[BC], dwNumPoints);

                    CGU_FLOAT RmpErr[MAX_POINTS][BLOCK_SIZE_4X4];
                    for(CGU_UINT32 i=0; i < _NmrClrs; i++) {
                        for(CGU_UINT32 r = 0; r < dwNumPoints; r++) {
                            CGU_FLOAT DistB = (Rmp[BC][r] - Blk[i][BC]);
                            RmpErr[r][i] = RmpErrG[r][i] + DistB * DistB * fWeightBlue;
                        }
                    }

                    for(CGU_INT nJitterR0 = nRefineStart; nJitterR0 <= nRefineEnd; nJitterR0++) {
                        InpRmp[RC][0] = cmp_min(cmp_max(InpRmp0[RC][0] + nJitterR0 * Fctrs[RC], 0.f), 255.f);
                        for(CGU_INT nJitterR1 = nRefineStart; nJitterR1 <= nRefineEnd; nJitterR1++) {
                            InpRmp[RC][1] = cmp_min(cmp_max(InpRmp0[RC][1] + nJitterR1 * Fctrs[RC], 0.f), 255.f);
                            cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);
                            cpu_BldClrRmp(Rmp[RC], WkRmpPts[RC], dwNumPoints);

                            // compute cumulative error
                            CGU_FLOAT mse = 0.f;
                            CGU_INT rmp_l = (Eq > 0) ? 1 : dwNumPoints;
                            for(CGU_UINT32 k = 0; k < _NmrClrs; k++) {
                                CGU_FLOAT MinErr = 10000000.f;
                                for(CGU_INT r = 0; r < rmp_l; r++) {
                                    CGU_FLOAT Dist = (Rmp[RC][r] - Blk[k][RC]);
                                    CGU_FLOAT Err = RmpErr[r][k] + Dist * Dist * fWeightRed;
                                    MinErr          = cmp_min(MinErr, Err);
                                }
                                mse += MinErr * _Rpt[k];
                            }

                            // save if we achieve better result
                            if(mse < bestE) {
                                bestE = mse;
                                for(CGU_UINT32 k = 0; k < 2; k++)
                                    for(CGU_UINT32 j = 0; j < 3; j++)
                                        _OutRmpPnts[j][k] = InpRmp[j][k];
                            }
                        }
                    }
                }
            }
        }
    }

    return bestE;
}
#endif

#if defined(USE_REFINE) 
CMP_STATIC CGU_FLOAT cmp_Refine(CGU_FLOAT _OutRmpPnts[NUM_CHANNELS][NUM_ENDPOINTS],
                  CGU_FLOAT  _InpRmpPnts[NUM_CHANNELS][NUM_ENDPOINTS],
                  CGU_FLOAT  _Blk[BLOCK_SIZE_4X4][NUM_CHANNELS], 
                  CGU_FLOAT  _Rpt[BLOCK_SIZE_4X4],
                  CGU_INT    _NmrClrs, 
                  CGU_UINT8  dwNumPoints, 
                  CGU_Vec3f  channelWeights,
                  CGU_UINT32 nRedBits, 
                  CGU_UINT32 nGreenBits, 
                  CGU_UINT32 nBlueBits, 
                  CGU_UINT32 nRefineSteps )
{
    ALIGN_16 CGU_FLOAT Rmp[NUM_CHANNELS][MAX_POINTS];

    if (nRefineSteps == 0) nRefineSteps = 1;


    CGU_FLOAT Blk[BLOCK_SIZE_4X4][NUM_CHANNELS];
    for(CGU_INT i = 0; i < _NmrClrs; i++)
        for(CGU_INT j = 0; j < 3; j++)
            Blk[i][j] = _Blk[i][j];

    CGU_FLOAT fWeightRed    = channelWeights.r;
    CGU_FLOAT fWeightGreen  = channelWeights.g;
    CGU_FLOAT fWeightBlue   = channelWeights.b;

    // here is our grid
    CGU_FLOAT Fctrs[3];
    Fctrs[RC] = (CGU_FLOAT)(1 << (PIX_GRID-nRedBits));
    Fctrs[GC] = (CGU_FLOAT)(1 << (PIX_GRID-nGreenBits));
    Fctrs[BC] = (CGU_FLOAT)(1 << (PIX_GRID-nBlueBits));

    CGU_FLOAT InpRmp0[NUM_CHANNELS][NUM_ENDPOINTS];
    CGU_FLOAT InpRmp[NUM_CHANNELS][NUM_ENDPOINTS];
    for(CGU_INT k = 0; k < 2; k++)
        for(CGU_INT j = 0; j < 3; j++)
            InpRmp0[j][k] = InpRmp[j][k] = _OutRmpPnts[j][k] = _InpRmpPnts[j][k];

    // make ramp endpoints the way they'll going to be decompressed
    // plus check whether the ramp is flat
    CGU_UINT8 Eq;
    CGU_FLOAT WkRmpPts[NUM_CHANNELS][NUM_ENDPOINTS];
    cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);

    // build ramp for all 3 colors
    cpu_BldRmp(Rmp, WkRmpPts, dwNumPoints);

    // clusterize for the current ramp
    CGU_FLOAT bestE = cpu_ClstrErr(Blk, _Rpt, Rmp, _NmrClrs, dwNumPoints, Eq, channelWeights);
    if(bestE == 0.f) //  || !nRefineSteps)    // if exact, we've done
        return bestE;

    // Tweak each component in isolation and get the best values

    // precompute ramp errors for Green and Blue
    CGU_FLOAT RmpErr[MAX_POINTS][BLOCK_SIZE_4X4];
    for(CGU_INT i=0; i < _NmrClrs; i++) {
        for(CGU_INT r = 0; r < dwNumPoints; r++) {
            CGU_FLOAT DistG = (Rmp[GC][r] - Blk[i][GC]);
            CGU_FLOAT DistB = (Rmp[BC][r] - Blk[i][BC]);
            RmpErr[r][i] = DistG * DistG * fWeightGreen + DistB * DistB * fWeightBlue;
        }
    }

    // First Red
    CGU_FLOAT bstC0 = InpRmp0[RC][0];
    CGU_FLOAT bstC1 = InpRmp0[RC][1];
    CGU_INT nRefineStart =  0 - (cmp_min(nRefineSteps, (CGU_UINT8)8));
    CGU_INT nRefineEnd   =  cmp_min(nRefineSteps, (CGU_UINT8)8);
    for(CGU_INT i = nRefineStart; i <= nRefineEnd; i++) {
        for(CGU_INT j = nRefineStart; j <= nRefineEnd; j++) {
            // make a move; both sides of interval.
            InpRmp[RC][0] = cmp_min(cmp_max(InpRmp0[RC][0] + i * Fctrs[RC], 0.f), 255.f);
            InpRmp[RC][1] = cmp_min(cmp_max(InpRmp0[RC][1] + j * Fctrs[RC], 0.f), 255.f);

            // make ramp endpoints the way they'll going to be decompressed
            // plus check whether the ramp is flat
            cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);

            // build ramp only for red
            cpu_BldClrRmp(Rmp[RC], WkRmpPts[RC], dwNumPoints);

            // compute cumulative error
            CGU_FLOAT mse = 0.f;
            CGU_INT rmp_l = (Eq > 0) ? 1 : dwNumPoints;
            for(CGU_INT k = 0; k < _NmrClrs; k++) {
                CGU_FLOAT MinErr = 10000000.f;
                for(CGU_INT r = 0; r < rmp_l; r++) {
                    CGU_FLOAT Dist = (Rmp[RC][r] - Blk[k][RC]);
                    CGU_FLOAT Err = RmpErr[r][k] + Dist * Dist * fWeightRed;
                    MinErr = cmp_minf(MinErr, Err);
                }
                mse += MinErr * _Rpt[k];
            }

            // save if we achieve better result
            if(mse < bestE) {
                bstC0 = InpRmp[RC][0];
                bstC1 = InpRmp[RC][1];
                bestE = mse;
            }
        }
    }

    // our best REDs
    InpRmp[RC][0] = bstC0;
    InpRmp[RC][1] = bstC1;

    // make ramp endpoints the way they'll going to be decompressed
    // plus check whether the ramp is flat
    cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);

    // build ramp only for green
    cpu_BldRmp(Rmp, WkRmpPts, dwNumPoints);

    // precompute ramp errors for Red and Blue
    for(CGU_INT i=0; i < _NmrClrs; i++) {
        for(CGU_INT r = 0; r < dwNumPoints; r++) {
            CGU_FLOAT DistR = (Rmp[RC][r] - Blk[i][RC]);
            CGU_FLOAT DistB = (Rmp[BC][r] - Blk[i][BC]);
            RmpErr[r][i] = DistR * DistR * fWeightRed + DistB * DistB * fWeightBlue;
        }
    }

    // Now green
    bstC0 = InpRmp0[GC][0];
    bstC1 = InpRmp0[GC][1];
    for(CGU_INT i = nRefineStart; i <= nRefineEnd; i++) {
        for(CGU_INT j = nRefineStart; j <= nRefineEnd; j++) {
            InpRmp[GC][0] = cmp_minf(cmp_maxf(InpRmp0[GC][0] + i * Fctrs[GC], 0.f), 255.f);
            InpRmp[GC][1] = cmp_minf(cmp_maxf(InpRmp0[GC][1] + j * Fctrs[GC], 0.f), 255.f);

            cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);
            cpu_BldClrRmp(Rmp[GC], WkRmpPts[GC], dwNumPoints);

            CGU_FLOAT mse = 0.f;
            CGU_INT rmp_l = (Eq > 0) ? 1 : dwNumPoints;
            for(CGU_INT k = 0; k < _NmrClrs; k++) {
                CGU_FLOAT MinErr = 10000000.f;
                for(CGU_INT r = 0; r < rmp_l; r++) {
                    CGU_FLOAT Dist = (Rmp[GC][r] - Blk[k][GC]);
                    CGU_FLOAT Err = RmpErr[r][k] +  Dist * Dist * fWeightGreen;
                    MinErr = cmp_minf(MinErr, Err);
                }
                mse += MinErr * _Rpt[k];
            }

            if(mse < bestE) {
                bstC0 = InpRmp[GC][0];
                bstC1 = InpRmp[GC][1];
                bestE = mse;
            }
        }
    }

    // our best GREENs
    InpRmp[GC][0] = bstC0;
    InpRmp[GC][1] = bstC1;

    cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);
    cpu_BldRmp(Rmp, WkRmpPts, dwNumPoints);

    // ramp err for Red and Green
    for(CGU_INT i=0; i < _NmrClrs; i++) {
        for(CGU_INT r = 0; r < dwNumPoints; r++) {
            CGU_FLOAT DistR = (Rmp[RC][r] - Blk[i][RC]);
            CGU_FLOAT DistG = (Rmp[GC][r] - Blk[i][GC]);
            RmpErr[r][i] = DistR * DistR * fWeightRed + DistG * DistG * fWeightGreen;
        }
    }

    bstC0 = InpRmp0[BC][0];
    bstC1 = InpRmp0[BC][1];
    // Now blue
    for(CGU_INT i = nRefineStart; i <= nRefineEnd; i++) {
        for(CGU_INT j = nRefineStart; j <= nRefineEnd; j++) {
            InpRmp[BC][0] = min(max(InpRmp0[BC][0] + i * Fctrs[BC], 0.f), 255.f);
            InpRmp[BC][1] = min(max(InpRmp0[BC][1] + j * Fctrs[BC], 0.f), 255.f);

            cpu_MkWkRmpPts(Eq, WkRmpPts, InpRmp, nRedBits, nGreenBits, nBlueBits);
            cpu_BldClrRmp(Rmp[BC], WkRmpPts[BC], dwNumPoints);

            CGU_FLOAT mse = 0.f;
            CGU_INT rmp_l = (Eq > 0) ? 1 : dwNumPoints;
            for(CGU_INT k = 0; k < _NmrClrs; k++) {
                CGU_FLOAT MinErr = 10000000.f;
                for(CGU_INT r = 0; r < rmp_l; r++) {
                    CGU_FLOAT Dist = (Rmp[BC][r] - Blk[k][BC]);
                    CGU_FLOAT Err = RmpErr[r][k] +  Dist * Dist * fWeightBlue;
                    MinErr = min(MinErr, Err);
                }
                mse += MinErr * _Rpt[k];
            }

            if(mse < bestE) {
                bstC0 = InpRmp[BC][0];
                bstC1 = InpRmp[BC][1];
                bestE = mse;
            }
        }
    }

    // our best BLUEs
    InpRmp[BC][0] = bstC0;
    InpRmp[BC][1] = bstC1;

    // return our best choice
    for(CGU_INT j = 0; j < 3; j++)
        for(CGU_INT k = 0; k < 2; k++)
            _OutRmpPnts[j][k] = InpRmp[j][k];

    return bestE;
}

#endif


//======================================================================================
// Codec from CompressonatorLib
//======================================================================================
#define BLOCK_SIZE_4X4          16
#define RG                      5
#define GG                      6
#define BG                      5

/*------------------------------------------------------------------------------------------------
// this is how the end points is going to be rounded in compressed format
------------------------------------------------------------------------------------------------*/
CMP_STATIC void cpu_MkRmpOnGrid(CGU_FLOAT _RmpF[NUM_CHANNELS][NUM_ENDPOINTS], 
                                CGU_FLOAT _MnMx[NUM_CHANNELS][NUM_ENDPOINTS],
                                CGU_FLOAT _Min, 
                                CGU_FLOAT _Max, 
                                CGU_UINT8 nRedBits, 
                                CGU_UINT8 nGreenBits, 
                                CGU_UINT8 nBlueBits)
{
    CGU_FLOAT Fctrs0[3];
    CGU_FLOAT Fctrs1[3];

    Fctrs1[RC] = (CGU_FLOAT)(1 << nRedBits);
    Fctrs1[GC] = (CGU_FLOAT)(1 << nGreenBits);
    Fctrs1[BC] = (CGU_FLOAT)(1 << nBlueBits);
    Fctrs0[RC] = (CGU_FLOAT)(1 << (PIX_GRID-nRedBits));
    Fctrs0[GC] = (CGU_FLOAT)(1 << (PIX_GRID-nGreenBits));
    Fctrs0[BC] = (CGU_FLOAT)(1 << (PIX_GRID-nBlueBits));

    for(int j = 0; j < 3; j++) {
        for(int k = 0; k < 2; k++) {
            _RmpF[j][k] = floor(_MnMx[j][k]);
            if(_RmpF[j][k] <= _Min)
                _RmpF[j][k] = _Min;
            else {
                _RmpF[j][k] += floor(128.f / Fctrs1[j]) - floor(_RmpF[j][k] / Fctrs1[j]);
                _RmpF[j][k] = cmp_minf(_RmpF[j][k], _Max);
            }

            _RmpF[j][k] = floor(_RmpF[j][k] / Fctrs0[j]) * Fctrs0[j];
        }
    }
}


// Find the first approximation of the line
// Assume there is a linear relation
//   Z = a * X_In
//   Z = b * Y_In
// Find a,b to minimize MSE between Z and Z_In
CMP_STATIC void cpu_FindAxis(CMP_OUT    CGU_FLOAT BlkSh[BLOCK_SIZE_4X4][NUM_CHANNELS],
                             CMP_IN     CGU_FLOAT LineDir0[NUM_CHANNELS],
                             CMP_IN     CGU_FLOAT fBlockCenter[NUM_CHANNELS], 
                             CMP_OUT    CGU_UINT8 CMP_REFINOUT AxisIsSmall, 
                             CMP_IN     CGU_FLOAT  BlkUV[BLOCK_SIZE_4X4][NUM_CHANNELS],
                             CMP_IN     CGU_FLOAT _inpRpt[BLOCK_SIZE_4X4], 
                             CMP_IN     int       nDimensions, 
                             CMP_IN     int       dwUniqueColors) 
{
    CGU_FLOAT Crrl[NUM_CHANNELS];
    CGU_FLOAT RGB2[NUM_CHANNELS];
    CGU_INT   i;

    LineDir0[0] = LineDir0[1] = LineDir0[2] = RGB2[0] = RGB2[1] = RGB2[2] =
    Crrl[0] = Crrl[1] = Crrl[2] = fBlockCenter[0] = fBlockCenter[1] = fBlockCenter[2] = 0.f;

    // sum position of all points
    CGU_FLOAT fNumPoints = 0.f;
    for(i=0; i < dwUniqueColors; i++) {
        fBlockCenter[0] += BlkUV[i][0] * _inpRpt[i];
        fBlockCenter[1] += BlkUV[i][1] * _inpRpt[i];
        fBlockCenter[2] += BlkUV[i][2] * _inpRpt[i];
        fNumPoints += _inpRpt[i];
    }

    // and then average to calculate center coordinate of block
    fBlockCenter[0] /= fNumPoints;
    fBlockCenter[1] /= fNumPoints;
    fBlockCenter[2] /= fNumPoints;

    for(i = 0; i < dwUniqueColors; i++) {
        // calculate output block as offsets around block center
        BlkSh[i][0] = BlkUV[i][0] - fBlockCenter[0];
        BlkSh[i][1] = BlkUV[i][1] - fBlockCenter[1];
        BlkSh[i][2] = BlkUV[i][2] - fBlockCenter[2];

        // compute correlation matrix
        // RGB2 = sum of ((distance from point from center) squared)
        // Crrl = ???????. Seems to be be some calculation based on distance from point center in two dimensions
        for(int j = 0; j < nDimensions; j++) {
            RGB2[j] += BlkSh[i][j] * BlkSh[i][j] * _inpRpt[i];
            Crrl[j] += BlkSh[i][j] * BlkSh[i][(j+1)%3] * _inpRpt[i];
        }
    }

    // if set's diameter is small
    int i0 = 0, i1 = 1;
    CGU_FLOAT mxRGB2 = 0.f;
    int k = 0, j = 0;
    CGU_FLOAT fEPS = fNumPoints * EPS;
    for(k = 0, j = 0; j < 3; j++) {
        if(RGB2[j] >= fEPS)
            k++;
        else
            RGB2[j] = 0.f;

        if(mxRGB2 < RGB2[j]) {
            mxRGB2 = RGB2[j];
            i0 = j;
        }
    }

    CGU_FLOAT fEPS2 = fNumPoints * EPS2;
    AxisIsSmall = 1;
    for(j = 0; j < 3; j++)
    {
        AxisIsSmall &= (RGB2[j] < fEPS2);
    }

    if(AxisIsSmall) // all are very small to avoid division on the small determinant
        return;

    if(k == 1) // really only 1 dimension
        LineDir0[i0]= 1.;
    else if(k == 2) { // really only 2 dimensions
        i1 = (RGB2[(i0+1)%3] > 0.f) ? (i0+1)%3 : (i0+2)%3;
        CGU_FLOAT Crl = (i1 == (i0+1)%3) ? Crrl[i0] : Crrl[(i0+2)%3];
        LineDir0[i1] = Crl/ RGB2[i0];
        LineDir0[i0]= 1.;
    } else {
        CGU_FLOAT maxDet = 100000.f;
        CGU_FLOAT Cs[3];
        // select max det for precision
        for(j = 0; j < nDimensions; j++) {
            CGU_FLOAT Det = RGB2[j] * RGB2[(j+1)%3] - Crrl[j] * Crrl[j];
            Cs[j] = abs(Crrl[j]/sqrt(RGB2[j] * RGB2[(j+1)%3]));
            if(maxDet < Det) {
                maxDet = Det;
                i0 = j;
            }
        }

        // inverse correl matrix
        //  --      --       --      --
        //  |  A   B |       |  C  -B |
        //  |  B   C |  =>   | -B   A |
        //  --      --       --     --
        CGU_FLOAT mtrx1[2][2];
        CGU_FLOAT vc1[2];
        CGU_FLOAT vc[2];
        vc1[0] = Crrl[(i0 + 2) %3];
        vc1[1] = Crrl[(i0 + 1) %3];
        // C
        mtrx1[0][0] = RGB2[(i0+1)%3];
        // A
        mtrx1[1][1] = RGB2[i0];
        // -B
        mtrx1[1][0] = mtrx1[0][1] = -Crrl[i0];
        // find a solution
        vc[0] = mtrx1[0][0] * vc1[0] + mtrx1[0][1] * vc1[1];
        vc[1] = mtrx1[1][0] * vc1[0] + mtrx1[1][1] * vc1[1];
        // normalize
        vc[0] /= maxDet;
        vc[1] /= maxDet;
        // find a line direction vector
        LineDir0[i0] = 1.;
        LineDir0[(i0 + 1) %3] = 1.;
        LineDir0[(i0 + 2) %3] = vc[0] + vc[1];
    }

    // normalize direction vector
    CGU_FLOAT Len = LineDir0[0] * LineDir0[0] + LineDir0[1] * LineDir0[1] + LineDir0[2] * LineDir0[2];
    Len = sqrt(Len);

    for(j = 0; j < 3; j++)
        LineDir0[j] = (Len > 0.f) ? LineDir0[j] / Len : 0.f;
}

CMP_STATIC CGU_FLOAT cpu_RampSrchW( CGU_FLOAT Prj[BLOCK_SIZE_4X4],
                                    CGU_FLOAT PrjErr[BLOCK_SIZE_4X4],
                                    CGU_FLOAT PreMRep[BLOCK_SIZE_4X4],
                                    CGU_FLOAT StepErr, 
                                    CGU_FLOAT lowPosStep, 
                                    CGU_FLOAT highPosStep,
                                    int dwUniqueColors,
                                    int dwNumPoints )
{
    CGU_FLOAT error = 0;
    CGU_FLOAT step = (highPosStep - lowPosStep)/(dwNumPoints - 1);
    CGU_FLOAT step_h = step * (CGU_FLOAT)0.5;
    CGU_FLOAT rstep = (CGU_FLOAT)1.0f / step;
    CGU_INT   i;

    for(i=0; i < dwUniqueColors; i++) {
        CGU_FLOAT v;
        // Work out which value in the block this select
        CGU_FLOAT del;

        if((del = Prj[i] - lowPosStep) <= 0)
            v = lowPosStep;
        else if(Prj[i] -  highPosStep >= 0)
            v = highPosStep;
        else
            v = floor((del + step_h) * rstep) * step + lowPosStep;

        // And accumulate the error
        CGU_FLOAT d = (Prj[i] - v);
        d *= d;
        CGU_FLOAT err = PreMRep[i] * d + PrjErr[i];
        error += err;
        if(StepErr < error) {
            error  = StepErr;
            break;
        }
    }
    return error;
}


//    This is a float point-based compression
//    it assumes that the number of unique colors is already known; input is in [0., 255.] range.
//    This is C version.
CMP_STATIC bool cpu_CompressRGBBlockX(  CMP_OUT CGU_FLOAT  _RsltRmpPnts[NUM_CHANNELS][NUM_ENDPOINTS],
                                        CMP_IN  CGU_FLOAT  src_image[BLOCK_SIZE_4X4][NUM_CHANNELS],
                                        CMP_IN  CGU_FLOAT  Rpt[BLOCK_SIZE_4X4],
                                        CMP_IN  int        dwUniqueColors,
                                        CMP_IN  CGU_UINT8  dwNumPoints, 
                                        CMP_IN  bool       b3DRefinement, 
                                        CMP_IN  CGU_UINT8   nRefinementSteps,
                                        CMP_IN  CGU_FLOAT  pfWeights[3],
                                        CMP_IN  CGU_UINT8  nRedBits, 
                                        CMP_IN  CGU_UINT8  nGreenBits, 
                                        CMP_IN  CGU_UINT8  nBlueBits,
                                        CMP_IN  CGU_FLOAT  fquality )
{
    ALIGN_16 CGU_FLOAT Prj0[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT Prj[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT PrjErr[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT LineDir[NUM_CHANNELS];
    ALIGN_16 CGU_FLOAT RmpIndxs[BLOCK_SIZE_4X4];

    CMP_UNUSED(fquality);
    CMP_UNUSED(b3DRefinement)

    CGU_FLOAT LineDirG[NUM_CHANNELS];
    CGU_FLOAT PosG[NUM_ENDPOINTS];
    CGU_FLOAT BlkUV[BLOCK_SIZE_4X4][NUM_CHANNELS];
    CGU_FLOAT BlkSh[BLOCK_SIZE_4X4][NUM_CHANNELS];
    CGU_FLOAT LineDir0[NUM_CHANNELS];
    CGU_FLOAT Mdl[NUM_CHANNELS];

    CGU_FLOAT rsltC[NUM_CHANNELS][NUM_ENDPOINTS];
    int i, j, k;

    // down to [0., 1.]
    for(i = 0; i < dwUniqueColors; i++)
        for(j = 0; j < 3; j++)
            BlkUV[i][j] = src_image[i][j] / 255.f;

    bool isDONE         = false;

    // as usual if not more then 2 different colors, we've done
    if(dwUniqueColors <= 2) {
        for(j = 0; j < 3; j++) {
            rsltC[j][0] = src_image[0][j];
            rsltC[j][1] = src_image[dwUniqueColors - 1][j];
        }
        isDONE = true;
    }

    if ( !isDONE ) {
        //    This is our first attempt to find an axis we will go along.
        //    The cumulation is done to find a line minimizing the MSE from the input 3D points.
        CGU_UINT8 bSmall;
        cpu_FindAxis(BlkSh, LineDir0, Mdl, bSmall, BlkUV, Rpt, 3, dwUniqueColors);

        //    While trying to find the axis we found that the diameter of the input set is quite small.
        //    Do not bother.
        if(bSmall) {
            for(j = 0; j < 3; j++) {
                rsltC[j][0] = src_image[0][j];
                rsltC[j][1] = src_image[dwUniqueColors - 1][j];
            }
            isDONE = true;
        }
    }

    // GCC is being an awful being when it comes to goto-jumps.
    // So please bear with this.
    if ( !isDONE ) {
        CGU_FLOAT ErrG = 10000000.f;
        CGU_FLOAT PrjBnd[NUM_ENDPOINTS];
        ALIGN_16 CGU_FLOAT PreMRep[BLOCK_SIZE_4X4];
        for(j =0; j < 3; j++)
            LineDir[j] = LineDir0[j];

       //    Here is the main loop.
        //    1. Project input set on the axis in consideration.
        //    2. Run 1 dimensional search (see scalar case) to find an (sub) optimal pair of end points.
        //    3. Compute the vector of indexes (or clusters) for the current approximate ramp.
        //    4. Present our color channels as 3 16DIM vectors.
        //    5. Find closest approximation of each of 16DIM color vector with the projection of the 16DIM index vector.
        //    6. Plug the projections as a new directional vector for the axis.
        //    7. Goto 1.
        //    D - is 16 dim "index" vector (or 16 DIM vector of indexes - {0, 1/3, 2/3, 0, ...,}, but shifted and normalized).
        //    Ci - is a 16 dim vector of color i.
        //    for each Ci find a scalar Ai such that
        //    (Ai * D - Ci) (Ai * D - Ci) -> min , i.e distance between vector AiD and C is min.
        //    You can think of D as a unit interval(vector) "clusterizer",
        //    and Ai is a scale you need to apply to the clusterizer to
        //    approximate the Ci vector instead of the unit vector.
        //    Solution is
        //    Ai = (D . Ci) / (D . D); . - is a dot product.
        //    in 3 dim space Ai(s) represent a line direction, along which
        //    we again try to find (sub)optimal quantizer.
        
        //    That's what our for(;;) loop is about.
        for(;;) {
            //  1. Project input set on the axis in consideration.
            // From Foley & Van Dam: Closest point of approach of a line (P + v) to a point (R) is
            //                            P + ((R-P).v) / (v.v))v
            // The distance along v is therefore (R-P).v / (v.v)
            // (v.v) is 1 if v is a unit vector.
            //
            PrjBnd[0] =  1000.;
            PrjBnd[1] = -1000.;
            for(i = 0; i < BLOCK_SIZE_4X4; i++)
                Prj0[i] = Prj[i] = PrjErr[i] = PreMRep[i] = 0.f;

            for(i = 0; i < dwUniqueColors; i++) {
                Prj0[i] = Prj[i] = BlkSh[i][0] * LineDir[0] + BlkSh[i][1] * LineDir[1] + BlkSh[i][2] * LineDir[2];

                PrjErr[i] = (BlkSh[i][0] - LineDir[0] * Prj[i]) * (BlkSh[i][0] - LineDir[0] * Prj[i])
                            + (BlkSh[i][1] - LineDir[1] * Prj[i]) * (BlkSh[i][1] - LineDir[1] * Prj[i])
                            + (BlkSh[i][2] - LineDir[2] * Prj[i]) * (BlkSh[i][2] - LineDir[2] * Prj[i]);

                PrjBnd[0] = min(PrjBnd[0], Prj[i]);
                PrjBnd[1] = max(PrjBnd[1], Prj[i]);
            }

            //  2. Run 1 dimensional search (see scalar case) to find an (sub) optimal pair of end points.

            // min and max of the search interval
            CGU_FLOAT stepf = 0.125f;

            CGU_FLOAT Scl[NUM_ENDPOINTS];
            Scl[0] = PrjBnd[0] - (PrjBnd[1] - PrjBnd[0]) * stepf;
            Scl[1] = PrjBnd[1] + (PrjBnd[1] - PrjBnd[0]) * stepf;

            // No range found exit
            if (Scl[0] == Scl[1]) {
                return false;
            }

            // compute scaling factor to scale down the search interval to [0.,1]
            const CGU_FLOAT Scl2 = (Scl[1] - Scl[0]) * (Scl[1] - Scl[0]);
            const CGU_FLOAT overScl = 1.f/(Scl[1] - Scl[0]);

            for(i = 0; i < dwUniqueColors; i++) {
                // scale them
                Prj[i] = (Prj[i] - Scl[0]) * overScl;
                // premultiply the scale squire to plug into error computation later
                PreMRep[i] = Rpt[i] * Scl2;
            }

            // scale first approximation of end points
            for(k = 0; k <2; k++)
                PrjBnd[k] = (PrjBnd[k] - Scl[0]) * overScl;

            CGU_FLOAT StepErr = MAX_ERROR;

            // search step
            static const CGU_FLOAT searchStep = 0.025f;

            // low Start/End; high Start/End
            const CGU_FLOAT lowStartEnd  = (PrjBnd[0] - 2.f * searchStep > 0.f) ?  PrjBnd[0] - 2.f * searchStep : 0.f;
            const CGU_FLOAT highStartEnd = (PrjBnd[1] + 2.f * searchStep < 1.f) ?  PrjBnd[1] + 2.f * searchStep : 1.f;

            // find the best endpoints
            CGU_FLOAT Pos[NUM_ENDPOINTS];
            CGU_FLOAT lowPosStep, highPosStep;
            CGU_FLOAT err;

            int l, h;
            for(l = 0, lowPosStep = lowStartEnd; l < 8; l++, lowPosStep += searchStep) {
                for(h = 0, highPosStep = highStartEnd; h < 8; h++, highPosStep -= searchStep) {
                    // compute an error for the current pair of end points.
                    err = cpu_RampSrchW(Prj, PrjErr, PreMRep, StepErr, lowPosStep, highPosStep, dwUniqueColors, dwNumPoints);

                    if(err < StepErr) {
                        // save better result
                        StepErr = err;
                        Pos[0] = lowPosStep;
                        Pos[1] = highPosStep;
                    }
                }
            }

            // inverse the scaling
            for(k = 0; k < 2; k++)
                Pos[k] = Pos[k] * (Scl[1] - Scl[0])+ Scl[0];

            // did we find somthing better from the previous run?
            if(StepErr + 0.001 < ErrG) {
                // yes, remember it
                ErrG = StepErr;
                LineDirG[0] =  LineDir[0];
                LineDirG[1] =  LineDir[1];
                LineDirG[2] =  LineDir[2];
                PosG[0] = Pos[0];
                PosG[1] = Pos[1];
                //  3. Compute the vector of indexes (or clusters) for the current approximate ramp.
                // indexes
                const CGU_FLOAT step = (Pos[1] - Pos[0]) / (CGU_FLOAT)(dwNumPoints - 1);
                const CGU_FLOAT step_h = step * (CGU_FLOAT)0.5;
                const CGU_FLOAT rstep = (CGU_FLOAT)1.0f / step;
                const CGU_FLOAT overBlkTp = 1.f/  (CGU_FLOAT)(dwNumPoints - 1) ;

                // here the index vector is computed,
                // shifted and normalized
                CGU_FLOAT indxAvrg = (CGU_FLOAT)(dwNumPoints - 1) / 2.f;

                for(i=0; i < dwUniqueColors; i++) {
                    CGU_FLOAT del;
                    //int n = (int)((b - _min_ex + (step*0.5f)) * rstep);
                    if((del = Prj0[i] - Pos[0]) <= 0)
                        RmpIndxs[i] = 0.f;
                    else if(Prj0[i] -  Pos[1] >= 0)
                        RmpIndxs[i] = (CGU_FLOAT)(dwNumPoints - 1);
                    else
                        RmpIndxs[i] = floor((del + step_h) * rstep);
                    // shift and normalization
                    RmpIndxs[i] = (RmpIndxs[i] - indxAvrg) * overBlkTp;
                }

                //  4. Present our color channels as 3 16DIM vectors.
                //  5. Find closest aproximation of each of 16DIM color vector with the pojection of the 16DIM index vector.
                CGU_FLOAT Crs[3], Len, Len2;
                for(i = 0, Crs[0] = Crs[1] = Crs[2] = Len = 0.f; i < dwUniqueColors; i++) {
                    const CGU_FLOAT PreMlt = RmpIndxs[i] * Rpt[i];
                    Len += RmpIndxs[i] * PreMlt;
                    for(j = 0; j < 3; j++)
                        Crs[j] += BlkSh[i][j] * PreMlt;
                }

                LineDir[0] = LineDir[1] = LineDir[2] = 0.f;
                if(Len > 0.f) {
                    LineDir[0] = Crs[0]/ Len;
                    LineDir[1] = Crs[1]/ Len;
                    LineDir[2] = Crs[2]/ Len;

                    //  6. Plug the projections as a new directional vector for the axis.
                    //  7. Goto 1.
                    Len2 = LineDir[0] * LineDir[0] + LineDir[1] * LineDir[1] + LineDir[2] * LineDir[2];
                    Len2 = sqrt(Len2);

                    LineDir[0] /= Len2;
                    LineDir[1] /= Len2;
                    LineDir[2] /= Len2;
                }
            } else // We was not able to find anything better.  Drop dead.
                break;
        }

        // inverse transform to find end-points of 3-color ramp
        for(k = 0; k < 2; k++)
            for(j = 0; j < 3; j++)
                rsltC[j][k] = (PosG[k] * LineDirG[j]  + Mdl[j]) * 255.f;
    }

    // We've dealt with (almost) unrestricted full precision realm.
    // Now back to the dirty digital world.
    
    // round the end points to make them look like compressed ones
    CGU_FLOAT inpRmpEndPts[NUM_CHANNELS][NUM_ENDPOINTS];
    cpu_MkRmpOnGrid(inpRmpEndPts, rsltC, 0.f, 255.f, nRedBits, nGreenBits, nBlueBits);

    // Try using this on 3 channels
    // static CGU_Vec2i cmp_getLinearEndPoints(CGU_FLOAT _Blk[BLOCK_SIZE_4X4], CMP_IN CGU_FLOAT fquality, CMP_IN CGU_BOOL isSigned);

    // This not a small procedure squeezes and stretches the ramp along each axis (R,G,B) separately while other 2 are fixed.
    // It does it only over coarse grid - 565 that is. It tries to squeeze more precision for the real world ramp.
#if defined(USE_REFINE) || defined(USE_REFINE3D)
     switch(nRefinementSteps) {
        case 1:
             cmp_Refine(_RsltRmpPnts, inpRmpEndPts, src_image, Rpt, dwUniqueColors, dwNumPoints, pfWeights, nRedBits, nGreenBits, nBlueBits,3);
             break;
        case 2:
            if (dwUniqueColors > 2)
                cmp_Refine3D(_RsltRmpPnts, inpRmpEndPts, src_image, Rpt, dwUniqueColors, dwNumPoints, pfWeights, nRedBits, nGreenBits, nBlueBits, 1);
            else
                cmp_Refine(_RsltRmpPnts, inpRmpEndPts, src_image, Rpt, dwUniqueColors, dwNumPoints, pfWeights, nRedBits, nGreenBits, nBlueBits,3);
            break;
        default:
            cmp_Refine(_RsltRmpPnts, inpRmpEndPts, src_image, Rpt, dwUniqueColors, dwNumPoints, pfWeights, nRedBits, nGreenBits, nBlueBits, 1);
            break;
     }
#endif
    return true;
}

// CPU: CompRGBBlock()
CMP_STATIC CGU_FLOAT cpu_CompRGBBlock32(CGU_UINT32  block_32[16],
                                      CGU_UINT32  compressedBlock[2],
                                      CGU_UINT32  dwBlockSize,
                                      CGU_UINT8   nRedBits,
                                      CGU_UINT8   nGreenBits,
                                      CGU_UINT8   nBlueBits,
                                      CGU_UINT8   nEndpoints[3][NUM_ENDPOINTS], 
                                      CGU_UINT8   pcIndices[BLOCK_SIZE_4X4],
                                      CGU_UINT8   dwNumPoints,
                                      bool        b3DRefinement,
                                      CGU_UINT8   m_nRefinementSteps,
                                      CGU_FLOAT  _pfChannelWeights[3],
                                      bool        _bUseAlpha,
                                      CGU_UINT8   _nAlphaThreshold)
{
    ALIGN_16 CGU_FLOAT Rpt[BLOCK_SIZE_4X4];
    ALIGN_16 CGU_FLOAT BlkIn[BLOCK_SIZE_4X4][NUM_CHANNELS];
    CGU_UINT32 mx;
    for (mx=0; mx < BLOCK_SIZE_4X4; mx++) {
        Rpt[mx] = 0;
        BlkIn[mx][0] = 0;
        BlkIn[mx][1] = 0;
        BlkIn[mx][2] = 0;
        BlkIn[mx][3] = 0;
    }

    compressedBlock[0] = 0;

    CGU_UINT32 dwAlphaThreshold = _nAlphaThreshold << 24;
    CGU_UINT32 dwColors = 0;
    CGU_UINT32 dwBlk[BLOCK_SIZE];
    for(CGU_UINT32 i = 0; i < dwBlockSize; i++)
        if(!_bUseAlpha || (block_32[i] & 0xff000000) >= dwAlphaThreshold)
            dwBlk[dwColors++] = block_32[i] | 0xff000000;

    // Do we have any colors ?
    static int id=0;
    if(dwColors) {
        bool bHasAlpha = (dwColors != dwBlockSize);
        if(bHasAlpha && _bUseAlpha && !(dwNumPoints & 0x1))
            return CMP_FLT_MAX;

        // Here we are computing an unique number of colors.
        // For each unique value we compute the number of it appearences.
        //qsort((void *)dwBlk, (size_t)dwColors, sizeof(CGU_UINT32), QSortIntCmp);
#ifndef ASPM_GPU // this is here for reminder when code moves to GPU
            std::sort(dwBlk, dwBlk + 15);
#else
            {
                CGU_UINT32 j;
                CMP_di     what[BLOCK_SIZE_4X4];

                for (i = 0; i < dwColors; i++)
                {
                    what[i].index = i;
                    what[i].data  = dwBlk[i];
                }

                CGU_UINT32 tmp_index;
                CGU_UINT32 tmp_data;

                for (i = 1; i < dwColors; i++)
                {
                    for (j = i; j > 0; j--)
                    {
                        if (what[j - 1].data > what[j].data)
                        {
                            tmp_index         = what[j].index;
                            tmp_data          = what[j].data;
                            what[j].index     = what[j - 1].index;
                            what[j].data      = what[j - 1].data;
                            what[j - 1].index = tmp_index;
                            what[j - 1].data  = tmp_data;
                        }
                    }
                }
                for (i = 0; i < dwColors; i++)
                    dwBlk[i] = what[i].data;
            }
#endif


        CGU_UINT32 new_p;
        CGU_UINT32 dwBlkU[BLOCK_SIZE_4X4];
        CGU_UINT32 dwUniqueColors = 0;
        new_p = dwBlkU[0] = dwBlk[0];
        Rpt[dwUniqueColors] = 1.f;
        CGU_UINT32 i;
        for( i = 1; i < dwColors; i++) {
            if(new_p != dwBlk[i]) {
                dwUniqueColors++;
                new_p = dwBlkU[dwUniqueColors] = dwBlk[i];
                Rpt[dwUniqueColors] = 1.f;
            } else
                Rpt[dwUniqueColors] += 1.f;
        }
        dwUniqueColors++;

        // switch to float
        for( i=0; i<dwUniqueColors; i++) {
            BlkIn[i][RC] = (CGU_FLOAT)((dwBlkU[i] >> 16) & 0xff); // R
            BlkIn[i][GC] = (CGU_FLOAT)((dwBlkU[i] >> 8)  & 0xff); // G
            BlkIn[i][BC] = (CGU_FLOAT)((dwBlkU[i] >> 0)  & 0xff); // B
            BlkIn[i][AC] = 255.0f;
        }

        CGU_FLOAT rsltC[NUM_CHANNELS][NUM_ENDPOINTS];
        if (cpu_CompressRGBBlockX(rsltC,                        //  CMP_EndPoints = CompressRGBBlock_Slow2 (
                          BlkIn,                                //  CGU_Vec3f  src_imageNorm[BLOCK_SIZE_4X4]
                          Rpt,                                  //  CGU_FLOAT  Rpt[BLOCK_SIZE_4X4],
                          dwUniqueColors,                       //  CGU_UINT32 dwUniqueColors,
                          dwNumPoints,                          //  CGU_UINT32 dwNumPoints,
                          b3DRefinement,                        //
                          m_nRefinementSteps,                       //  CGU_UINT32 m_nRefinementSteps,
                          _pfChannelWeights,                    //  CGU_Vec3f  channelWeightsBGR,
                          nRedBits,                             //  );
                          nGreenBits, 
                          nBlueBits,
                          1.0f) )
        {
            // return to integer realm
            for(int ch = 0; ch < 3; ch++)
                for(int j = 0; j < 2; j++)
                    nEndpoints[ch][j] =  (CGU_UINT8 )rsltC[ch][j];
            //printf("Endpoints {%3d,%3d,%3d} {%3d,%3d,%3d} ", nEndpoints[0][0],nEndpoints[1][0],nEndpoints[2][0],
            //                                                  nEndpoints[0][1],nEndpoints[1][1],nEndpoints[2][1]);

            // Now get the indices using the new end points
            return cpu_Clstr(block_32, dwBlockSize, nEndpoints, pcIndices, dwNumPoints, _pfChannelWeights, _bUseAlpha,_nAlphaThreshold, nRedBits, nGreenBits, nBlueBits);
        }
        else {
            CGU_FLOAT        CompErr = CMP_FLT_MAX;
            if (dwNumPoints < 4) {
                CGU_Vec3f        src_imageNorm[BLOCK_SIZE_4X4];
    
                for (CGU_UINT32 px = 0; px < 16; px++)
                {
                   src_imageNorm[px].r = (CGU_FLOAT)((block_32[px] >> 16) & 0xff)/ 255.0f;
                   src_imageNorm[px].g = (CGU_FLOAT)((block_32[px] >> 8)  & 0xff)/ 255.0f;
                   src_imageNorm[px].b = (CGU_FLOAT)((block_32[px] >> 0)  & 0xff)/ 255.0f;
                }
    
                // Do a quick compression test
                CGU_Vec3f srcRGB[16];      // The list of source colors with blue channel altered
                CGU_Vec3f average_rgb;     // The centrepoint of the axis
                CGU_FLOAT  errLQ    = CMP_FLT_MAX;
                cgu_CompressRGBBlock_MinMax(src_imageNorm, 1.0f, false,srcRGB, average_rgb, errLQ);
                CGU_Vec2ui cmp =  cgu_CompressRGBBlock_Fast(src_imageNorm, 1.0f, false,srcRGB, average_rgb, CompErr);

                compressedBlock[0] = cmp.x;
                compressedBlock[1] = cmp.y;
            }
            return CompErr;
        }
    } else {
        // All colors transparent
        nEndpoints[0][0] = nEndpoints[1][0] = nEndpoints[2][0] = 0;
        nEndpoints[0][1] = nEndpoints[1][1] = nEndpoints[2][1] = 0xff;
        for (CGU_UINT32 ms=0; ms<dwBlockSize; ms++)
            pcIndices[ms] = 0xff;
        return 0.0;
    }
}


CMP_STATIC CGU_Vec2ui cpu_CompRGBBlock(CMP_IN CGU_Vec4uc bgraBlock[BLOCK_SIZE_4X4],
                                       CMP_IN CMP_BC15Options BC15Options,
                                       CMP_INOUT CGU_FLOAT CMP_REFINOUT err)
{
    CGU_Vec2ui  cmpBlock         = {0U,0U};
    CGU_FLOAT   pfChannelWeights[3]     = {1.0f,1.0f,1.0f};
    CGU_UINT8   nEndpoints[2][3][2];
    CGU_UINT8   nIndices[2][BLOCK_SIZE_4X4]; 
    CGU_UINT32  compressedBlock[2] = {0,0};

    CGU_FLOAT fError3 = CMP_FLT_MAX;

    fError3 = cpu_CompRGBBlock32((CGU_UINT32*)bgraBlock,
                                          compressedBlock, 
                                          BLOCK_SIZE_4X4, RG, GG, BG, 
                                          nEndpoints[0], 
                                          nIndices[0], 
                                          3, 
                                          BC15Options.m_b3DRefinement, 
                                          BC15Options.m_nRefinementSteps, 
                                          pfChannelWeights, 
                                          BC15Options.m_bUseAlpha, 
                                          BC15Options.m_nAlphaThreshold);
    // use case of small min max ranges
    if (compressedBlock[0] > 0)
    {
        //return cmpBlockBlue;
        cmpBlock.x = compressedBlock[0];
        cmpBlock.y = compressedBlock[1];
        err = fError3;
    }
    else
    {
        CGU_FLOAT fError4 = CMP_FLT_MAX;
        fError4 = (fError3 == 0.0) ? CMP_FLT_MAX :cpu_CompRGBBlock32((CGU_UINT32*)bgraBlock, 
                                                                   compressedBlock, 
                                                                   BLOCK_SIZE_4X4, RG, GG, BG, 
                                                                   nEndpoints[1], 
                                                                   nIndices[1], 
                                                                   4, 
                                                                   BC15Options.m_b3DRefinement, 
                                                                   BC15Options.m_nRefinementSteps, 
                                                                   pfChannelWeights, 
                                                                   BC15Options.m_bUseAlpha, 
                                                                   BC15Options.m_nAlphaThreshold);

        CGU_UINT32 nMethod;
        if (fError3 <= fError4) {
            err = fError3;
            nMethod = 0;
        }
        else {
            err = fError4;
            nMethod = 1;
        }


        CGU_UINT32 c0 = BC1ConstructColour((nEndpoints[nMethod][RC][0] >> (8-RG)), (nEndpoints[nMethod][GC][0] >> (8-GG)), (nEndpoints[nMethod][BC][0] >> (8-BG)));
        CGU_UINT32 c1 = BC1ConstructColour((nEndpoints[nMethod][RC][1] >> (8-RG)), (nEndpoints[nMethod][GC][1] >> (8-GG)), (nEndpoints[nMethod][BC][1] >> (8-BG)));
        if(nMethod == 1 && c0 <= c1 || nMethod == 0 && c0 > c1)
            compressedBlock[0] = c1 | (c0<<16);
        else
            compressedBlock[0] = c0 | (c1<<16);

        compressedBlock[1] = 0;
        for(CGU_UINT32 i=0; i<16; i++)
            compressedBlock[1] |= (nIndices[nMethod][i] << (2*i));

        cmpBlock.x = compressedBlock[0];
        cmpBlock.y = compressedBlock[1];
    }

    return cmpBlock;
}

#endif

#ifdef ENABLE_NEW_CODE

//---------------------------------------- Common Utility Code -------------------------------------------------------
// 1 - Dim error
CMP_STATIC CGU_FLOAT cgu_RampSrchW( CGU_FLOAT  Prj[BLOCK_SIZE_4X4],
                                    CGU_FLOAT  PrjErr[BLOCK_SIZE_4X4],
                                    CGU_FLOAT  PreMRep[BLOCK_SIZE_4X4],
                                    CGU_FLOAT  StepErr,
                                    CGU_FLOAT  lowPosStep,
                                    CGU_FLOAT  highPosStep,
                                    CGU_UINT32 dwUniqueColors,
                                    CGU_UINT32 dwNumPoints)
{
    CGU_FLOAT error  = 0;
    CGU_FLOAT step   = (highPosStep - lowPosStep) / (dwNumPoints - 1);
    CGU_FLOAT step_h = step * (CGU_FLOAT)0.5;
    CGU_FLOAT rstep  = (CGU_FLOAT)1.0f / step;

    for (CGU_UINT32 i = 0; i < dwUniqueColors; i++)
    {
        CGU_FLOAT v;
        // Work out which value in the block this select
        CGU_FLOAT del;

        if ((del = Prj[i] - lowPosStep) <= 0)
            v = lowPosStep;
        else if (Prj[i] - highPosStep >= 0)
            v = highPosStep;
        else
            v = floor((del + step_h) * rstep) * step + lowPosStep;

        // And accumulate the error
        CGU_FLOAT d = (Prj[i] - v);
        d *= d;
        CGU_FLOAT err = PreMRep[i] * d + PrjErr[i];
        error += err;
        if (StepErr < error)
        {
            error = StepErr;
            break;
        }
    }
    return error;
}

CMP_STATIC CGU_UINT32  cgu_processCluster(  CMP_IN  CMP_EndPoints   EndPoints,
                                            CMP_IN  CGU_Vec4f       rgbBlock_normal[BLOCK_SIZE_4X4],
                                            CMP_IN  CGU_UINT32      dwAlphaThreshold,
                                            CMP_IN  CGU_Vec3f       channelWeights,
                                            CMP_IN  CGU_UINT8       indices[BLOCK_SIZE_4X4],
                                            CMP_OUT CGU_FLOAT  CMP_REFINOUT  Err )
{
    Err = 0.f;
    CGU_UINT32 pcIndices = 0;
    CGU_UINT32 R, G, B;
    
    R                  = (CGU_UINT32)(EndPoints.Color0.z);
    G                  = (CGU_UINT32)(EndPoints.Color0.y);
    B                  = (CGU_UINT32)(EndPoints.Color0.x);
    CGU_INT32 cluster0 = cmp_constructColor(R, G, B);
    
    R                  = (CGU_UINT32)(EndPoints.Color1.z);
    G                  = (CGU_UINT32)(EndPoints.Color1.y);
    B                  = (CGU_UINT32)(EndPoints.Color1.x);
    CGU_INT32 cluster1 = cmp_constructColor(R, G, B);
    
    CGU_Vec3f InpRmp[NUM_ENDPOINTS];
    if ((cluster0 <= cluster1)  // valid for 4 channels
                                // || (cluster0 > cluster1)    // valid for 3 channels
    )
    {
        // inverse endpoints
        InpRmp[0] = EndPoints.Color1;
        InpRmp[1] = EndPoints.Color0;
    }
    else
    {
        InpRmp[0] = EndPoints.Color0;
        InpRmp[1] = EndPoints.Color1;
    }
    
    CGU_Vec3f srcblockLinear[BLOCK_SIZE_4X4];
    CGU_FLOAT srcblockA[BLOCK_SIZE_4X4];
    
    // Swizzle the source RGB to BGR for processing
    for (CGU_UINT32 i = 0; i < BLOCK_SIZE_4X4; i++)
    {
        srcblockLinear[i].z = rgbBlock_normal[i].x * 255.0f;
        srcblockLinear[i].y = rgbBlock_normal[i].y * 255.0f;
        srcblockLinear[i].x = rgbBlock_normal[i].z * 255.0f;
        srcblockA[i]     = 0.0f;
        //if (dwAlphaThreshold > 0)
        //{
        //    CGU_UINT32 alpha = (CGU_UINT32)BlockA[i];
        //    if (alpha >= dwAlphaThreshold)
        //        srcblockA[i] = BlockA[i];
        //}
    }
    

    // cmp_ClstrBas2()
    // input ramp is on the coarse grid
    // make ramp endpoints the way they'll going to be decompressed
    CGU_Vec3f InpRmpL[NUM_ENDPOINTS];
    CGU_Vec3f Fctrs = {32.0F, 64.0F, 32.0F};  // 1 << RG,1 << GG,1 << BG
    
    {
        //   ConstantRamp = MkWkRmpPts(InpRmpL, InpRmp);
        InpRmpL[0] = InpRmp[0] + cmp_floorVec3f(InpRmp[0] / Fctrs);
        InpRmpL[0] = cmp_clampVec3f(InpRmpL[0], 0.0f, 255.0f);
        InpRmpL[1] = InpRmp[1] + cmp_floorVec3f(InpRmp[1] / Fctrs);
        InpRmpL[1] = cmp_clampVec3f(InpRmpL[1], 0.0f, 255.0f);
    }  // MkWkRmpPts
    
    // build ramp
    CGU_Vec3f LerpRmp[4];
    CGU_Vec3f offset = {1.0f, 1.0f, 1.0f};
    {
        //BldRmp(Rmp, InpRmpL, dwNumChannels);
        // linear interpolate end points to get the ramp
        LerpRmp[0] = InpRmpL[0];
        LerpRmp[3] = InpRmpL[1];
        LerpRmp[1] = cmp_floorVec3f((InpRmpL[0] * 2.0f + LerpRmp[3] + offset) / 3.0f);
        LerpRmp[2] = cmp_floorVec3f((InpRmpL[0] + LerpRmp[3] * 2.0f + offset) / 3.0f);
    }  // BldRmp
    
    //=========================================================================
    // Clusterize, Compute error and find DXTC indexes for the current cluster
    //=========================================================================
    {
        // Clusterize
        CGU_UINT32 alpha;
    
        // For each colour in the original block assign it
        // to the closest cluster and compute the cumulative error
        for (CGU_UINT32 i = 0; i < BLOCK_SIZE_4X4; i++)
        {
            alpha = (CGU_UINT32)srcblockA[i];
            if ((dwAlphaThreshold > 0) && alpha == 0)
            {                                      //*((CGU_UINT32 *)&_Blk[i][AC]) == 0)
                pcIndices |= cmp_set2Bit32(4, i);  // dwNumChannels 3 or 4 (default is 4)
                indices[i] = 4;
            }
            else
            {
                CGU_FLOAT shortest      = 99999999999.f;
                CGU_UINT8 shortestIndex = 0;
    
                CGU_Vec3f channelWeightsBGR;
                channelWeightsBGR.x = channelWeights.z;
                channelWeightsBGR.y = channelWeights.y;
                channelWeightsBGR.z = channelWeights.x;
    
                for (CGU_UINT8 rampindex = 0; rampindex < 4; rampindex++)
                {
                    // r is either 1 or 4
                    // calculate the distance for each component
                    CGU_FLOAT distance = cmp_dotVec3f(((srcblockLinear[i] - LerpRmp[rampindex]) * channelWeightsBGR),
                                             ((srcblockLinear[i] - LerpRmp[rampindex]) * channelWeightsBGR));
                    if (distance < shortest)
                    {
                        shortest      = distance;
                        shortestIndex = rampindex;
                    }
                }
    
                Err += shortest;

                // The total is a sum of (error += shortest)
                // We have the index of the best cluster, so assign this in the block
                // Reorder indices to match correct DXTC ordering
                if (shortestIndex == 3)  // dwNumChannels - 1
                    shortestIndex = 1;
                else if (shortestIndex)
                    shortestIndex++;
                pcIndices |= cmp_set2Bit32(shortestIndex, i);
                indices[i] = shortestIndex;
            }
        }  // BLOCK_SIZE_4X4
    }      // Clusterize
    
    return pcIndices;
}
#endif

// Process a rgbBlock which is normalized (0.0f ... 1.0f), signed normal is not implemented
CMP_STATIC CGU_Vec2ui CompressBlockBC1_NORMALIZED(CMP_IN CGU_Vec4f src_imageNorm[BLOCK_SIZE_4X4],
                                                  CMP_IN CMP_BC15Options BC15Options)
{
    bool usingMaxQualityOnly = false;



#ifndef ASPM_GPU
    if (BC15Options.m_fquality > 0.75) 
          usingMaxQualityOnly = true;
#endif

    CGU_FLOAT  CompErr          = CMP_FLT_MAX;
    CGU_Vec2ui cmpBlock         = {0U,0U};
    CGU_Vec2ui cmpBlockTemp     = {0U,0U};
    CGU_FLOAT  CompErrTemp;

    // Transfer to RGB Norm from RGBA Norm
    CGU_Vec3f src_imageRGBNorm[16];
    CGU_Vec4uc pixels[16];
    CGU_Vec4uc pixelsBGRA[16];

    for (CGU_UINT32 sr = 0; sr < 16; sr++) {
        src_imageRGBNorm[sr] = src_imageNorm[sr].rgb;
        pixelsBGRA[sr].b = pixels[sr].r = src_imageNorm[sr].r * 255.0f;
        pixelsBGRA[sr].g = pixels[sr].g = src_imageNorm[sr].g * 255.0f;
        pixelsBGRA[sr].r = pixels[sr].b = src_imageNorm[sr].b * 255.0f;
        pixelsBGRA[sr].a = pixels[sr].a = src_imageNorm[sr].a * 255.0f;
    }


    // check for a punch through transparent alpha setting
    if ((BC15Options.m_fquality < 0.75) && (BC15Options.m_bUseAlpha)) {
        CGU_Vec2ui cmpBlockAlpha         = {0xffff0000,0xffffffffU};
        for (CGU_UINT32 sr = 0; sr < 16; sr++) 
            if (pixels[sr].a < BC15Options.m_nAlphaThreshold) {
                return cmpBlockAlpha;
            }
    }

    //================
    // extern codec
    //================
    // For debugging
    // CGU_Vec2ui cmpBlockRed   = {0xF800F800,0x00000000};
    // CGU_Vec2ui cmpBlockGreen = {0x7E007E00,0x00000000};
    // CGU_Vec2ui cmpBlockBlue  = {0x1F001F00,0x00000000};

    if (!BC15Options.m_bUseAlpha ) {
        //==========================================
        // Gain +0.3 dB for images with soild blocks
        //==========================================
        bool bAllColoursEqual = true;
        
        // Load the whole 4x4 block
        for (CGU_UINT32 i = 0u; (i < 16u) && bAllColoursEqual; ++i)
        {
            for (CGU_INT c = 0; c < 3; c++)
                bAllColoursEqual  = bAllColoursEqual && (pixels[0][c] == pixels[i][c]);
        }

        if (bAllColoursEqual) {
            cmpBlock = cgu_solidColorBlock(pixels[0].x,pixels[0].y,pixels[0].z);
            CompErr  = cgu_RGBABlockErrorLinear(pixels, cmpBlock);
            if (BC15Options.m_nRefinementSteps < 1) return cmpBlock;
        }
    }

    if (!usingMaxQualityOnly) {
        //====================================
        // Get src image data, min,max...
        //=====================================
        //CMP_EncodeData edata;
        //cmp_get_encode_data(edata,pixels);

        if (!BC15Options.m_bUseAlpha) {
            //====================================
            // Fast Compression, low quality
            //=====================================
            CGU_Vec3f srcRGB[16];      // The list of source colors with blue channel altered
            CGU_Vec3f average_rgb;     // The centrepoint of the axis
            CGU_FLOAT  errLQ    = CMP_FLT_MAX;
            cmpBlockTemp  = cgu_CompressRGBBlock_MinMax(src_imageRGBNorm, BC15Options.m_fquality, BC15Options.m_bIsSRGB,srcRGB, average_rgb, errLQ);
            if ((BC15Options.m_fquality < CMP_QUALITY0) || (errLQ == 0.0f))
                return cmpBlockTemp;

            if (CompErr > errLQ) {
                CompErr  = errLQ;
                cmpBlock = cmpBlockTemp;
            }

            cmpBlockTemp  = cgu_CompressRGBBlock_Fast(src_imageRGBNorm, BC15Options.m_fquality, BC15Options.m_bIsSRGB,srcRGB, average_rgb, errLQ);
            if (CompErr > errLQ) {
                CompErr  = errLQ;
                cmpBlock = cmpBlockTemp;
            }
            if (BC15Options.m_fquality < CMP_QUALITY1)
                return cmpBlock;
        }

        //========================================
        // use GPU codec lower quality then CPU
        //========================================
         cmpBlockTemp = cgu_CompRGBBlock(src_imageNorm,BC15Options);
         CompErrTemp  = cgu_RGBABlockErrorLinear(pixels, cmpBlockTemp);
         if (CompErr > CompErrTemp) {
              CompErr  = CompErrTemp;
              cmpBlock = cmpBlockTemp;
         }

         if (BC15Options.m_fquality < CMP_QUALITY2) return cmpBlock;
    }// if useCGUCodecs

    //====================================
    // High Quality Codec CPU only
    //=====================================
#ifndef ASPM_GPU
    cmpBlockTemp = cpu_CompRGBBlock(pixelsBGRA,BC15Options,CompErrTemp);
    CompErrTemp  = cgu_RGBABlockErrorLinear(pixels, cmpBlockTemp);
    if (CompErr > CompErrTemp) {
        CompErr  = CompErrTemp;
        cmpBlock = cmpBlockTemp;
    }
#endif

    return cmpBlock;
}
