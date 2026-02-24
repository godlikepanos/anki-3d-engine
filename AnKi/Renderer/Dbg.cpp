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
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
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

static const F32 g_gizmoArrowPositions[13][3] = {{-0.016779f, -0.016779f, 0.779639f},
												 {-0.016779f, 0.016779f, 0.779639f},
												 {-0.016779f, -0.016779f, 0.0f},
												 {-0.016779f, 0.016779f, 0.0f},
												 {0.016779f, -0.016779f, 0.779639f},
												 {0.016779f, 0.016779f, 0.779639f},
												 {0.016779f, -0.016779f, 0.0f},
												 {0.016779f, 0.016779f, 0.0f},
												 {-0.056151f, -0.056151f, 0.787911f},
												 {-0.056151f, 0.056151f, 0.787911f},
												 {0.056151f, -0.056151f, 0.787911f},
												 {0.056151f, 0.056151f, 0.787911f},
												 {0.0f, 0.0f, 1.0f}};

static const U16 g_gizmoArrowIndices[22][3] = {{1, 2, 0},  {3, 6, 2},  {7, 4, 6},  {1, 8, 9},   {6, 0, 2},    {3, 5, 7},  {8, 10, 12}, {4, 8, 0},
											   {1, 11, 5}, {5, 10, 4}, {9, 8, 12}, {11, 9, 12}, {10, 11, 12}, {1, 3, 2},  {3, 7, 6},   {7, 5, 4},
											   {1, 0, 8},  {6, 4, 0},  {3, 1, 5},  {4, 10, 8},  {1, 9, 11},   {5, 11, 10}};

static const F32 g_gizmoScalePositions[8][3] = {
	{-0.056151f, -0.056151f, 0.651073f}, {-0.056151f, 0.056151f, 0.651073f}, {-0.056151f, -0.056151f, 0.520279f}, {-0.056151f, 0.056151f, 0.520279f},
	{0.056151f, -0.056151f, 0.651073f},  {0.056151f, 0.056151f, 0.651073f},  {0.056151f, -0.056151f, 0.520279f},  {0.056151f, 0.056151f, 0.520279f}};

static const U16 g_gizmoScaleIndices[12][3] = {
	{1, 2, 0}, {3, 6, 2}, {7, 4, 6}, {5, 0, 4}, {6, 0, 2}, {3, 5, 7}, {1, 3, 2}, {3, 7, 6}, {7, 5, 4}, {5, 1, 0}, {6, 4, 0}, {3, 1, 5},
};

static const F32 g_gizmoRingPositions[256][3] = {{0.0f, 1.0f, -0.005991f},
												 {0.0f, 1.0f, 0.005991f},
												 {0.098017f, 0.995185f, -0.005991f},
												 {0.098017f, 0.995185f, 0.005991f},
												 {0.19509f, 0.980785f, -0.005991f},
												 {0.19509f, 0.980785f, 0.005991f},
												 {0.290285f, 0.95694f, -0.005991f},
												 {0.290285f, 0.95694f, 0.005991f},
												 {0.382683f, 0.92388f, -0.005991f},
												 {0.382683f, 0.923879f, 0.005991f},
												 {0.471397f, 0.881921f, -0.005991f},
												 {0.471397f, 0.881921f, 0.005991f},
												 {0.55557f, 0.83147f, -0.005991f},
												 {0.55557f, 0.83147f, 0.005991f},
												 {0.634393f, 0.77301f, -0.005991f},
												 {0.634393f, 0.77301f, 0.005991f},
												 {0.707107f, 0.707107f, -0.005991f},
												 {0.707107f, 0.707107f, 0.005991f},
												 {0.77301f, 0.634393f, -0.005991f},
												 {0.77301f, 0.634393f, 0.005991f},
												 {0.83147f, 0.55557f, -0.005991f},
												 {0.83147f, 0.55557f, 0.005991f},
												 {0.881921f, 0.471397f, -0.005991f},
												 {0.881921f, 0.471397f, 0.005991f},
												 {0.92388f, 0.382683f, -0.005991f},
												 {0.92388f, 0.382683f, 0.005991f},
												 {0.95694f, 0.290285f, -0.005991f},
												 {0.95694f, 0.290285f, 0.005991f},
												 {0.980785f, 0.19509f, -0.005991f},
												 {0.980785f, 0.19509f, 0.005991f},
												 {0.995185f, 0.098017f, -0.005991f},
												 {0.995185f, 0.098017f, 0.005991f},
												 {1.0f, 0.0f, -0.005991f},
												 {1.0f, -0.0f, 0.005991f},
												 {0.995185f, -0.098017f, -0.005991f},
												 {0.995185f, -0.098017f, 0.005991f},
												 {0.980785f, -0.19509f, -0.005991f},
												 {0.980785f, -0.19509f, 0.005991f},
												 {0.95694f, -0.290285f, -0.005991f},
												 {0.95694f, -0.290285f, 0.005991f},
												 {0.92388f, -0.382683f, -0.005991f},
												 {0.92388f, -0.382683f, 0.005991f},
												 {0.881921f, -0.471397f, -0.005991f},
												 {0.881921f, -0.471397f, 0.005991f},
												 {0.83147f, -0.55557f, -0.005991f},
												 {0.83147f, -0.55557f, 0.005991f},
												 {0.77301f, -0.634393f, -0.005991f},
												 {0.77301f, -0.634393f, 0.005991f},
												 {0.707107f, -0.707107f, -0.005991f},
												 {0.707107f, -0.707107f, 0.005991f},
												 {0.634393f, -0.77301f, -0.005991f},
												 {0.634393f, -0.77301f, 0.005991f},
												 {0.55557f, -0.83147f, -0.005991f},
												 {0.55557f, -0.83147f, 0.005991f},
												 {0.471397f, -0.881921f, -0.005991f},
												 {0.471397f, -0.881921f, 0.005991f},
												 {0.382683f, -0.923879f, -0.005991f},
												 {0.382683f, -0.92388f, 0.005991f},
												 {0.290285f, -0.95694f, -0.005991f},
												 {0.290285f, -0.95694f, 0.005991f},
												 {0.19509f, -0.980785f, -0.005991f},
												 {0.19509f, -0.980785f, 0.005991f},
												 {0.098017f, -0.995185f, -0.005991f},
												 {0.098017f, -0.995185f, 0.005991f},
												 {0.0f, -1.0f, -0.005991f},
												 {0.0f, -1.0f, 0.005991f},
												 {-0.098017f, -0.995185f, -0.005991f},
												 {-0.098017f, -0.995185f, 0.005991f},
												 {-0.19509f, -0.980785f, -0.005991f},
												 {-0.19509f, -0.980785f, 0.005991f},
												 {-0.290285f, -0.95694f, -0.005991f},
												 {-0.290285f, -0.95694f, 0.005991f},
												 {-0.382683f, -0.923879f, -0.005991f},
												 {-0.382683f, -0.92388f, 0.005991f},
												 {-0.471397f, -0.881921f, -0.005991f},
												 {-0.471397f, -0.881921f, 0.005991f},
												 {-0.55557f, -0.83147f, -0.005991f},
												 {-0.55557f, -0.83147f, 0.005991f},
												 {-0.634393f, -0.77301f, -0.005991f},
												 {-0.634393f, -0.77301f, 0.005991f},
												 {-0.707107f, -0.707107f, -0.005991f},
												 {-0.707107f, -0.707107f, 0.005991f},
												 {-0.77301f, -0.634393f, -0.005991f},
												 {-0.77301f, -0.634393f, 0.005991f},
												 {-0.83147f, -0.55557f, -0.005991f},
												 {-0.83147f, -0.55557f, 0.005991f},
												 {-0.881921f, -0.471397f, -0.005991f},
												 {-0.881921f, -0.471397f, 0.005991f},
												 {-0.92388f, -0.382683f, -0.005991f},
												 {-0.92388f, -0.382683f, 0.005991f},
												 {-0.95694f, -0.290285f, -0.005991f},
												 {-0.95694f, -0.290285f, 0.005991f},
												 {-0.980785f, -0.19509f, -0.005991f},
												 {-0.980785f, -0.19509f, 0.005991f},
												 {-0.995185f, -0.098017f, -0.005991f},
												 {-0.995185f, -0.098017f, 0.005991f},
												 {-1.0f, 0.0f, -0.005991f},
												 {-1.0f, -0.0f, 0.005991f},
												 {-0.995185f, 0.098017f, -0.005991f},
												 {-0.995185f, 0.098017f, 0.005991f},
												 {-0.980785f, 0.19509f, -0.005991f},
												 {-0.980785f, 0.19509f, 0.005991f},
												 {-0.95694f, 0.290285f, -0.005991f},
												 {-0.95694f, 0.290285f, 0.005991f},
												 {-0.92388f, 0.382683f, -0.005991f},
												 {-0.92388f, 0.382683f, 0.005991f},
												 {-0.881921f, 0.471397f, -0.005991f},
												 {-0.881921f, 0.471397f, 0.005991f},
												 {-0.83147f, 0.55557f, -0.005991f},
												 {-0.83147f, 0.55557f, 0.005991f},
												 {-0.77301f, 0.634393f, -0.005991f},
												 {-0.77301f, 0.634393f, 0.005991f},
												 {-0.707107f, 0.707107f, -0.005991f},
												 {-0.707107f, 0.707107f, 0.005991f},
												 {-0.634393f, 0.77301f, -0.005991f},
												 {-0.634393f, 0.77301f, 0.005991f},
												 {-0.55557f, 0.83147f, -0.005991f},
												 {-0.55557f, 0.83147f, 0.005991f},
												 {-0.471397f, 0.881921f, -0.005991f},
												 {-0.471397f, 0.881921f, 0.005991f},
												 {-0.382683f, 0.92388f, -0.005991f},
												 {-0.382683f, 0.923879f, 0.005991f},
												 {-0.290285f, 0.95694f, -0.005991f},
												 {-0.290285f, 0.95694f, 0.005991f},
												 {-0.19509f, 0.980785f, -0.005991f},
												 {-0.19509f, 0.980785f, 0.005991f},
												 {-0.098017f, 0.995185f, -0.005991f},
												 {-0.098017f, 0.995185f, 0.005991f},
												 {-0.695875f, 0.464969f, 0.005991f},
												 {-0.738099f, 0.394522f, 0.005991f},
												 {-0.530937f, 0.646949f, 0.005991f},
												 {-0.591793f, 0.591793f, 0.005991f},
												 {-0.646949f, 0.530937f, 0.005991f},
												 {-0.320276f, 0.773215f, 0.005991f},
												 {-0.394522f, 0.738099f, 0.005991f},
												 {-0.163275f, 0.82084f, 0.005991f},
												 {-0.082033f, 0.832891f, 0.005991f},
												 {-0.242945f, 0.800884f, 0.005991f},
												 {-0.464969f, 0.695875f, 0.005991f},
												 {-0.82084f, -0.163275f, 0.005991f},
												 {-0.832891f, -0.082033f, 0.005991f},
												 {-0.800884f, -0.242945f, 0.005991f},
												 {-0.738099f, -0.394522f, 0.005991f},
												 {-0.773215f, -0.320276f, 0.005991f},
												 {-0.591793f, -0.591793f, 0.005991f},
												 {-0.646949f, -0.530937f, 0.005991f},
												 {-0.464969f, -0.695875f, 0.005991f},
												 {-0.394522f, -0.738099f, 0.005991f},
												 {-0.530937f, -0.646949f, 0.005991f},
												 {-0.695875f, -0.464969f, 0.005991f},
												 {0.163275f, -0.82084f, 0.005991f},
												 {0.082033f, -0.832891f, 0.005991f},
												 {0.394522f, -0.738099f, 0.005991f},
												 {0.320276f, -0.773215f, 0.005991f},
												 {0.242945f, -0.800884f, 0.005991f},
												 {0.530937f, -0.646949f, 0.005991f},
												 {0.591793f, -0.591793f, 0.005991f},
												 {0.646949f, -0.530937f, 0.005991f},
												 {0.695875f, -0.464969f, 0.005991f},
												 {0.738099f, -0.394522f, 0.005991f},
												 {0.464969f, -0.695875f, 0.005991f},
												 {0.464969f, 0.695875f, 0.005991f},
												 {0.394522f, 0.738099f, 0.005991f},
												 {0.646949f, 0.530937f, 0.005991f},
												 {0.591793f, 0.591793f, 0.005991f},
												 {0.530937f, 0.646949f, 0.005991f},
												 {0.738099f, 0.394522f, 0.005991f},
												 {0.800884f, 0.242945f, 0.005991f},
												 {0.773215f, 0.320276f, 0.005991f},
												 {0.832891f, 0.082033f, 0.005991f},
												 {0.82084f, 0.163275f, 0.005991f},
												 {0.695875f, 0.464969f, 0.005991f},
												 {-0.836921f, -0.0f, 0.005991f},
												 {-0.832891f, 0.082033f, 0.005991f},
												 {-0.82084f, 0.163275f, 0.005991f},
												 {-0.800884f, 0.242945f, 0.005991f},
												 {-0.773215f, 0.320276f, 0.005991f},
												 {-0.242945f, -0.800884f, 0.005991f},
												 {-0.163275f, -0.82084f, 0.005991f},
												 {0.163275f, 0.82084f, 0.005991f},
												 {0.082033f, 0.832891f, 0.005991f},
												 {0.0f, 0.836921f, 0.005991f},
												 {-0.320276f, -0.773215f, 0.005991f},
												 {0.0f, -0.836921f, 0.005991f},
												 {-0.082033f, -0.832891f, 0.005991f},
												 {0.242945f, 0.800884f, 0.005991f},
												 {0.320276f, 0.773215f, 0.005991f},
												 {0.800884f, -0.242945f, 0.005991f},
												 {0.773215f, -0.320276f, 0.005991f},
												 {0.836921f, -0.0f, 0.005991f},
												 {0.82084f, -0.163275f, 0.005991f},
												 {0.832891f, -0.082033f, 0.005991f},
												 {0.242945f, 0.800884f, -0.005991f},
												 {0.163275f, 0.82084f, -0.005991f},
												 {0.464969f, 0.695875f, -0.005991f},
												 {0.394522f, 0.738099f, -0.005991f},
												 {0.320276f, 0.773215f, -0.005991f},
												 {0.591793f, 0.591793f, -0.005991f},
												 {0.646949f, 0.530937f, -0.005991f},
												 {0.695875f, 0.464969f, -0.005991f},
												 {0.738099f, 0.394522f, -0.005991f},
												 {0.773215f, 0.320276f, -0.005991f},
												 {0.530937f, 0.646949f, -0.005991f},
												 {0.394522f, -0.738099f, -0.005991f},
												 {0.320276f, -0.773215f, -0.005991f},
												 {0.591793f, -0.591793f, -0.005991f},
												 {0.530937f, -0.646949f, -0.005991f},
												 {0.464969f, -0.695875f, -0.005991f},
												 {0.695875f, -0.464969f, -0.005991f},
												 {0.738099f, -0.394522f, -0.005991f},
												 {0.773215f, -0.320276f, -0.005991f},
												 {0.800884f, -0.242945f, -0.005991f},
												 {0.82084f, -0.163275f, -0.005991f},
												 {0.646949f, -0.530937f, -0.005991f},
												 {-0.738099f, -0.394522f, -0.005991f},
												 {-0.773215f, -0.320276f, -0.005991f},
												 {-0.591793f, -0.591793f, -0.005991f},
												 {-0.695875f, -0.464969f, -0.005991f},
												 {-0.646949f, -0.530937f, -0.005991f},
												 {-0.394522f, -0.738099f, -0.005991f},
												 {-0.464969f, -0.695875f, -0.005991f},
												 {-0.242945f, -0.800884f, -0.005991f},
												 {-0.163275f, -0.82084f, -0.005991f},
												 {-0.320276f, -0.773215f, -0.005991f},
												 {-0.530937f, -0.646949f, -0.005991f},
												 {-0.800884f, 0.242945f, -0.005991f},
												 {-0.82084f, 0.163275f, -0.005991f},
												 {-0.773215f, 0.320276f, -0.005991f},
												 {-0.695875f, 0.464969f, -0.005991f},
												 {-0.738099f, 0.394522f, -0.005991f},
												 {-0.530937f, 0.646949f, -0.005991f},
												 {-0.591793f, 0.591793f, -0.005991f},
												 {-0.394522f, 0.738099f, -0.005991f},
												 {-0.320276f, 0.773215f, -0.005991f},
												 {-0.464969f, 0.695875f, -0.005991f},
												 {-0.646949f, 0.530937f, -0.005991f},
												 {0.0f, 0.836921f, -0.005991f},
												 {0.082033f, 0.832891f, -0.005991f},
												 {0.163275f, -0.82084f, -0.005991f},
												 {0.242945f, -0.800884f, -0.005991f},
												 {0.82084f, 0.163275f, -0.005991f},
												 {0.800884f, 0.242945f, -0.005991f},
												 {0.836921f, 0.0f, -0.005991f},
												 {0.832891f, -0.082033f, -0.005991f},
												 {0.832891f, 0.082033f, -0.005991f},
												 {-0.836921f, 0.0f, -0.005991f},
												 {-0.832891f, -0.082033f, -0.005991f},
												 {-0.832891f, 0.082033f, -0.005991f},
												 {-0.82084f, -0.163275f, -0.005991f},
												 {-0.800884f, -0.242945f, -0.005991f},
												 {-0.163275f, 0.82084f, -0.005991f},
												 {-0.082033f, 0.832891f, -0.005991f},
												 {0.082033f, -0.832891f, -0.005991f},
												 {0.0f, -0.836921f, -0.005991f},
												 {-0.082033f, -0.832891f, -0.005991f},
												 {-0.242945f, 0.800884f, -0.005991f}};

static const U16 g_gizmoRingIndices[512][3] = {
	{1, 2, 0},       {3, 4, 2},       {5, 6, 4},       {7, 8, 6},       {9, 10, 8},      {11, 12, 10},    {13, 14, 12},    {15, 16, 14},
	{17, 18, 16},    {19, 20, 18},    {21, 22, 20},    {23, 24, 22},    {25, 26, 24},    {27, 28, 26},    {29, 30, 28},    {31, 32, 30},
	{33, 34, 32},    {35, 36, 34},    {37, 38, 36},    {39, 40, 38},    {41, 42, 40},    {43, 44, 42},    {45, 46, 44},    {47, 48, 46},
	{49, 50, 48},    {51, 52, 50},    {53, 54, 52},    {55, 56, 54},    {57, 58, 56},    {59, 60, 58},    {61, 62, 60},    {63, 64, 62},
	{64, 67, 66},    {66, 69, 68},    {68, 71, 70},    {70, 73, 72},    {72, 75, 74},    {74, 77, 76},    {76, 79, 78},    {78, 81, 80},
	{80, 83, 82},    {82, 85, 84},    {84, 87, 86},    {86, 89, 88},    {88, 91, 90},    {90, 93, 92},    {92, 95, 94},    {94, 97, 96},
	{96, 99, 98},    {98, 101, 100},  {100, 103, 102}, {102, 105, 104}, {104, 107, 106}, {106, 109, 108}, {108, 111, 110}, {110, 113, 112},
	{112, 115, 114}, {114, 117, 116}, {116, 119, 118}, {118, 121, 120}, {120, 123, 122}, {122, 125, 124}, {63, 150, 151},  {133, 137, 121},
	{124, 127, 126}, {126, 1, 0},     {193, 6, 192},   {219, 223, 74},  {237, 181, 236}, {193, 180, 237}, {192, 179, 193}, {196, 185, 192},
	{162, 196, 195}, {161, 195, 194}, {202, 161, 194}, {197, 165, 202}, {164, 198, 163}, {163, 199, 171}, {171, 200, 166}, {166, 201, 168},
	{168, 241, 167}, {167, 240, 170}, {170, 244, 169}, {169, 242, 189}, {189, 243, 191}, {191, 212, 190}, {190, 211, 187}, {187, 210, 188},
	{188, 209, 159}, {159, 208, 158}, {158, 213, 157}, {157, 205, 156}, {156, 206, 155}, {155, 207, 160}, {160, 203, 152}, {152, 204, 153},
	{153, 239, 154}, {154, 238, 150}, {150, 252, 151}, {151, 253, 183}, {184, 253, 254}, {178, 254, 222}, {177, 222, 221}, {182, 221, 223},
	{182, 219, 147}, {147, 220, 146}, {148, 220, 224}, {144, 224, 216}, {145, 216, 218}, {149, 218, 217}, {142, 217, 214}, {143, 214, 215},
	{141, 215, 249}, {139, 249, 248}, {140, 248, 246}, {172, 246, 245}, {173, 245, 247}, {174, 247, 226}, {175, 226, 225}, {176, 225, 227},
	{129, 227, 229}, {128, 229, 228}, {132, 228, 235}, {131, 235, 231}, {130, 231, 230}, {138, 230, 234}, {134, 234, 232}, {133, 232, 233},
	{137, 233, 255}, {135, 255, 250}, {136, 250, 251}, {181, 251, 236}, {1, 3, 2},       {3, 5, 4},       {5, 7, 6},       {7, 9, 8},
	{9, 11, 10},     {11, 13, 12},    {13, 15, 14},    {15, 17, 16},    {17, 19, 18},    {19, 21, 20},    {21, 23, 22},    {23, 25, 24},
	{25, 27, 26},    {27, 29, 28},    {29, 31, 30},    {31, 33, 32},    {33, 35, 34},    {35, 37, 36},    {37, 39, 38},    {39, 41, 40},
	{41, 43, 42},    {43, 45, 44},    {45, 47, 46},    {47, 49, 48},    {49, 51, 50},    {51, 53, 52},    {53, 55, 54},    {55, 57, 56},
	{57, 59, 58},    {59, 61, 60},    {61, 63, 62},    {63, 65, 64},    {64, 65, 67},    {66, 67, 69},    {68, 69, 71},    {70, 71, 73},
	{72, 73, 75},    {74, 75, 77},    {76, 77, 79},    {78, 79, 81},    {80, 81, 83},    {82, 83, 85},    {84, 85, 87},    {86, 87, 89},
	{88, 89, 91},    {90, 91, 93},    {92, 93, 95},    {94, 95, 97},    {96, 97, 99},    {98, 99, 101},   {100, 101, 103}, {102, 103, 105},
	{104, 105, 107}, {106, 107, 109}, {108, 109, 111}, {110, 111, 113}, {112, 113, 115}, {114, 115, 117}, {116, 117, 119}, {118, 119, 121},
	{120, 121, 123}, {122, 123, 125}, {138, 117, 115}, {115, 113, 130}, {113, 111, 131}, {130, 113, 131}, {111, 109, 128}, {109, 107, 129},
	{107, 105, 129}, {105, 103, 176}, {129, 105, 176}, {103, 101, 175}, {101, 99, 174},  {175, 101, 174}, {99, 97, 173},   {97, 95, 172},
	{173, 97, 172},  {95, 93, 139},   {93, 91, 141},   {91, 89, 143},   {89, 87, 143},   {87, 85, 142},   {85, 83, 149},   {142, 85, 149},
	{83, 81, 145},   {81, 79, 144},   {145, 81, 144},  {79, 77, 146},   {77, 75, 147},   {75, 73, 147},   {73, 71, 182},   {147, 73, 182},
	{71, 69, 177},   {69, 67, 178},   {177, 69, 178},  {67, 65, 184},   {65, 63, 183},   {184, 65, 183},  {63, 61, 150},   {61, 59, 154},
	{59, 57, 153},   {57, 55, 152},   {55, 53, 160},   {53, 51, 160},   {51, 49, 155},   {49, 47, 156},   {155, 49, 156},  {47, 45, 158},
	{45, 43, 159},   {43, 41, 188},   {41, 39, 187},   {39, 37, 190},   {37, 35, 191},   {35, 33, 191},   {33, 31, 189},   {191, 33, 189},
	{31, 29, 170},   {29, 27, 167},   {27, 25, 168},   {25, 23, 166},   {23, 21, 171},   {21, 19, 171},   {19, 17, 163},   {17, 15, 164},
	{163, 17, 164},  {15, 13, 161},   {13, 11, 162},   {11, 9, 186},    {9, 7, 185},     {7, 5, 185},     {5, 179, 185},   {13, 162, 161},
	{9, 185, 186},   {171, 19, 163},  {29, 167, 170},  {25, 166, 168},  {37, 191, 190},  {45, 159, 158},  {41, 187, 188},  {160, 51, 155},
	{61, 154, 150},  {57, 152, 153},  {77, 147, 146},  {93, 141, 139},  {109, 129, 128}, {138, 115, 130}, {91, 143, 141},  {143, 87, 142},
	{11, 186, 162},  {132, 131, 111}, {128, 132, 111}, {15, 161, 165},  {15, 165, 164},  {176, 103, 175}, {174, 99, 173},  {23, 171, 166},
	{27, 168, 167},  {140, 172, 95},  {139, 140, 95},  {31, 170, 169},  {31, 169, 189},  {149, 83, 145},  {39, 190, 187},  {43, 188, 159},
	{148, 144, 79},  {146, 148, 79},  {47, 158, 157},  {47, 157, 156},  {182, 71, 177},  {178, 67, 184},  {55, 160, 152},  {59, 153, 154},
	{151, 183, 63},  {180, 179, 5},   {5, 3, 180},     {3, 1, 181},     {1, 127, 136},   {127, 125, 135}, {125, 123, 135}, {123, 121, 137},
	{135, 123, 137}, {121, 119, 133}, {119, 117, 134}, {133, 119, 134}, {117, 138, 134}, {3, 181, 180},   {127, 135, 136}, {136, 181, 1},
	{124, 125, 127}, {126, 127, 1},   {216, 224, 78},  {78, 80, 216},   {80, 82, 218},   {82, 84, 217},   {84, 86, 217},   {86, 88, 214},
	{88, 90, 215},   {214, 88, 215},  {90, 92, 249},   {92, 94, 248},   {249, 92, 248},  {94, 96, 245},   {96, 98, 247},   {98, 100, 247},
	{100, 102, 226}, {247, 100, 226}, {102, 104, 225}, {104, 106, 227}, {225, 104, 227}, {106, 108, 229}, {108, 110, 228}, {229, 108, 228},
	{110, 112, 231}, {112, 114, 230}, {114, 116, 234}, {116, 118, 234}, {118, 120, 232}, {120, 122, 233}, {232, 120, 233}, {122, 124, 255},
	{124, 126, 250}, {255, 124, 250}, {126, 0, 236},   {0, 2, 237},     {2, 4, 237},     {4, 6, 193},     {237, 4, 193},   {6, 8, 192},
	{8, 10, 196},    {192, 8, 196},   {10, 12, 195},   {12, 14, 194},   {195, 12, 194},  {14, 16, 197},   {16, 18, 198},   {18, 20, 199},
	{20, 22, 200},   {22, 24, 201},   {24, 26, 241},   {26, 28, 241},   {28, 30, 240},   {241, 28, 240},  {30, 32, 242},   {32, 34, 243},
	{34, 36, 212},   {36, 38, 211},   {38, 40, 210},   {40, 42, 210},   {42, 44, 209},   {44, 46, 208},   {209, 44, 208},  {46, 48, 205},
	{48, 50, 206},   {50, 52, 207},   {52, 54, 203},   {54, 56, 203},   {56, 58, 204},   {203, 56, 204},  {58, 60, 239},   {60, 62, 238},
	{239, 60, 238},  {62, 252, 238},  {48, 206, 205},  {52, 203, 207},  {210, 42, 209},  {32, 243, 242},  {36, 211, 212},  {24, 241, 201},
	{16, 198, 197},  {20, 200, 199},  {0, 237, 236},   {112, 230, 231}, {96, 247, 245},  {80, 218, 216},  {217, 218, 82},  {217, 86, 214},
	{114, 234, 230}, {234, 118, 232}, {239, 204, 58},  {215, 90, 249},  {50, 207, 206},  {246, 248, 94},  {46, 205, 213},  {46, 213, 208},
	{245, 246, 94},  {226, 102, 225}, {38, 210, 211},  {227, 106, 229}, {34, 212, 243},  {30, 242, 244},  {235, 228, 110}, {231, 235, 110},
	{30, 244, 240},  {22, 201, 200},  {233, 122, 255}, {18, 199, 198},  {251, 250, 126}, {14, 197, 202},  {14, 202, 194},  {236, 251, 126},
	{195, 196, 10},  {253, 252, 62},  {62, 64, 253},   {64, 66, 254},   {66, 68, 222},   {68, 70, 221},   {70, 72, 221},   {72, 74, 223},
	{221, 72, 223},  {74, 76, 219},   {76, 78, 220},   {219, 76, 220},  {78, 224, 220},  {64, 254, 253},  {68, 221, 222},  {222, 254, 66},
	{237, 180, 181}, {193, 179, 180}, {192, 185, 179}, {196, 186, 185}, {162, 186, 196}, {161, 162, 195}, {202, 165, 161}, {197, 164, 165},
	{164, 197, 198}, {163, 198, 199}, {171, 199, 200}, {166, 200, 201}, {168, 201, 241}, {167, 241, 240}, {170, 240, 244}, {169, 244, 242},
	{189, 242, 243}, {191, 243, 212}, {190, 212, 211}, {187, 211, 210}, {188, 210, 209}, {159, 209, 208}, {158, 208, 213}, {157, 213, 205},
	{156, 205, 206}, {155, 206, 207}, {160, 207, 203}, {152, 203, 204}, {153, 204, 239}, {154, 239, 238}, {150, 238, 252}, {151, 252, 253},
	{184, 183, 253}, {178, 184, 254}, {177, 178, 222}, {182, 177, 221}, {182, 223, 219}, {147, 219, 220}, {148, 146, 220}, {144, 148, 224},
	{145, 144, 216}, {149, 145, 218}, {142, 149, 217}, {143, 142, 214}, {141, 143, 215}, {139, 141, 249}, {140, 139, 248}, {172, 140, 246},
	{173, 172, 245}, {174, 173, 247}, {175, 174, 226}, {176, 175, 225}, {129, 176, 227}, {128, 129, 229}, {132, 128, 228}, {131, 132, 235},
	{130, 131, 231}, {138, 130, 230}, {134, 138, 234}, {133, 134, 232}, {137, 133, 233}, {135, 137, 255}, {136, 135, 250}, {181, 136, 251}};

static constexpr F32 kCubePositions[] = {
	// Front face
	-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

	// Back face
	0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,

	// Left face
	-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f,

	// Right face
	0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,

	// Top face
	-0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,

	// Bottom face
	-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f};

class Dbg::InternalCtx
{
public:
	class
	{
	public:
		BufferView m_renderableIndices;
		BufferView m_drawIndirectArgs;
		BufferHandle m_handle;
	} m_particleEmitter;
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
	m_rtDescr = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
															  Format::kR8G8B8A8_Unorm, "Dbg");
	m_rtDescr.bake();

	m_objectPickingRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x / 2, getRenderer().getInternalResolution().y / 2, Format::kR32_Uint, "ObjectPicking");
	m_objectPickingRtDescr.bake();

	m_objectPickingDepthRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x / 2, getRenderer().getInternalResolution().y / 2, Format::kD32_Sfloat, "ObjectPickingDepth");
	m_objectPickingDepthRtDescr.bake();

	ResourceManager& rsrcManager = ResourceManager::getSingleton();
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/GiProbe.png", m_giProbeImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/LightBulb.png", m_pointLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/SpotLight.png", m_spotLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/Decal.png", m_decalImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/ReflectionProbe.png", m_reflectionImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/Particles.png", m_particlesImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/Clouds.png", m_cloudImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Editor/Sun.png", m_sunImage));

	ANKI_CHECK(rsrcManager.loadResource("ShaderBinaries/Dbg.ankiprogbin", m_dbgProg));

	{
		BufferInitInfo buffInit("Dbg cube verts");
		buffInit.m_size = sizeof(Vec3) * 8;
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		m_boxLines.m_positionsBuff = GrManager::getSingleton().newBuffer(buffInit);

		Vec3* verts = static_cast<Vec3*>(m_boxLines.m_positionsBuff->map(0, kMaxPtrSize));

		constexpr F32 kSize = 1.0f;
		verts[0] = Vec3(kSize, kSize, kSize); // front top right
		verts[1] = Vec3(-kSize, kSize, kSize); // front top left
		verts[2] = Vec3(-kSize, -kSize, kSize); // front bottom left
		verts[3] = Vec3(kSize, -kSize, kSize); // front bottom right
		verts[4] = Vec3(kSize, kSize, -kSize); // back top right
		verts[5] = Vec3(-kSize, kSize, -kSize); // back top left
		verts[6] = Vec3(-kSize, -kSize, -kSize); // back bottom left
		verts[7] = Vec3(kSize, -kSize, -kSize); // back bottom right

		m_boxLines.m_positionsBuff->unmap();

		constexpr U kIndexCount = 12 * 2;
		buffInit.setName("Dbg cube indices");
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
		buffInit.m_size = kIndexCount * sizeof(U16);
		m_boxLines.m_indexBuff = GrManager::getSingleton().newBuffer(buffInit);
		U16* indices = static_cast<U16*>(m_boxLines.m_indexBuff->map(0, kMaxPtrSize));

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

		m_boxLines.m_indexBuff->unmap();

		ANKI_ASSERT(c == kIndexCount);
	}

	initGizmos();

	{
		BufferInitInfo buffInit("Debug cube");
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_size = sizeof(kCubePositions);
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
		m_debugPoint.m_positionsBuff = GrManager::getSingleton().newBuffer(buffInit);

		void* mapped = m_debugPoint.m_positionsBuff->map(0, kMaxPtrSize);
		memcpy(mapped, kCubePositions, sizeof(kCubePositions));
		m_debugPoint.m_positionsBuff->unmap();
	}

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

		void* mapped = positionsBuff->map(0, kMaxPtrSize);
		memcpy(mapped, positionsArray.getBegin(), positionsArray.getSizeInBytes());
		positionsBuff->unmap();

		buffInit.m_size = indicesArray.getSizeInBytes();
		indicesBuff = GrManager::getSingleton().newBuffer(buffInit);

		mapped = indicesBuff->map(0, kMaxPtrSize);
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

void Dbg::drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const ImageResource& image, Bool objectPicking,
							RenderPassWorkContext& rgraphCtx)
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
	variantInitInfo.addMutation("OBJECT_TYPE", U32(type));
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel,
											 (objectPicking) ? "BilboardsRenderPicking" : "BilboardsRenderMain");
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
	consts.m_viewProjMat = getRenderingContext().m_matrices.m_viewProjection;
	consts.m_camTrf = getRenderingContext().m_matrices.m_cameraTransform;
	consts.m_depthFailureVisualization = !m_options.m_depthTest;
	cmdb.setFastConstants(&consts, sizeof(consts));

	if(!objectPicking)
	{
		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
	}
	cmdb.bindSrv(1, 0, getClusterBinning().getPackedObjectsBuffer(type));
	cmdb.bindSrv(2, 0, getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type));
	cmdb.bindSrv(3, 0, TextureView(&image.getTexture(), TextureSubresourceDesc::all()));
	cmdb.bindSrv(4, 0, TextureView(&m_spotLightImage->getTexture(), TextureSubresourceDesc::all()));

	cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

	cmdb.draw(PrimitiveTopology::kTriangles, 6, objCount);
}

void Dbg::drawParticleEmitters(const InternalCtx& ictx, Bool objectPicking, RenderPassWorkContext& rgraphCtx)
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
	variantInitInfo.addMutation("OBJECT_TYPE", 0);
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel,
											 (objectPicking) ? "ParticleEmittersRenderPicking" : "ParticleEmittersRenderMain");
	const ShaderProgramResourceVariant* variant;
	m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
	cmdb.bindShaderProgram(&variant->getProgram());

	struct Constants
	{
		Mat4 m_viewProjMat;
		Mat3x4 m_camTrf;

		UVec3 m_padding;
		U32 m_depthFailureVisualization;
	} consts;
	consts.m_viewProjMat = getRenderingContext().m_matrices.m_viewProjection;
	consts.m_camTrf = getRenderingContext().m_matrices.m_cameraTransform;
	consts.m_depthFailureVisualization = !m_options.m_depthTest;
	cmdb.setFastConstants(&consts, sizeof(consts));

	if(!objectPicking)
	{
		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
	}

	cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
	cmdb.bindSrv(2, 0, ictx.m_particleEmitter.m_renderableIndices);
	cmdb.bindSrv(3, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());
	cmdb.bindSrv(4, 0, TextureView(&m_particlesImage->getTexture(), TextureSubresourceDesc::all()));

	cmdb.bindSampler(1, 1, getRenderer().getSamplers().m_trilinearRepeatAniso.get());

	cmdb.drawIndirect(PrimitiveTopology::kTriangles, ictx.m_particleEmitter.m_drawIndirectArgs);
}

void Dbg::drawNonGpuSceneObjects(Bool objectPicking, RenderPassWorkContext& rgraphCtx)
{
	auto drawNode = [&](const SceneNode& node, const ImageResource& iconImage) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
		variantInitInfo.addMutation("OBJECT_TYPE", 0);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel,
												 (objectPicking) ? "IconRenderPicking" : "IconRenderMain");
		const ShaderProgramResourceVariant* variant;
		m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
		cmdb.bindShaderProgram(&variant->getProgram());

		struct Constants
		{
			Mat4 m_viewProjMat;
			Mat3x4 m_camTrf;

			U32 m_depthFailureVisualization;
			U32 m_sceneNodeUuid;
			U32 m_padding1;
			U32 m_padding2;

			Vec3 m_objectPosition;
			U32 m_padding3;
		} consts;
		consts.m_viewProjMat = getRenderingContext().m_matrices.m_viewProjection;
		consts.m_camTrf = getRenderingContext().m_matrices.m_cameraTransform;
		consts.m_depthFailureVisualization = !m_options.m_depthTest;
		consts.m_sceneNodeUuid = node.getUuid();
		consts.m_objectPosition = node.getWorldTransform().getOrigin().xyz;

		cmdb.setFastConstants(&consts, sizeof(consts));

		if(!objectPicking)
		{
			rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
		}
		cmdb.bindSrv(1, 0, TextureView(&iconImage.getTexture(), TextureSubresourceDesc::all()));
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

		cmdb.draw(PrimitiveTopology::kTriangles, 6);
	};

	SceneGraph::getSingleton().visitNodes([&](const SceneNode& node) {
		if(node.hasComponent<SkyboxComponent>())
		{
			drawNode(node, *m_cloudImage);
		}

		if(node.hasComponent<LightComponent>()
		   && node.getFirstComponentOfType<LightComponent>().getLightComponentType() == LightComponentType::kDirectional)
		{
			drawNode(node, *m_sunImage);
		}

		return FunctorContinue::kContinue;
	});
}

void Dbg::drawIcons(const InternalCtx& ictx, Bool objectPicking, RenderPassWorkContext& rgraphCtx)
{
	drawNonRenderable(GpuSceneNonRenderableObjectType::kLight, GpuSceneArrays::Light::getSingleton().getElementCount(), *m_pointLightImage,
					  objectPicking, rgraphCtx);
	drawNonRenderable(GpuSceneNonRenderableObjectType::kDecal, GpuSceneArrays::Decal::getSingleton().getElementCount(), *m_decalImage, objectPicking,
					  rgraphCtx);
	drawNonRenderable(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe,
					  GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount(), *m_giProbeImage, objectPicking, rgraphCtx);
	drawNonRenderable(GpuSceneNonRenderableObjectType::kReflectionProbe, GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount(),
					  *m_reflectionImage, objectPicking, rgraphCtx);

	drawParticleEmitters(ictx, objectPicking, rgraphCtx);

	drawNonGpuSceneObjects(objectPicking, rgraphCtx);
}

void Dbg::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);

	m_runCtx.m_objectPickingRt = {};

	if(!isEnabled())
	{
		return;
	}

	InternalCtx ictx;

	// Common stuff for the particle emitters
	populateRenderGraphParticleEmitters(ictx);

	// Debug visualization
	if(m_options.mainDbgPass())
	{
		populateRenderGraphMain(ictx);
	}

	// Object picking
	if(m_options.m_objectPicking)
	{
		populateRenderGraphObjectPicking(ictx);
	}
}

void Dbg::populateRenderGraphParticleEmitters(InternalCtx& ictx)
{
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const U32 particleEmitterCount = GpuSceneArrays::ParticleEmitter2::getSingleton().getElementCount();
	const BufferView renderableIndices = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<U32>(particleEmitterCount + 1);

	const BufferView drawIndirectArgs = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<DrawIndirectArgs>(1);
	const BufferHandle handle = rgraph.importBuffer(drawIndirectArgs, BufferUsageBit::kNone);

	ictx.m_particleEmitter.m_drawIndirectArgs = drawIndirectArgs;
	ictx.m_particleEmitter.m_renderableIndices = renderableIndices;
	ictx.m_particleEmitter.m_handle = handle;

	// Prepare the prepare
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Dbg: Zero particle emitter stuff");

		pass.newBufferDependency(handle, BufferUsageBit::kCopyDestination);

		pass.setWork([drawIndirectArgs](RenderPassWorkContext& rgraphCtx) {
			rgraphCtx.m_commandBuffer->zeroBuffer(drawIndirectArgs);
		});
	}

	// Prepare
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Dbg: Prepare particle emitters");

		pass.newBufferDependency(handle, BufferUsageBit::kUavCompute);

		const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
		if(visOut.m_dependency.isValid())
		{
			pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kSrvCompute);
		}

		const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();
		if(fvisOut.m_dependency.isValid())
		{
			pass.newBufferDependency(fvisOut.m_dependency, BufferUsageBit::kSrvCompute);
		}

		pass.setWork([drawIndirectArgs, renderableIndices, this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "ParticleEmittersPrepare");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());

			cmdb.bindUav(0, 0, drawIndirectArgs);
			cmdb.bindUav(1, 0, renderableIndices);

			const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
			if(visOut.m_dependency.isValid())
			{
				cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());
				cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

				const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
				cmdb.dispatchCompute((allAabbCount + 63) / 64, 1, 1);
			}

			const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();
			if(fvisOut.m_dependency.isValid())
			{
				cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());
				cmdb.bindSrv(2, 0, fvisOut.m_visibleAaabbIndicesBuffer);

				const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
				cmdb.dispatchCompute((allAabbCount + 63) / 64, 1, 1);
			}
		});
	}
}

void Dbg::populateRenderGraphMain(InternalCtx& ictx)
{
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

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

	pass.newBufferDependency(ictx.m_particleEmitter.m_handle, BufferUsageBit::kIndirectDraw);

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

	if(m_options.m_indirectDiffuseProbes && isIndirectDiffuseClipmapsEnabled())
	{
		getIndirectDiffuseClipmaps().setDependenciesForDrawDebugProbes(pass);
	}

	pass.setWork([this, ictx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(Dbg);
		ANKI_ASSERT(m_options.mainDbgPass());

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		// Set common state
		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		cmdb.setDepthWrite(false);

		cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
		cmdb.setDepthCompareOperation(m_options.m_depthTest ? CompareOperation::kLess : CompareOperation::kAlways);
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
			consts.m_viewProjMat = getRenderingContext().m_matrices.m_viewProjection;
			consts.m_depthFailureVisualization = !m_options.m_depthTest;

			cmdb.setFastConstants(&consts, sizeof(consts));
			cmdb.bindVertexBuffer(0, BufferView(m_boxLines.m_positionsBuff.get()), sizeof(Vec3));
			cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
			cmdb.bindIndexBuffer(BufferView(m_boxLines.m_indexBuff.get()), IndexType::kU16);
		}

		// GBuffer AABBs
		const U32 gbufferAllAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
		if(m_options.m_renderableBoundingBoxes && gbufferAllAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());

			const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
			cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, gbufferAllAabbCount);
		}

		// Forward shading renderables
		const U32 forwardAllAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
		if(m_options.m_renderableBoundingBoxes && forwardAllAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());

			const GpuVisibilityOutput& visOut = getForwardShading().getGpuVisibilityOutput();
			cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, forwardAllAabbCount);
		}

		// Icons
		if(m_options.m_sceneGraphIcons)
		{
			drawIcons(ictx, false, rgraphCtx);
		}

		// Physics
		if(m_options.m_physics)
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

						m_positions.emplaceBack(HVec4(Vec4(pos.xyz0)));
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

				cmdb.setFastConstants(&getRenderingContext().m_matrices.m_viewProjection, sizeof(getRenderingContext().m_matrices.m_viewProjection));

				cmdb.draw(PrimitiveTopology::kLines, vertCount);
			}
		}

		if(m_options.m_indirectDiffuseProbes && isIndirectDiffuseClipmapsEnabled())
		{
			getIndirectDiffuseClipmaps().drawDebugProbes(rgraphCtx, m_options.m_indirectDiffuseProbesClipmap,
														 m_options.m_indirectDiffuseProbesClipmapType,
														 m_options.m_indirectDiffuseProbesClipmapColorScale);
		}

		if(m_gizmos.m_enabled)
		{
			cmdb.setDepthCompareOperation(CompareOperation::kAlways);
			drawGizmos(m_gizmos.m_trf, false, cmdb);
		}

		// Restore state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
		cmdb.setDepthCompareOperation(CompareOperation::kLess);
		cmdb.setDepthWrite(true);
	});
}

void Dbg::populateRenderGraphObjectPicking(InternalCtx& ictx)
{
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

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
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "RenderablesPreparePicking");
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
		pass.newBufferDependency(ictx.m_particleEmitter.m_handle, BufferUsageBit::kIndirectDraw);

		GraphicsRenderPassTargetDesc colorRti(objectPickingRt);
		colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
		GraphicsRenderPassTargetDesc depthRti(objectPickingDepthRt);
		depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
		depthRti.m_clearValue.m_depthStencil.m_depth = 1.0;
		depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
		pass.setRenderpassInfo({colorRti}, &depthRti);

		pass.setWork(
			[this, lodAndRenderableIndicesBuff, drawIndirectArgsBuff, drawCountBuff, maxVisibleCount, ictx](RenderPassWorkContext& rgraphCtx) {
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				// Set common state
				cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x / 2, getRenderer().getInternalResolution().y / 2);
				cmdb.setDepthCompareOperation(CompareOperation::kLess);

				ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
				variantInitInfo.addMutation("OBJECT_TYPE", 0);
				variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "RenderablesRenderPicking");
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

				cmdb.setFastConstants(&getRenderingContext().m_matrices.m_viewProjection, sizeof(getRenderingContext().m_matrices.m_viewProjection));

				cmdb.drawIndexedIndirectCount(PrimitiveTopology::kTriangles, drawIndirectArgsBuff, sizeof(DrawIndexedIndirectArgs), drawCountBuff,
											  maxVisibleCount);

				drawIcons(ictx, true, rgraphCtx);

				// Draw gizmos
				if(m_gizmos.m_enabled)
				{
					cmdb.setDepthCompareOperation(CompareOperation::kAlways);
					drawGizmos(m_gizmos.m_trf, true, cmdb);
					cmdb.setDepthCompareOperation(CompareOperation::kLess);
				}
			});
	}

	// Read the UUID RT to get the UUID that is over the mouse
	{
		U32 uuid;
		PtrSize dataOut;
		getRenderer().getReadbackManager().readMostRecentData(m_readback, &uuid, sizeof(uuid), dataOut);
		m_runCtx.m_objPickingRes = {};
		if(dataOut)
		{
			if(uuid & (1u << 31u))
			{
				// It's a gizmo
				uuid &= ~(1u << 31u);
				if(uuid < 3)
				{
					m_runCtx.m_objPickingRes.m_translationAxis = uuid;
				}
				else if(uuid < 6)
				{
					m_runCtx.m_objPickingRes.m_scaleAxis = uuid - 3;
				}
				else
				{
					ANKI_ASSERT(uuid - 6 < 3);
					m_runCtx.m_objPickingRes.m_rotationAxis = uuid - 6;
				}
			}
			else
			{
				m_runCtx.m_objPickingRes.m_sceneNodeUuid = uuid;
			}
		}

		const BufferView readbackBuff = getRenderer().getReadbackManager().allocateStructuredBuffer<U32>(m_readback, 1);

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Picking");

		pass.newTextureDependency(objectPickingRt, TextureUsageBit::kSrvCompute);

		pass.setWork([this, readbackBuff, objectPickingRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "ReadPickingBuffer");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			rgraphCtx.bindSrv(0, 0, objectPickingRt);
			cmdb.bindUav(0, 0, readbackBuff);

			Vec2 mousePos = Input::getSingleton().getMousePositionNdc();
			mousePos.y = -mousePos.y;
			mousePos = mousePos / 2.0f + 0.5f;
			mousePos *= Vec2(getRenderer().getInternalResolution() / 2);

			const UVec4 consts(UVec2(mousePos), 0u, 0u);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute(1, 1, 1);
		});
	}
}

void Dbg::drawGizmos(const Mat3x4& worldTransform, Bool objectPicking, CommandBuffer& cmdb) const
{
	// Draw a gizmo
	auto draw = [&](Vec4 color, U32 id, Euler rotation, Vec3 scale, Buffer& positionsBuff, Buffer& indexBuff) {
		struct Consts
		{
			Mat4 m_mvp;
			Vec4 m_color;
		} consts;
		consts.m_mvp = getRenderingContext().m_matrices.m_viewProjection * Mat4(worldTransform, Vec4(0.0f, 0.0f, 0.0f, 1.0f))
					   * Mat4(Vec3(0.0f), Mat3(rotation), scale);
		consts.m_color = color;

		struct PickingConsts
		{
			Mat4 m_mvp;

			UVec3 m_padding;
			U32 m_id;
		} constsPicking;
		constsPicking.m_mvp = consts.m_mvp;
		constsPicking.m_id = id;

		if(objectPicking)
		{
			cmdb.setFastConstants(&constsPicking, sizeof(constsPicking));
		}
		else
		{
			cmdb.setFastConstants(&consts, sizeof(consts));
		}

		cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
		cmdb.bindVertexBuffer(0, BufferView(&positionsBuff), sizeof(Vec3));

		cmdb.bindIndexBuffer(BufferView(&indexBuff), IndexType::kU16);

		cmdb.drawIndexed(PrimitiveTopology::kTriangles, U32(indexBuff.getSize() / sizeof(U16)));
	};

	// Draw all gizmos of an axis
	auto drawAxis = [&](Euler rot, Vec3 color, U32 id) {
		const F32 alpha = 1.0f;
		draw(Vec4(color, alpha), id + 0, rot, Vec3(1.0f), *m_gizmos.m_arrowPositions, *m_gizmos.m_arrowIndices);
		draw(Vec4(color, alpha), id + 3, rot, Vec3(1.0f), *m_gizmos.m_scalePositions, *m_gizmos.m_scaleIndices);
		draw(Vec4(color, alpha), id + 6, rot, Vec3(0.4f), *m_gizmos.m_ringPositions, *m_gizmos.m_ringIndices);
	};

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
	variantInitInfo.addMutation("OBJECT_TYPE", 0);
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel,
											 (objectPicking) ? "GizmosRenderPicking" : "GizmosRenderMain");
	const ShaderProgramResourceVariant* variant;
	m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
	cmdb.bindShaderProgram(&variant->getProgram());

	const Array<Vec3, 3> axis = {worldTransform.getXAxis().normalize(), worldTransform.getYAxis().normalize(), worldTransform.getZAxis().normalize()};
	const Array<Euler, 3> rotations = {Euler(0.0f, kPi / 2.0f, 0.0f), Euler(-kPi / 2.0f, 0.0f, 0.0f), Euler(0.0f, 0.0f, 0.0f)};
	const Array<Vec3, 3> colors = {Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)};
	const Vec3 viewDir = getRenderingContext().m_matrices.m_cameraTransform.getZAxis();

	// Since we don't use depth test draw the axis that points to the camera more
	struct DotProduct
	{
		F32 m_dot;
		U32 m_axis;
	};

	Array<DotProduct, 3> dotProducts;
	for(U32 i = 0; i < 3; ++i)
	{
		dotProducts[i].m_axis = i;
		dotProducts[i].m_dot = viewDir.dot(axis[i]);
	}

	std::sort(dotProducts.getBegin(), dotProducts.getEnd(), [](const DotProduct& a, const DotProduct& b) {
		return a.m_dot < b.m_dot;
	});

	for(U32 i = 0; i < 3; ++i)
	{
		const U32 id = (1u << 31u);
		const U32 axisi = dotProducts[i].m_axis;
		drawAxis(rotations[axisi], colors[axisi], id | axisi);
	}
}

} // end namespace anki
