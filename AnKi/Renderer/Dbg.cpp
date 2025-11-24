// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Scene.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Shaders/Include/GpuVisibilityTypes.h>
#include <AnKi/Window/Input.h>

namespace anki {

static const F32 g_gizmoArrowPositions[49][3] = {{0.035852f, -0.0f, -0.0f},
												 {0.035852f, -0.0f, 0.752929f},
												 {0.033123f, -0.01372f, -0.0f},
												 {0.033123f, -0.01372f, 0.752929f},
												 {0.025351f, -0.025351f, -0.0f},
												 {0.025351f, -0.025351f, 0.752929f},
												 {0.01372f, -0.033123f, -0.0f},
												 {0.01372f, -0.033123f, 0.752929f},
												 {0.0f, -0.035852f, -0.0f},
												 {-0.0f, -0.035852f, 0.752929f},
												 {-0.01372f, -0.033123f, -0.0f},
												 {-0.01372f, -0.033123f, 0.752929f},
												 {-0.025351f, -0.025351f, -0.0f},
												 {-0.025351f, -0.025351f, 0.752929f},
												 {-0.033123f, -0.01372f, -0.0f},
												 {-0.033123f, -0.01372f, 0.752929f},
												 {-0.035852f, -0.0f, -0.0f},
												 {-0.035852f, -0.0f, 0.752929f},
												 {-0.033123f, 0.01372f, -0.0f},
												 {-0.033123f, 0.01372f, 0.752929f},
												 {-0.025351f, 0.025351f, -0.0f},
												 {-0.025351f, 0.025351f, 0.752929f},
												 {-0.01372f, 0.033123f, -0.0f},
												 {-0.01372f, 0.033123f, 0.752929f},
												 {0.0f, 0.035852f, -0.0f},
												 {-0.0f, 0.035852f, 0.752929f},
												 {0.01372f, 0.033123f, -0.0f},
												 {0.01372f, 0.033123f, 0.752929f},
												 {0.025351f, 0.025351f, -0.0f},
												 {0.025351f, 0.025351f, 0.752929f},
												 {0.033123f, 0.01372f, -0.0f},
												 {0.033123f, 0.01372f, 0.752929f},
												 {0.072426f, -0.0f, 0.778954f},
												 {0.066913f, -0.027717f, 0.778954f},
												 {0.051213f, -0.051213f, 0.778954f},
												 {0.027716f, -0.066913f, 0.778954f},
												 {-0.0f, -0.072427f, 0.778954f},
												 {-0.027716f, -0.066913f, 0.778954f},
												 {-0.051213f, -0.051213f, 0.778954f},
												 {-0.066913f, -0.027717f, 0.778954f},
												 {-0.072426f, -0.0f, 0.778954f},
												 {-0.066913f, 0.027716f, 0.778954f},
												 {-0.051213f, 0.051213f, 0.778954f},
												 {-0.027716f, 0.066913f, 0.778954f},
												 {-0.0f, 0.072426f, 0.778954f},
												 {0.027716f, 0.066913f, 0.778954f},
												 {0.051213f, 0.051213f, 0.778954f},
												 {0.066913f, 0.027716f, 0.778954f},
												 {-0.0f, -0.0f, 1.0f}};

static const U16 g_gizmoArrowIndices[94][3] = {
	{1, 2, 0},    {3, 4, 2},    {5, 6, 4},    {6, 9, 8},    {9, 10, 8},   {11, 12, 10}, {13, 14, 12}, {15, 16, 14}, {17, 18, 16}, {19, 20, 18},
	{21, 22, 20}, {23, 24, 22}, {25, 26, 24}, {27, 28, 26}, {13, 37, 38}, {29, 30, 28}, {31, 0, 30},  {6, 14, 22},  {46, 45, 48}, {23, 42, 43},
	{1, 47, 32},  {7, 36, 9},   {19, 40, 41}, {27, 46, 29}, {5, 33, 34},  {15, 38, 39}, {23, 44, 25}, {9, 37, 11},  {19, 42, 21}, {29, 47, 31},
	{7, 34, 35},  {15, 40, 17}, {1, 33, 3},   {25, 45, 27}, {39, 38, 48}, {47, 46, 48}, {40, 39, 48}, {33, 32, 48}, {32, 47, 48}, {41, 40, 48},
	{34, 33, 48}, {42, 41, 48}, {35, 34, 48}, {43, 42, 48}, {36, 35, 48}, {44, 43, 48}, {37, 36, 48}, {45, 44, 48}, {38, 37, 48}, {1, 3, 2},
	{3, 5, 4},    {5, 7, 6},    {6, 7, 9},    {9, 11, 10},  {11, 13, 12}, {13, 15, 14}, {15, 17, 16}, {17, 19, 18}, {19, 21, 20}, {21, 23, 22},
	{23, 25, 24}, {25, 27, 26}, {27, 29, 28}, {13, 11, 37}, {29, 31, 30}, {31, 1, 0},   {30, 0, 2},   {2, 4, 6},    {6, 8, 10},   {10, 12, 14},
	{14, 16, 22}, {16, 18, 22}, {18, 20, 22}, {22, 24, 26}, {26, 28, 22}, {28, 30, 22}, {30, 2, 6},   {6, 10, 14},  {30, 6, 22},  {23, 21, 42},
	{1, 31, 47},  {7, 35, 36},  {19, 17, 40}, {27, 45, 46}, {5, 3, 33},   {15, 13, 38}, {23, 43, 44}, {9, 36, 37},  {19, 41, 42}, {29, 46, 47},
	{7, 5, 34},   {15, 39, 40}, {1, 32, 33},  {25, 44, 45}};

static const F32 g_gizmoScalePositions[64][3] = {
	{0.035852f, -0.0f, -0.0f},       {0.035852f, -0.0f, 0.752929f},       {0.033123f, -0.01372f, -0.0f},       {0.033123f, -0.01372f, 0.752929f},
	{0.025351f, -0.025351f, -0.0f},  {0.025351f, -0.025351f, 0.752929f},  {0.01372f, -0.033123f, -0.0f},       {0.01372f, -0.033123f, 0.752929f},
	{0.0f, -0.035852f, -0.0f},       {-0.0f, -0.035852f, 0.752929f},      {-0.01372f, -0.033123f, -0.0f},      {-0.01372f, -0.033123f, 0.752929f},
	{-0.025351f, -0.025351f, -0.0f}, {-0.025351f, -0.025351f, 0.752929f}, {-0.033123f, -0.01372f, -0.0f},      {-0.033123f, -0.01372f, 0.752929f},
	{-0.035852f, -0.0f, -0.0f},      {-0.035852f, -0.0f, 0.752929f},      {-0.033123f, 0.01372f, -0.0f},       {-0.033123f, 0.01372f, 0.752929f},
	{-0.025351f, 0.025351f, -0.0f},  {-0.025351f, 0.025351f, 0.752929f},  {-0.01372f, 0.033123f, -0.0f},       {-0.01372f, 0.033123f, 0.752929f},
	{0.0f, 0.035852f, -0.0f},        {-0.0f, 0.035852f, 0.752929f},       {0.01372f, 0.033123f, -0.0f},        {0.01372f, 0.033123f, 0.752929f},
	{0.025351f, 0.025351f, -0.0f},   {0.025351f, 0.025351f, 0.752929f},   {0.033123f, 0.01372f, -0.0f},        {0.033123f, 0.01372f, 0.752929f},
	{0.072426f, -0.0f, 0.778954f},   {0.066913f, -0.027717f, 0.778954f},  {0.051213f, -0.051213f, 0.778954f},  {0.027716f, -0.066913f, 0.778954f},
	{-0.0f, -0.072427f, 0.778954f},  {-0.027716f, -0.066913f, 0.778954f}, {-0.051213f, -0.051213f, 0.778954f}, {-0.066913f, -0.027717f, 0.778954f},
	{-0.072426f, -0.0f, 0.778954f},  {-0.066913f, 0.027716f, 0.778954f},  {-0.051213f, 0.051213f, 0.778954f},  {-0.027716f, 0.066913f, 0.778954f},
	{-0.0f, 0.072426f, 0.778954f},   {0.027716f, 0.066913f, 0.778954f},   {0.051213f, 0.051213f, 0.778954f},   {0.066913f, 0.027716f, 0.778954f},
	{0.072426f, -0.0f, 0.998954f},   {0.066913f, -0.027717f, 0.998954f},  {0.051213f, -0.051213f, 0.998954f},  {0.027716f, -0.066913f, 0.998954f},
	{-0.0f, -0.072427f, 0.998954f},  {-0.027716f, -0.066913f, 0.998954f}, {-0.051213f, -0.051213f, 0.998954f}, {-0.066913f, -0.027717f, 0.998954f},
	{-0.072426f, -0.0f, 0.998954f},  {-0.066913f, 0.027716f, 0.998954f},  {-0.051213f, 0.051213f, 0.998954f},  {-0.027716f, 0.066913f, 0.998954f},
	{-0.0f, 0.072426f, 0.998954f},   {0.027716f, 0.066913f, 0.998954f},   {0.051213f, 0.051213f, 0.998954f},   {0.066913f, 0.027716f, 0.998954f},
};

static const U16 g_gizmoScaleIndices[124][3] = {
	{1, 2, 0},    {3, 4, 2},    {5, 6, 4},    {6, 9, 8},    {9, 10, 8},   {11, 12, 10}, {13, 14, 12}, {15, 16, 14}, {17, 18, 16}, {19, 20, 18},
	{21, 22, 20}, {23, 24, 22}, {25, 26, 24}, {27, 28, 26}, {13, 37, 38}, {29, 30, 28}, {31, 0, 30},  {6, 14, 22},  {39, 56, 40}, {23, 42, 43},
	{1, 47, 32},  {7, 36, 9},   {19, 40, 41}, {27, 46, 29}, {5, 33, 34},  {15, 38, 39}, {23, 44, 25}, {9, 37, 11},  {19, 42, 21}, {29, 47, 31},
	{7, 34, 35},  {15, 40, 17}, {1, 33, 3},   {25, 45, 27}, {45, 62, 46}, {47, 62, 63}, {38, 53, 54}, {43, 60, 44}, {44, 61, 45}, {36, 53, 37},
	{35, 52, 36}, {42, 59, 43}, {47, 48, 32}, {34, 51, 35}, {40, 57, 41}, {41, 58, 42}, {34, 49, 50}, {32, 49, 33}, {38, 55, 39}, {57, 53, 49},
	{1, 3, 2},    {3, 5, 4},    {5, 7, 6},    {6, 7, 9},    {9, 11, 10},  {11, 13, 12}, {13, 15, 14}, {15, 17, 16}, {17, 19, 18}, {19, 21, 20},
	{21, 23, 22}, {23, 25, 24}, {25, 27, 26}, {27, 29, 28}, {13, 11, 37}, {29, 31, 30}, {31, 1, 0},   {30, 0, 2},   {2, 4, 6},    {6, 8, 10},
	{10, 12, 14}, {14, 16, 22}, {16, 18, 22}, {18, 20, 22}, {22, 24, 26}, {26, 28, 22}, {28, 30, 22}, {30, 2, 6},   {6, 10, 14},  {30, 6, 22},
	{39, 55, 56}, {23, 21, 42}, {1, 31, 47},  {7, 35, 36},  {19, 17, 40}, {27, 45, 46}, {5, 3, 33},   {15, 13, 38}, {23, 43, 44}, {9, 36, 37},
	{19, 41, 42}, {29, 46, 47}, {7, 5, 34},   {15, 39, 40}, {1, 32, 33},  {25, 44, 45}, {45, 61, 62}, {47, 46, 62}, {38, 37, 53}, {43, 59, 60},
	{44, 60, 61}, {36, 52, 53}, {35, 51, 52}, {42, 58, 59}, {47, 63, 48}, {34, 50, 51}, {40, 56, 57}, {41, 57, 58}, {34, 33, 49}, {32, 48, 49},
	{38, 54, 55}, {49, 48, 63}, {63, 62, 61}, {61, 60, 59}, {59, 58, 61}, {58, 57, 61}, {57, 56, 53}, {56, 55, 53}, {55, 54, 53}, {53, 52, 49},
	{52, 51, 49}, {51, 50, 49}, {49, 63, 61}, {49, 61, 57},
};

static const F32 g_gizmoRingPositions[64][3] = {
	{0.0f, 1.0f, -0.008517f},
	{0.0f, 1.0f, 0.008517f},
	{0.382683f, 0.92388f, -0.008517f},
	{0.382683f, 0.92388f, 0.008517f},
	{0.707107f, 0.707107f, -0.008517f},
	{0.707107f, 0.707107f, 0.008517f},
	{0.923879f, 0.382683f, -0.008517f},
	{0.923879f, 0.382683f, 0.008517f},
	{1.0f, 0.0f, -0.008517f},
	{1.0f, 0.0f, 0.008517f},
	{0.923879f, -0.382683f, -0.008517f},
	{0.923879f, -0.382683f, 0.008517f},
	{0.707107f, -0.707107f, -0.008517f},
	{0.707107f, -0.707107f, 0.008517f},
	{0.382683f, -0.92388f, -0.008517f},
	{0.382683f, -0.92388f, 0.008517f},
	{0.0f, -1.0f, -0.008517f},
	{0.0f, -1.0f, 0.008517f},
	{-0.382683f, -0.92388f, -0.008517f},
	{-0.382683f, -0.92388f, 0.008517f},
	{-0.707107f, -0.707107f, -0.008517f},
	{-0.707107f, -0.707107f, 0.008517f},
	{-0.923879f, -0.382683f, -0.008517f},
	{-0.923879f, -0.382683f, 0.008517f},
	{-1.0f, 0.0f, -0.008517f},
	{-1.0f, 0.0f, 0.008517f},
	{-0.923879f, 0.382683f, -0.008517f},
	{-0.923879f, 0.382683f, 0.008517f},
	{-0.707107f, 0.707107f, -0.008517f},
	{-0.707107f, 0.707107f, 0.008517f},
	{-0.382683f, 0.92388f, -0.008517f},
	{-0.382683f, 0.92388f, 0.008517f},
	{-0.0f, 0.914026f, 0.011446f},
	{0.349782f, 0.84445f, 0.011446f},
	{0.646314f, 0.646314f, 0.011446f},
	{0.844449f, 0.349783f, 0.011446f},
	{0.914025f, 0.0f, 0.011446f},
	{0.844449f, -0.349782f, 0.011446f},
	{0.646314f, -0.646314f, 0.011446f},
	{0.349782f, -0.844449f, 0.011446f},
	{-0.0f, -0.914025f, 0.011446f},
	{-0.349782f, -0.844449f, 0.011446f},
	{-0.646314f, -0.646314f, 0.011446f},
	{-0.844449f, -0.349782f, 0.011446f},
	{-0.914025f, 0.0f, 0.011446f},
	{-0.844449f, 0.349783f, 0.011446f},
	{-0.646314f, 0.646314f, 0.011446f},
	{-0.349782f, 0.84445f, 0.011446f},
	{0.0f, 0.914026f, -0.011446f},
	{0.349783f, 0.84445f, -0.011446f},
	{0.646314f, 0.646314f, -0.011446f},
	{0.84445f, 0.349783f, -0.011446f},
	{0.914026f, 0.0f, -0.011446f},
	{0.84445f, -0.349782f, -0.011446f},
	{0.646314f, -0.646314f, -0.011446f},
	{0.349783f, -0.844449f, -0.011446f},
	{0.0f, -0.914025f, -0.011446f},
	{-0.349782f, -0.844449f, -0.011446f},
	{-0.646313f, -0.646314f, -0.011446f},
	{-0.844449f, -0.349782f, -0.011446f},
	{-0.914025f, 0.0f, -0.011446f},
	{-0.844449f, 0.349783f, -0.011446f},
	{-0.646313f, 0.646314f, -0.011446f},
	{-0.349782f, 0.84445f, -0.011446f},
};

static const U16 g_gizmoRingIndices[128][3] = {
	{0, 3, 2},    {3, 4, 2},    {5, 6, 4},    {7, 8, 6},    {9, 10, 8},   {11, 12, 10}, {13, 14, 12}, {15, 16, 14}, {17, 18, 16}, {19, 20, 18},
	{21, 22, 20}, {23, 24, 22}, {25, 26, 24}, {27, 28, 26}, {23, 42, 43}, {29, 30, 28}, {31, 0, 30},  {41, 58, 42}, {31, 32, 1},  {7, 36, 9},
	{19, 40, 41}, {29, 45, 46}, {3, 34, 5},   {13, 39, 15}, {25, 43, 44}, {9, 37, 11},  {21, 41, 42}, {31, 46, 47}, {5, 35, 7},   {17, 39, 40},
	{1, 33, 3},   {27, 44, 45}, {11, 38, 13}, {6, 52, 51},  {35, 50, 51}, {42, 59, 43}, {36, 51, 52}, {43, 60, 44}, {36, 53, 37}, {45, 60, 61},
	{37, 54, 38}, {46, 61, 62}, {38, 55, 39}, {47, 62, 63}, {39, 56, 40}, {33, 48, 49}, {32, 63, 48}, {40, 57, 41}, {34, 49, 50}, {8, 53, 52},
	{10, 54, 53}, {12, 55, 54}, {14, 56, 55}, {16, 57, 56}, {18, 58, 57}, {22, 58, 20}, {24, 59, 22}, {26, 60, 24}, {28, 61, 26}, {30, 62, 28},
	{0, 63, 30},  {2, 48, 0},   {4, 49, 2},   {6, 50, 4},   {0, 1, 3},    {3, 5, 4},    {5, 7, 6},    {7, 9, 8},    {9, 11, 10},  {11, 13, 12},
	{13, 15, 14}, {15, 17, 16}, {17, 19, 18}, {19, 21, 20}, {21, 23, 22}, {23, 25, 24}, {25, 27, 26}, {27, 29, 28}, {23, 21, 42}, {29, 31, 30},
	{31, 1, 0},   {41, 57, 58}, {31, 47, 32}, {7, 35, 36},  {19, 17, 40}, {29, 27, 45}, {3, 33, 34},  {13, 38, 39}, {25, 23, 43}, {9, 36, 37},
	{21, 19, 41}, {31, 29, 46}, {5, 34, 35},  {17, 15, 39}, {1, 32, 33},  {27, 25, 44}, {11, 37, 38}, {6, 8, 52},   {35, 34, 50}, {42, 58, 59},
	{36, 35, 51}, {43, 59, 60}, {36, 52, 53}, {45, 44, 60}, {37, 53, 54}, {46, 45, 61}, {38, 54, 55}, {47, 46, 62}, {39, 55, 56}, {33, 32, 48},
	{32, 47, 63}, {40, 56, 57}, {34, 33, 49}, {8, 10, 53},  {10, 12, 54}, {12, 14, 55}, {14, 16, 56}, {16, 18, 57}, {18, 20, 58}, {22, 59, 58},
	{24, 60, 59}, {26, 61, 60}, {28, 62, 61}, {30, 63, 62}, {0, 48, 63},  {2, 49, 48},  {4, 50, 49},  {6, 51, 50},
};

Dbg::Dbg()
{
	registerDebugRenderTarget("ObjectPicking");
}

Dbg::~Dbg()
{
}

Error Dbg::init()
{
	// RT descr
	m_rtDescr = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
															  Format::kR8G8B8A8_Unorm, "Dbg");
	m_rtDescr.bake();

	m_objectPickingRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR32_Uint, "ObjectPicking");
	m_objectPickingRtDescr.bake();

	m_objectPickingDepthRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kD32_Sfloat, "ObjectPickingDepth");
	m_objectPickingDepthRtDescr.bake();

	ResourceManager& rsrcManager = ResourceManager::getSingleton();
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/GiProbe.ankitex", m_giProbeImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/LightBulb.ankitex", m_pointLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/SpotLight.ankitex", m_spotLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/GreenDecal.ankitex", m_decalImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Mirror.ankitex", m_reflectionImage));

	ANKI_CHECK(rsrcManager.loadResource("ShaderBinaries/Dbg.ankiprogbin", m_dbgProg));

	{
		BufferInitInfo buffInit("Dbg cube verts");
		buffInit.m_size = sizeof(Vec3) * 8;
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		m_cubeVertsBuffer = GrManager::getSingleton().newBuffer(buffInit);

		Vec3* verts = static_cast<Vec3*>(m_cubeVertsBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		constexpr F32 kSize = 1.0f;
		verts[0] = Vec3(kSize, kSize, kSize); // front top right
		verts[1] = Vec3(-kSize, kSize, kSize); // front top left
		verts[2] = Vec3(-kSize, -kSize, kSize); // front bottom left
		verts[3] = Vec3(kSize, -kSize, kSize); // front bottom right
		verts[4] = Vec3(kSize, kSize, -kSize); // back top right
		verts[5] = Vec3(-kSize, kSize, -kSize); // back top left
		verts[6] = Vec3(-kSize, -kSize, -kSize); // back bottom left
		verts[7] = Vec3(kSize, -kSize, -kSize); // back bottom right

		m_cubeVertsBuffer->unmap();

		constexpr U kIndexCount = 12 * 2;
		buffInit.setName("Dbg cube indices");
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
		buffInit.m_size = kIndexCount * sizeof(U16);
		m_cubeIndicesBuffer = GrManager::getSingleton().newBuffer(buffInit);
		U16* indices = static_cast<U16*>(m_cubeIndicesBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		U c = 0;
		indices[c++] = 0;
		indices[c++] = 1;
		indices[c++] = 1;
		indices[c++] = 2;
		indices[c++] = 2;
		indices[c++] = 3;
		indices[c++] = 3;
		indices[c++] = 0;

		indices[c++] = 4;
		indices[c++] = 5;
		indices[c++] = 5;
		indices[c++] = 6;
		indices[c++] = 6;
		indices[c++] = 7;
		indices[c++] = 7;
		indices[c++] = 4;

		indices[c++] = 0;
		indices[c++] = 4;
		indices[c++] = 1;
		indices[c++] = 5;
		indices[c++] = 2;
		indices[c++] = 6;
		indices[c++] = 3;
		indices[c++] = 7;

		m_cubeIndicesBuffer->unmap();

		ANKI_ASSERT(c == kIndexCount);
	}

	initGizmos();

	return Error::kNone;
}

void Dbg::initGizmos()
{
	auto createPair = [](CString name, ConstWeakArray<F32> positionsArray, ConstWeakArray<U16> indicesArray, BufferPtr& positionsBuff,
						 BufferPtr& indicesBuff) {
		BufferInitInfo buffInit(name);
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_size = positionsArray.getSizeInBytes();
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
		positionsBuff = GrManager::getSingleton().newBuffer(buffInit);

		void* mapped = positionsBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
		memcpy(mapped, positionsArray.getBegin(), positionsArray.getSizeInBytes());
		positionsBuff->unmap();

		buffInit.m_size = indicesArray.getSizeInBytes();
		indicesBuff = GrManager::getSingleton().newBuffer(buffInit);

		mapped = indicesBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
		memcpy(mapped, indicesArray.getBegin(), indicesArray.getSizeInBytes());
		indicesBuff->unmap();
	};

	createPair("GizmosArrow", ConstWeakArray<F32>(&g_gizmoArrowPositions[0][0], sizeof(g_gizmoArrowPositions) / sizeof(F32)),
			   ConstWeakArray<U16>(&g_gizmoArrowIndices[0][0], sizeof(g_gizmoArrowIndices) / sizeof(U16)), m_gizmos.m_arrowPositions,
			   m_gizmos.m_arrowIndices);

	createPair("GizmosScale", ConstWeakArray<F32>(&g_gizmoScalePositions[0][0], sizeof(g_gizmoScalePositions) / sizeof(F32)),
			   ConstWeakArray<U16>(&g_gizmoScaleIndices[0][0], sizeof(g_gizmoScaleIndices) / sizeof(U16)), m_gizmos.m_scalePositions,
			   m_gizmos.m_scaleIndices);

	createPair("GizmosRing", ConstWeakArray<F32>(&g_gizmoRingPositions[0][0], sizeof(g_gizmoRingPositions) / sizeof(F32)),
			   ConstWeakArray<U16>(&g_gizmoRingIndices[0][0], sizeof(g_gizmoRingIndices) / sizeof(U16)), m_gizmos.m_ringPositions,
			   m_gizmos.m_ringIndices);
}

void Dbg::drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const RenderingContext& ctx, const ImageResource& image,
							CommandBuffer& cmdb)
{
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
	variantInitInfo.addMutation("OBJECT_TYPE", U32(type));
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Bilboards");
	const ShaderProgramResourceVariant* variant;
	m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
	cmdb.bindShaderProgram(&variant->getProgram());

	class Constants
	{
	public:
		Mat4 m_viewProjMat;
		Mat3x4 m_camTrf;

		UVec3 m_padding;
		U32 m_depthFailureVisualization;
	} consts;
	consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;
	consts.m_camTrf = ctx.m_matrices.m_cameraTransform;
	consts.m_depthFailureVisualization = !(m_options & DbgOption::kDepthTest);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindSrv(1, 0, getClusterBinning().getPackedObjectsBuffer(type));
	cmdb.bindSrv(2, 0, getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type));

	cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
	cmdb.bindSrv(3, 0, TextureView(&image.getTexture(), TextureSubresourceDesc::all()));
	cmdb.bindSrv(4, 0, TextureView(&m_spotLightImage->getTexture(), TextureSubresourceDesc::all()));

	cmdb.draw(PrimitiveTopology::kTriangles, 6, objCount);
}

void Dbg::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);

	m_runCtx.m_objectPickingRt = {};

	if(!isEnabled())
	{
		return;
	}

	// Debug visualization
	if(!!(m_options & (DbgOption::kDbgScene)))
	{
		populateRenderGraphMain(ctx);
	}

	// Object picking
	if(!!(m_options & DbgOption::kObjectPicking))
	{
		populateRenderGraphObjectPicking(ctx);
	}
}

void Dbg::populateRenderGraphMain(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Debug");

	GraphicsRenderPassTargetDesc colorRti(m_runCtx.m_rt);
	colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
	GraphicsRenderPassTargetDesc depthRti(getGBuffer().getDepthRt());
	depthRti.m_loadOperation = RenderTargetLoadOperation::kLoad;
	depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	pass.setRenderpassInfo({colorRti}, &depthRti);

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kRtvDsvWrite);
	pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel | TextureUsageBit::kRtvDsvRead);

	const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
	if(visOut.m_dependency.isValid())
	{
		pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kSrvGeometry);
	}

	const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();
	if(fvisOut.m_dependency.isValid())
	{
		pass.newBufferDependency(fvisOut.m_dependency, BufferUsageBit::kSrvGeometry);
	}

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(Dbg);
		ANKI_ASSERT(!!(m_options & DbgOption::kDbgScene));

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		// Set common state
		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		cmdb.setDepthWrite(false);

		cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
		cmdb.setDepthCompareOperation(!!(m_options & DbgOption::kDepthTest) ? CompareOperation::kLess : CompareOperation::kAlways);
		cmdb.setLineWidth(2.0f);

		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());

		// Common code for boxes stuff
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "RenderableBoxes");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			class Constants
			{
			public:
				Vec4 m_color;
				Mat4 m_viewProjMat;

				UVec3 m_padding;
				U32 m_depthFailureVisualization;
			} consts;
			consts.m_color = Vec4(1.0f, 1.0f, 0.0f, 1.0f);
			consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;
			consts.m_depthFailureVisualization = !(m_options & DbgOption::kDepthTest);

			cmdb.setFastConstants(&consts, sizeof(consts));
			cmdb.bindVertexBuffer(0, BufferView(m_cubeVertsBuffer.get()), sizeof(Vec3));
			cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
			cmdb.bindIndexBuffer(BufferView(m_cubeIndicesBuffer.get()), IndexType::kU16);
		}

		// GBuffer AABBs
		const U32 gbufferAllAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
		if(!!(m_options & DbgOption::kBoundingBoxes) && gbufferAllAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());

			const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
			cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, gbufferAllAabbCount);
		}

		// Forward shading renderables
		const U32 forwardAllAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
		if(!!(m_options & DbgOption::kBoundingBoxes) && forwardAllAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());

			const GpuVisibilityOutput& visOut = getForwardShading().getGpuVisibilityOutput();
			cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, forwardAllAabbCount);
		}

		// Draw non-renderables
		if(!!(m_options & DbgOption::kIcons))
		{
			drawNonRenderable(GpuSceneNonRenderableObjectType::kLight, GpuSceneArrays::Light::getSingleton().getElementCount(), ctx,
							  *m_pointLightImage, cmdb);
			drawNonRenderable(GpuSceneNonRenderableObjectType::kDecal, GpuSceneArrays::Decal::getSingleton().getElementCount(), ctx, *m_decalImage,
							  cmdb);
			drawNonRenderable(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe,
							  GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount(), ctx, *m_giProbeImage, cmdb);
			drawNonRenderable(GpuSceneNonRenderableObjectType::kReflectionProbe, GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount(),
							  ctx, *m_reflectionImage, cmdb);
		}

		// Physics
		if(!!(m_options & DbgOption::kPhysics))
		{
			class MyPhysicsDebugDrawerInterface final : public PhysicsDebugDrawerInterface
			{
			public:
				RendererDynamicArray<HVec4> m_positions;
				RendererDynamicArray<Array<U8, 4>> m_colors;

				void drawLines(ConstWeakArray<Vec3> lines, Array<U8, 4> color) override
				{
					static constexpr U32 kMaxVerts = 1024 * 100;

					for(const Vec3& pos : lines)
					{
						if(m_positions.getSize() >= kMaxVerts)
						{
							break;
						}

						m_positions.emplaceBack(HVec4(pos.xyz0()));
						m_colors.emplaceBack(color);
					}
				}
			} drawerInterface;

			PhysicsWorld::getSingleton().debugDraw(drawerInterface);

			const U32 vertCount = drawerInterface.m_positions.getSize();
			if(vertCount)
			{
				HVec4* positions;
				const BufferView positionBuff =
					RebarTransientMemoryPool::getSingleton().allocate(drawerInterface.m_positions.getSizeInBytes(), sizeof(HVec4), positions);
				memcpy(positions, drawerInterface.m_positions.getBegin(), drawerInterface.m_positions.getSizeInBytes());

				U8* colors;
				const BufferView colorBuff =
					RebarTransientMemoryPool::getSingleton().allocate(drawerInterface.m_colors.getSizeInBytes(), sizeof(U8) * 4, colors);
				memcpy(colors, drawerInterface.m_colors.getBegin(), drawerInterface.m_colors.getSizeInBytes());

				ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
				variantInitInfo.addMutation("OBJECT_TYPE", 0);
				variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Lines");
				const ShaderProgramResourceVariant* variant;
				m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
				cmdb.bindShaderProgram(&variant->getProgram());

				cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR16G16B16A16_Sfloat, 0);
				cmdb.setVertexAttribute(VertexAttributeSemantic::kColor, 1, Format::kR8G8B8A8_Unorm, 0);
				cmdb.bindVertexBuffer(0, positionBuff, sizeof(HVec4));
				cmdb.bindVertexBuffer(1, colorBuff, sizeof(U8) * 4);

				cmdb.setFastConstants(&ctx.m_matrices.m_viewProjection, sizeof(ctx.m_matrices.m_viewProjection));

				cmdb.draw(PrimitiveTopology::kLines, vertCount);
			}
		}

		if(m_gizmos.m_enabled)
		{
			cmdb.setDepthCompareOperation(CompareOperation::kAlways);
			drawGizmos(m_gizmos.m_trf, ctx, cmdb);
		}

		// Restore state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
		cmdb.setDepthCompareOperation(CompareOperation::kLess);
		cmdb.setDepthWrite(true);
	});
}

void Dbg::populateRenderGraphObjectPicking(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
	const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();

	U32 maxVisibleCount = 0;
	if(visOut.containsDrawcalls())
	{
		maxVisibleCount += U32(visOut.m_visibleAaabbIndicesBuffer.getRange() / sizeof(LodAndGpuSceneRenderableBoundingVolumeIndex));
	}
	if(fvisOut.containsDrawcalls())
	{
		maxVisibleCount += U32(fvisOut.m_visibleAaabbIndicesBuffer.getRange() / sizeof(LodAndGpuSceneRenderableBoundingVolumeIndex));
	}
	const BufferView drawIndirectArgsBuff =
		GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<DrawIndexedIndirectArgs>(maxVisibleCount);

	const BufferView drawCountBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<U32>(1);
	const BufferHandle bufferHandle = rgraph.importBuffer(drawCountBuff, BufferUsageBit::kNone);

	const BufferView lodAndRenderableIndicesBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<U32>(maxVisibleCount);

	// Zero draw count
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Zero");

		pass.newBufferDependency(bufferHandle, BufferUsageBit::kCopyDestination);

		pass.setWork([drawCountBuff, lodAndRenderableIndicesBuff, drawIndirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			cmdb.zeroBuffer(drawCountBuff);
			cmdb.zeroBuffer(lodAndRenderableIndicesBuff);
			cmdb.zeroBuffer(drawIndirectArgsBuff);
		});
	}

	// Prepare pass
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Prepare");

		if(visOut.m_dependency.isValid())
		{
			pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kSrvCompute);
		}

		if(fvisOut.m_dependency.isValid())
		{
			pass.newBufferDependency(fvisOut.m_dependency, BufferUsageBit::kSrvCompute);
		}

		pass.newBufferDependency(bufferHandle, BufferUsageBit::kUavCompute);

		pass.setWork([this, drawIndirectArgsBuff, drawCountBuff, lodAndRenderableIndicesBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "PrepareRenderableUuids");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(2, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());

			cmdb.bindUav(0, 0, drawIndirectArgsBuff);
			cmdb.bindUav(1, 0, drawCountBuff);
			cmdb.bindUav(2, 0, lodAndRenderableIndicesBuff);

			// Do GBuffer
			const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
			if(visOut.containsDrawcalls())
			{
				cmdb.bindSrv(0, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());
				cmdb.bindSrv(3, 0, visOut.m_visibleAaabbIndicesBuffer);

				const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
				cmdb.dispatchCompute((allAabbCount + 63) / 64, 1, 1);
			}

			// Do ForwardShading
			const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();
			if(fvisOut.containsDrawcalls())
			{
				cmdb.bindSrv(0, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());
				cmdb.bindSrv(3, 0, fvisOut.m_visibleAaabbIndicesBuffer);

				const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
				cmdb.dispatchCompute((allAabbCount + 63) / 64, 1, 1);
			}
		});
	}

	// The render pass that draws the UUIDs to a buffer
	const RenderTargetHandle objectPickingRt = rgraph.newRenderTarget(m_objectPickingRtDescr);
	m_runCtx.m_objectPickingRt = objectPickingRt;
	const RenderTargetHandle objectPickingDepthRt = rgraph.newRenderTarget(m_objectPickingDepthRtDescr);
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Object Picking: Draw UUIDs");

		pass.newBufferDependency(bufferHandle, BufferUsageBit::kIndirectDraw);
		pass.newTextureDependency(objectPickingRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(objectPickingDepthRt, TextureUsageBit::kRtvDsvWrite);

		GraphicsRenderPassTargetDesc colorRti(objectPickingRt);
		colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
		GraphicsRenderPassTargetDesc depthRti(objectPickingDepthRt);
		depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
		depthRti.m_clearValue.m_depthStencil.m_depth = 1.0;
		depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
		pass.setRenderpassInfo({colorRti}, &depthRti);

		pass.setWork(
			[this, lodAndRenderableIndicesBuff, &ctx, drawIndirectArgsBuff, drawCountBuff, maxVisibleCount](RenderPassWorkContext& rgraphCtx) {
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				// Set common state
				cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
				cmdb.setDepthCompareOperation(CompareOperation::kLess);

				ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
				variantInitInfo.addMutation("OBJECT_TYPE", 0);
				variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "RenderableUuids");
				const ShaderProgramResourceVariant* variant;
				m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
				cmdb.bindShaderProgram(&variant->getProgram());

				cmdb.bindIndexBuffer(UnifiedGeometryBuffer::getSingleton().getBufferView(), IndexType::kU16);

				cmdb.bindSrv(0, 0, lodAndRenderableIndicesBuff);
				cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
				cmdb.bindSrv(2, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
				cmdb.bindSrv(3, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());
				cmdb.bindSrv(4, 0, UnifiedGeometryBuffer::getSingleton().getBufferView(), Format::kR16G16B16A16_Unorm);
				cmdb.bindSrv(5, 0, UnifiedGeometryBuffer::getSingleton().getBufferView(), Format::kR8G8B8A8_Uint);
				cmdb.bindSrv(6, 0, UnifiedGeometryBuffer::getSingleton().getBufferView(), Format::kR8G8B8A8_Snorm);
				cmdb.bindSrv(7, 0, GpuSceneBuffer::getSingleton().getBufferView());

				cmdb.setFastConstants(&ctx.m_matrices.m_viewProjection, sizeof(ctx.m_matrices.m_viewProjection));

				cmdb.drawIndexedIndirectCount(PrimitiveTopology::kTriangles, drawIndirectArgsBuff, sizeof(DrawIndexedIndirectArgs), drawCountBuff,
											  maxVisibleCount);
			});
	}

	// Read the UUID RT to get the UUID that is over the mouse
	{
		U32 uuid;
		PtrSize dataOut;
		getRenderer().getReadbackManager().readMostRecentData(m_readback, &uuid, sizeof(uuid), dataOut);
		if(dataOut)
		{
			m_runCtx.m_objUuid = uuid;
		}
		else
		{
			m_runCtx.m_objUuid = 0;
		}

		const BufferView readbackBuff = getRenderer().getReadbackManager().allocateStructuredBuffer<U32>(m_readback, 1);

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Picking");

		pass.newTextureDependency(objectPickingRt, TextureUsageBit::kSrvCompute);

		pass.setWork([this, readbackBuff, objectPickingRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "RenderableUuidsPick");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			rgraphCtx.bindSrv(0, 0, objectPickingRt);
			cmdb.bindUav(0, 0, readbackBuff);

			Vec2 mousePos = Input::getSingleton().getMousePositionNdc();
			mousePos.y() = -mousePos.y();
			mousePos = mousePos / 2.0f + 0.5f;
			mousePos *= Vec2(getRenderer().getInternalResolution() / 2);

			const UVec4 consts(UVec2(mousePos), 0u, 0u);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute(1, 1, 1);
		});
	}
}

void Dbg::drawGizmos(const Mat3x4& worldTransform, const RenderingContext& ctx, CommandBuffer& cmdb) const
{
	auto draw = [&](Vec4 color, Euler rotation, Vec3 scale, Buffer& positionsBuff, Buffer& indexBuff) {
		struct Consts
		{
			Mat4 m_mvp;
			Vec4 m_color;
		} consts;
		consts.m_mvp = ctx.m_matrices.m_viewProjection * Mat4(worldTransform, Vec4(0.0f, 0.0f, 0.0f, 1.0f)) * Mat4(Vec3(0.0f), Mat3(rotation), scale);
		consts.m_color = color;
		cmdb.setFastConstants(&consts, sizeof(consts));

		cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
		cmdb.bindVertexBuffer(0, BufferView(&positionsBuff), sizeof(Vec3));

		cmdb.bindIndexBuffer(BufferView(&indexBuff), IndexType::kU16);

		cmdb.drawIndexed(PrimitiveTopology::kTriangles, U32(indexBuff.getSize() / sizeof(U16)));
	};

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
	variantInitInfo.addMutation("OBJECT_TYPE", 0);
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Gizmos");
	const ShaderProgramResourceVariant* variant;
	m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
	cmdb.bindShaderProgram(&variant->getProgram());

	const F32 alpha = 1.0f;
	draw(Vec4(1.0f, 0.0f, 0.0f, alpha), Euler(0.0f, kPi / 2.0f, 0.0f), Vec3(1.0f), *m_gizmos.m_arrowPositions, *m_gizmos.m_arrowIndices);
	draw(Vec4(1.0f, 0.0f, 0.0f, alpha), Euler(0.0f, kPi / 2.0f, 0.0f), Vec3(0.95f, 0.95f, 0.6f), *m_gizmos.m_scalePositions,
		 *m_gizmos.m_scaleIndices);
	draw(Vec4(1.0f, 0.0f, 0.0f, alpha), Euler(0.0f, kPi / 2.0f, 0.0f), Vec3(0.4f), *m_gizmos.m_ringPositions, *m_gizmos.m_ringIndices);

	draw(Vec4(0.0f, 1.0f, 0.0f, alpha), Euler(-kPi / 2.0f, 0.0f, 0.0f), Vec3(1.0f), *m_gizmos.m_arrowPositions, *m_gizmos.m_arrowIndices);
	draw(Vec4(0.0f, 1.0f, 0.0f, alpha), Euler(-kPi / 2.0f, 0.0f, 0.0f), Vec3(0.95f, 0.95f, 0.6f), *m_gizmos.m_scalePositions,
		 *m_gizmos.m_scaleIndices);
	draw(Vec4(0.0f, 1.0f, 0.0f, alpha), Euler(-kPi / 2.0f, 0.0f, 0.0f), Vec3(0.4f), *m_gizmos.m_ringPositions, *m_gizmos.m_ringIndices);

	draw(Vec4(0.0f, 0.0f, 1.0f, alpha), Euler(0.0f, 0.0f, 0.0f), Vec3(1.0f), *m_gizmos.m_arrowPositions, *m_gizmos.m_arrowIndices);
	draw(Vec4(0.0f, 0.0f, 1.0f, alpha), Euler(0.0f, 0.0f, 0.0f), Vec3(0.95f, 0.95f, 0.6f), *m_gizmos.m_scalePositions, *m_gizmos.m_scaleIndices);
	draw(Vec4(0.0f, 0.0f, 1.0f, alpha), Euler(0.0f, 0.0f, 0.0f), Vec3(0.4f), *m_gizmos.m_ringPositions, *m_gizmos.m_ringIndices);
}

} // end namespace anki
