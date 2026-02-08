// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/EditorUi.h>
#include <AnKi/Scene.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Collision.h>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

template<typename TFunc>
class DeferredPop
{
public:
	TFunc m_func;

	DeferredPop(TFunc func)
		: m_func(func)
	{
	}

	~DeferredPop()
	{
		m_func();
	}
};

#define ANKI_PUSH_POP(func, ...) \
	ImGui::Push##func(__VA_ARGS__); \
	DeferredPop ANKI_CONCATENATE(pop, __LINE__)([] { \
		ImGui::Pop##func(); \
	})

/// This pushes a width for text input widgets that leaves "labelSize" chars for the label
#define ANKI_PUSH_POP_TEXT_INPUT_WIDTH(labelSize) \
	const F32 labelWidthBase = ImGui::GetFontSize() * labelSize; /* Some amount of width for label, based on font size. */ \
	const F32 labelWidthMax = ImGui::GetContentRegionAvail().x * 0.40f; /* ...but always leave some room for framed widgets. */ \
	ANKI_PUSH_POP(ItemWidth, -min(labelWidthBase, labelWidthMax))

// Using some NDC coords find the closest point these coords are to a ray
static F32 projectNdcToRay(Vec2 ndc, Vec3 rayOrigin, Vec3 rayDir)
{
	const Frustum& frustum = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>().getFrustum();
	const Mat4 invMvp = frustum.getViewProjectionMatrix().invert();

	Vec4 v4 = frustum.getViewProjectionMatrix() * rayOrigin.xyz1;
	const Vec2 rayOriginNdc = v4.xy / v4.w;

	v4 = frustum.getViewProjectionMatrix() * (rayOrigin + rayDir).xyz1;
	const Vec2 rayDirNdc = (v4.xy / v4.w - rayOriginNdc).normalize();

	const Vec2 disiredPosNdc = ndc.projectTo(rayOriginNdc, rayDirNdc);
	const Bool positiveSizeOfAxis = (disiredPosNdc - rayOriginNdc).dot(rayDirNdc) > 0.0f;

	// Create 2 lines (0 and 1) and find the closest point between them on line 1
	// Line equation: r(t) = a + t * b

	// Create line 0 which is built from the camera origin and a far point that was unprojected from NDC
	v4 = invMvp * Vec4(disiredPosNdc, 1.0f, 1.0f);
	const Vec3 a0 = v4.xyz / v4.w;
	const Vec3 b0 = frustum.getWorldTransform().getOrigin().xyz - a0;

	// Line 1 is built from the ray
	const Vec3 a1 = rayOrigin;
	const Vec3 b1 = rayDir; // = rayDir + rayOrigin - rayOrigin

	// Solve the system
	const Vec3 w = a0 - a1;

	const F32 A = b0.dot(b0);
	const F32 B = b0.dot(b1);
	const F32 C = b1.dot(b1);
	const F32 D = b0.dot(w);
	const F32 E = b1.dot(w);

	const F32 t = (A * E - B * D) / (A * C - B * B);

	const Vec3 touchingPoint = a1 + t * b1;

	const F32 distFromOrign = (touchingPoint - rayOrigin).length() * ((positiveSizeOfAxis) ? 1.0f : -1.0f);

	return distFromOrign;
}

static Bool projectNdcToPlane(Vec2 ndc, Plane plane, Vec3& point)
{
	const Frustum& frustum = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>().getFrustum();
	const Mat4 invMvp = frustum.getViewProjectionMatrix().invert();

	Vec4 v4 = invMvp * Vec4(ndc, 1.0f, 1.0f);
	v4 /= v4.w;

	const Vec3 rayOrigin = frustum.getWorldTransform().getOrigin().xyz;
	const Vec3 rayDir = (v4.xyz - rayOrigin).normalize();

	Vec4 collisionPoint;
	const Bool collides = testCollision(plane, Ray(rayOrigin, rayDir), collisionPoint);

	if(collides)
	{
		point = collisionPoint.xyz;
	}

	return collides;
}

EditorUi::EditorUi()
{
	Logger::getSingleton().addMessageHandler(this, loggerMessageHandler);
}

EditorUi::~EditorUi()
{
	Logger::getSingleton().removeMessageHandler(this, loggerMessageHandler);
}

void EditorUi::loggerMessageHandler(void* ud, const LoggerMessageInfo& info)
{
	EditorUi& self = *static_cast<EditorUi*>(ud);
	String logEntry;
	logEntry.sprintf("[%s][%-4s] %s [%s:%d][%s][%s]\n", kLoggerMessageTypeText[info.m_type], info.m_subsystem ? info.m_subsystem : "N/A", info.m_msg,
					 info.m_file, info.m_line, info.m_func, info.m_threadName);

	LockGuard lock(self.m_consoleWindow.m_logMtx);
	self.m_consoleWindow.m_log.emplaceBack(std::pair(info.m_type, logEntry));
	self.m_consoleWindow.m_forceLogScrollDown = true;
}

void EditorUi::draw(UiCanvas& canvas)
{
	// Setup style and stuff
	m_canvas = &canvas;

	if(!m_font)
	{
		const Array<CString, 2> fnames = {"EngineAssets/UbuntuRegular.ttf", "EngineAssets/Editor/materialdesignicons-webfont.ttf"};
		m_font = canvas.addFonts(fnames);
	}

	if(!m_monospaceFont)
	{
		m_monospaceFont = canvas.addFont("EngineAssets/UbuntuMonoRegular.ttf");
	}

	ImGui::PushFont(m_font, m_fontSize);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 20.0f);

	const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.0f;

	// Do some pre-drawing work
	m_showDeleteSceneNodeDialog = false;
	m_sceneGraphView = gatherAllSceneNodes();
	validateSelectedNode();
	handleInput();
	objectPicking();

	// Draw the windows
	ImGui::Begin("MainWindow", nullptr,
				 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
					 | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
	ImGui::SetWindowSize(canvas.getSizef());

	mainMenu();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	consoleWindow();
	cVarsWindow();
	debugRtsWindow();

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialPos(viewportPos);
		const Vec2 initialSize(400.0f, viewportSize.y - kConsoleHeight);

		Bool deleteSelectedNode;
		m_sceneHierarchyWindow.drawWindow(initialPos, initialSize, ImGuiWindowFlags_NoCollapse, m_onNextUpdateFocusOnSelectedNode, m_selectedNode,
										  deleteSelectedNode);

		m_onNextUpdateFocusOnSelectedNode = false;
		m_showDeleteSceneNodeDialog = m_showDeleteSceneNodeDialog || deleteSelectedNode;
	}

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const F32 initialWidth = 500.0f;
		const Vec2 initialPos(viewportSize.x - initialWidth, viewportPos.y);
		const Vec2 initialSize(initialWidth, viewportSize.y - kConsoleHeight);

		m_sceneNodePropertiesWindow.drawWindow(m_selectedNode, m_sceneGraphView, initialPos, initialSize, ImGuiWindowFlags_NoCollapse);
	}

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(viewportSize.x / 2.0f, kConsoleHeight);
		const Vec2 initialPos = Vec2(viewportSize.x / 2.0f, viewportPos.y + viewportSize.y - initialSize.y);

		m_assetBrowserWindow.drawWindow(initialSize, initialPos, ImGuiWindowFlags_NoCollapse);
	}

	deleteSelectedNodeDialog(m_showDeleteSceneNodeDialog);

	ImGui::End();

	ImGui::PopStyleVar();
	ImGui::PopFont();

	// Mouse is over any window if mouse is hovering or if clicked on a window
	m_mouseOverAnyWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::GetIO().WantCaptureMouse;

	m_canvas = nullptr;
}

void EditorUi::validateSelectedNode()
{
	if(m_selectedNode == nullptr)
	{
		return;
	}

	Bool found = false;
	for(const SceneGraphViewScene& sceneView : m_sceneGraphView.m_scenes)
	{
		for(U32 i = 0; i < sceneView.m_nodeNames.getSize(); ++i)
		{
			if(sceneView.m_nodes[i] == m_selectedNode && sceneView.m_nodeUuids[i] == m_selectedNode->getUuid())
			{
				found = true;
				break;
			}
		}
	}

	if(!found)
	{
		m_selectedNode = nullptr;
		m_selectedNodeUuid = 0;
	}
}

void EditorUi::deleteSelectedNodeDialog(Bool del)
{
	if(del)
	{
		const Vec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, Vec2(0.5f));
		ImGui::OpenPopup("Delete Node?");
	}

	if(ImGui::BeginPopupModal("Delete Node?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Delete node: %s", m_selectedNode->getName().cstr());

		if(ImGui::Button("Yes"))
		{
			m_selectedNode->markForDeletion();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if(ImGui::Button("No"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void EditorUi::mainMenu()
{
	// The menu and toolbox is based on the comments in https://github.com/ocornut/imgui/issues/3518

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	if(ImGui::BeginViewportSideBar("##MainMenu", viewport, ImGuiDir_Up, ImGui::GetFrameHeight(), windowFlags))
	{
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu(ICON_MDI_FOLDER_OUTLINE " File"))
			{
				if(ImGui::MenuItem(ICON_MDI_CLOSE_CIRCLE " Quit", "CTRL+Q"))
				{
					m_quit = true;
				}
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu(ICON_MDI_APPLICATION_OUTLINE " Windows"))
			{
				if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Console"))
				{
					m_showConsoleWindow = true;
				}

				if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " SceneNode Props"))
				{
					m_sceneNodePropertiesWindow.m_open = true;
				}

				if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Scene Hierarchy"))
				{
					m_sceneHierarchyWindow.m_open = true;
				}

				if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Assets"))
				{
					m_assetBrowserWindow.m_open = true;
				}

				if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " CVars Editor"))
				{
					m_showCVarEditorWindow = true;
				}

				if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Debug Render Targets"))
				{
					m_showDebugRtsWindow = true;
				}

				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu(ICON_MDI_CUBE_SCAN " Debug"))
			{
				DbgOptions& options = Renderer::getSingleton().getDbg().getOptions();
				Bool bBoundingBoxes = options.m_renderableBoundingBoxes;
				if(ImGui::Checkbox("Visible Renderables", &bBoundingBoxes))
				{
					options.m_renderableBoundingBoxes = bBoundingBoxes;
				}

				Bool bPhysics = options.m_physics;
				if(ImGui::Checkbox("Physics Bodies", &bPhysics))
				{
					options.m_physics = bPhysics;
				}

				Bool bDepthTest = options.m_depthTest;
				if(ImGui::Checkbox("Depth Test", &bDepthTest))
				{
					options.m_depthTest = bDepthTest;
				}

				ImGui::SeparatorText("Indirect Diffuse");
				Bool bProbes = options.m_indirectDiffuseProbes;
				if(ImGui::Checkbox("Indirect Probes", &bProbes))
				{
					options.m_indirectDiffuseProbes = bProbes;
				}

				ImGui::BeginDisabled(!bProbes);

				// Clipmap
				I32 clipmap = I32(options.m_indirectDiffuseProbesClipmap);
				if(ImGui::InputInt("Clipmap", &clipmap))
				{
					options.m_indirectDiffuseProbesClipmap = clamp<U8>(U8(clipmap), 0, kIndirectDiffuseClipmapCount - 1);
				}

				// Clipmap type
				if(ImGui::BeginCombo("Clipmap Type", kIndirectDiffuseClipmapsProbeTypeNames[options.m_indirectDiffuseProbesClipmapType]))
				{
					for(IndirectDiffuseClipmapsProbeType type : EnumIterable<IndirectDiffuseClipmapsProbeType>())
					{
						const Bool selected = type == options.m_indirectDiffuseProbesClipmapType;
						if(ImGui::Selectable(kIndirectDiffuseClipmapsProbeTypeNames[type], selected))
						{
							options.m_indirectDiffuseProbesClipmapType = type;
						}

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if(selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Clipmap color scale
				F32 scale = options.m_indirectDiffuseProbesClipmapColorScale;
				if(ImGui::SliderFloat("Color Scale", &scale, 0.0f, 1.0f))
				{
					options.m_indirectDiffuseProbesClipmapColorScale = scale;
				}

				ImGui::EndDisabled();

				ImGui::EndMenu();
			}

			// Title
			{
				CString text = "AnKi 3D Engine Editor";
				const F32 menuBarWidth = ImGui::GetWindowWidth();
				const F32 textWidth = ImGui::CalcTextSize(text.cstr()).x;
				ImGui::SameLine(menuBarWidth - menuBarWidth / 2.0f - textWidth / 2.0f);

				ImGui::TextUnformatted(text.cstr());
			}

			// Quit bnt
			{
				const Char* text = ICON_MDI_CLOSE_CIRCLE;
				const Vec2 textSize = ImGui::CalcTextSize(text);

				const F32 menuBarWidth = ImGui::GetWindowWidth();
				ImGui::SameLine(menuBarWidth - textSize.x - ImGui::GetStyle().FramePadding.x * 2.0f - kMargin);

				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4(1.0f, 0.0f, 0.0f, 1.0f));

				if(ImGui::Button(text))
				{
					m_quit = true;
				}

				ImGui::PopStyleColor();
			}

			ImGui::EndMenuBar();
		}

		if(Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0 && Input::getSingleton().getKey(KeyCode::kQ) > 0)
		{
			m_quit = true;
		}
	}
	ImGui::End();

	// Toolbox
	if(ImGui::BeginViewportSideBar("##Toolbox", viewport, ImGuiDir_Up, ImGui::GetFrameHeight(), windowFlags))
	{
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::Button(ICON_MDI_CONTENT_SAVE_ALL))
			{
				if(SceneGraph::getSingleton().saveScene("./Scene.ankiscene", SceneGraph::getSingleton().getActiveScene()))
				{
					ANKI_LOGE("Failed to save scene");
				}
			}
			ImGui::SetItemTooltip("Save scene");

			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::CalcTextSize("00.000").x);

			if(ImGui::SliderFloat(ICON_MDI_AXIS_ARROW "&" ICON_MDI_ARROW_EXPAND_ALL " Snapping", &m_toolbox.m_scaleTranslationSnapping, 0.0, 10.0f))
			{
				const F32 roundTo = 0.5f;
				m_toolbox.m_scaleTranslationSnapping = round(m_toolbox.m_scaleTranslationSnapping / roundTo) * roundTo;
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::CalcTextSize("00.000").x);

			if(ImGui::SliderFloat(ICON_MDI_ROTATE_ORBIT " Snapping", &m_toolbox.m_rotationSnappingDeg, 0.0, 90.0f))
			{
				const F32 roundTo = 1.0f;
				m_toolbox.m_rotationSnappingDeg = round(m_toolbox.m_rotationSnappingDeg / roundTo) * roundTo;
			}

			// Play button
			{
				const Bool bPaused = SceneGraph::getSingleton().isPaused();

				CString text = (bPaused) ? ICON_MDI_PLAY_CIRCLE " Play" : ICON_MDI_STOP_CIRCLE "Stop (Esc)";
				const F32 menuBarWidth = ImGui::GetWindowWidth();
				const F32 textWidth = ImGui::CalcTextSize(text.cstr()).x;
				ImGui::SameLine(menuBarWidth - menuBarWidth / 2.0f - textWidth / 2.0f);

				if(ImGui::Button(text.cstr()))
				{
					SceneGraph::getSingleton().pause(!bPaused);
				}
			}

			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
}

void EditorUi::cVarsWindow()
{
	if(!m_showCVarEditorWindow)
	{
		return;
	}

	F32 maxCvarTextSize = 0.0f;
	CVarSet::getSingleton().iterateCVars([&](CVar& cvar) {
		maxCvarTextSize = max(maxCvarTextSize, ImGui::CalcTextSize(cvar.getName().cstr()).x);
		return FunctorContinue::kContinue;
	});

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 initialSize = Vec2(900.0f, m_canvas->getSizef().y * 0.8f);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Once, Vec2(0.5f));
	}

	if(ImGui::Begin("CVars Editor", &m_showCVarEditorWindow, 0))
	{
		drawfilteredText(m_cvarsEditorWindow.m_filter);

		if(ImGui::BeginChild("##Child", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if(ImGui::BeginTable("Table", 2, 0))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, maxCvarTextSize + 20.0f);
				ImGui::TableSetupColumn("Value");

				I32 id = 0;
				CVarSet::getSingleton().iterateCVars([&](CVar& cvar) {
					if(!m_cvarsEditorWindow.m_filter.PassFilter(cvar.getName().cstr()))
					{
						return FunctorContinue::kContinue;
					}

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(cvar.getName().cstr());

					ImGui::TableNextColumn();

					ImGui::PushID(id++);

					ImGui::SetNextItemWidth(-1.0f);

					if(cvar.getValueType() == CVarValueType::kBool)
					{
						BoolCVar& bcvar = static_cast<BoolCVar&>(cvar);
						Bool val = bcvar;
						ImGui::Checkbox("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericF32)
					{
						NumericCVar<F32>& bcvar = static_cast<NumericCVar<F32>&>(cvar);
						F32 val = bcvar;
						ImGui::InputFloat("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericF64)
					{
						NumericCVar<F64>& bcvar = static_cast<NumericCVar<F64>&>(cvar);
						F64 val = bcvar;
						ImGui::InputDouble("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericU8)
					{
						NumericCVar<U8>& bcvar = static_cast<NumericCVar<U8>&>(cvar);
						I32 val = bcvar;
						ImGui::InputInt("", &val);
						bcvar = U8(val);
					}
					else if(cvar.getValueType() == CVarValueType::kNumericU16)
					{
						NumericCVar<U16>& bcvar = static_cast<NumericCVar<U16>&>(cvar);
						I32 val = bcvar;
						ImGui::InputInt("", &val);
						bcvar = U8(val);
					}
					else if(cvar.getValueType() == CVarValueType::kNumericU32)
					{
						NumericCVar<U32>& bcvar = static_cast<NumericCVar<U32>&>(cvar);
						I32 val = bcvar;
						ImGui::InputInt("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericPtrSize)
					{
						NumericCVar<PtrSize>& bcvar = static_cast<NumericCVar<PtrSize>&>(cvar);
						I32 val = I32(bcvar);
						ImGui::InputInt("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kString)
					{
						StringCVar& bcvar = static_cast<StringCVar&>(cvar);
						CString value = bcvar;
						Char str[256];
						std::strncpy(str, value.cstr(), sizeof(str));
						ImGui::InputText("", str, sizeof(str));
						bcvar = str;
					}
					else
					{
						ANKI_ASSERT(!"TODO");
					}

					ImGui::PopID();

					return FunctorContinue::kContinue;
				});

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	ImGui::End();
}

void EditorUi::debugRtsWindow()
{
	if(!m_showDebugRtsWindow)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 initialSize = Vec2(450.0f, m_canvas->getSizef().y * 0.4f);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Once, Vec2(0.5f));
	}

	if(ImGui::Begin("Debug Render Targets", &m_showDebugRtsWindow, 0))
	{
		const Bool refresh = ImGui::Checkbox("Disable tonemapping", &m_debugRtsWindow.m_disableTonemapping);
		ImGui::TextUnformatted("");
		ImGui::Separator();

		if(ImGui::BeginChild("Content", Vec2(-1.0f, -1.0f)))
		{
			// Gather the names
			DynamicArray<String> rtNames;
			Renderer::getSingleton().iterateDebugRenderTargetNames([&](CString name) {
				rtNames.emplaceBack(name);
				return FunctorContinue::kContinue;
			});

			std::sort(rtNames.getBegin(), rtNames.getEnd());

			for(const String& name : rtNames)
			{
				Bool isActive = (name == Renderer::getSingleton().getCurrentDebugRenderTarget());
				if(ImGui::Checkbox(name.cstr(), &isActive) || (isActive && refresh))
				{
					Renderer::getSingleton().setCurrentDebugRenderTarget(isActive ? name : "", m_debugRtsWindow.m_disableTonemapping);
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void EditorUi::consoleWindow()
{
	if(!m_showConsoleWindow)
	{
		return;
	}

	auto& state = m_consoleWindow;

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(viewportSize.x / 2.0f, kConsoleHeight);
		const Vec2 initialPos = Vec2(0.0f, viewportPos.y + viewportSize.y - initialSize.y);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Console", &m_showConsoleWindow, ImGuiWindowFlags_NoCollapse))
	{
		// Lua input
		{
			Char consoleTxt[kMaxTextInputLen] = "";
			ImGui::SetNextItemWidth(-1.0f);
			if(ImGui::InputTextWithHint(" ", ICON_MDI_LANGUAGE_LUA " LUA console", consoleTxt, sizeof(consoleTxt),
										ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
			{
				[[maybe_unused]] const Error err = ScriptManager::getSingleton().evalString(consoleTxt);
				ImGui::SetKeyboardFocusHere(-1); // On enter it loses focus so call this to fix it
			}
		}

		// Clear Log
		{
			if(ImGui::Button(ICON_MDI_DELETE))
			{
				state.m_log.destroy();
			}
			ImGui::SetItemTooltip("Clear log");
			ImGui::SameLine();
		}

		// Search log
		drawfilteredText(state.m_logFilter);

		// Log
		{
			if(ImGui::BeginChild("Log", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
			{
				ImGui::PushFont(m_monospaceFont, 0.0f);

				if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					LockGuard lock(state.m_logMtx);

					for(const auto& logEntry : state.m_log)
					{
						if(state.m_logFilter.PassFilter(logEntry.second.cstr()))
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							constexpr Array<Vec3, U(LoggerMessageType::kCount)> colors = {Vec3(0.074f, 0.631f, 0.054f), Vec3(0.074f, 0.354f, 0.631f),
																						  Vec3(1.0f, 0.0f, 0.0f), Vec3(0.756f, 0.611f, 0.0f),
																						  Vec3(1.0f, 0.0f, 0.0f)};
							ImGui::PushStyleColor(ImGuiCol_Text, Vec4(colors[logEntry.first].xyz1));
							ImGui::TextUnformatted(logEntry.second.cstr());
							ImGui::PopStyleColor();
						}
					}

					if(state.m_forceLogScrollDown)
					{
						ImGui::SetScrollHereY(1.0f);
						state.m_forceLogScrollDown = false;
					}

					ImGui::EndTable();
				}

				ImGui::PopFont();
			}
			ImGui::EndChild();
		}
	}

	ImGui::End();
}

void EditorUi::objectPicking()
{
	if(!m_mouseOverAnyWindow && Input::getSingleton().getMouseButton(MouseButton::kLeft) == 1)
	{
		const DbgObjectPickingResult& res = Renderer::getSingleton().getDbg().getObjectPickingResultAtMousePosition();

		if(res.m_sceneNodeUuid != 0)
		{
			for(const SceneGraphViewScene& sceneView : m_sceneGraphView.m_scenes)
			{
				Bool done = false;
				for(U32 i = 0; i < sceneView.m_nodeNames.getSize(); ++i)
				{
					if(sceneView.m_nodeUuids[i] == res.m_sceneNodeUuid)
					{
						m_selectedNode = sceneView.m_nodes[i];
						m_selectedNodeUuid = sceneView.m_nodeUuids[i];
						m_onNextUpdateFocusOnSelectedNode = true;
						ANKI_LOGV("Selecting scene node: %s", sceneView.m_nodes[i]->getName().cstr());
						done = true;
						break;
					}
				}

				if(done)
				{
					break;
				}
			}

			m_objectPicking.m_translationAxisSelected = kMaxU32;
			m_objectPicking.m_scaleAxisSelected = kMaxU32;
			m_objectPicking.m_rotationAxisSelected = kMaxU32;
		}
		else if(res.isValid() && m_selectedNode)
		{
			// Clicked a gizmo

			const Transform& nodeTrf = m_selectedNode->getLocalTransform();
			const Vec3 nodeOrigin = nodeTrf.getOrigin().xyz;
			Array<Vec3, 3> rotationAxis;
			rotationAxis[0] = nodeTrf.getRotation().getXAxis();
			rotationAxis[1] = nodeTrf.getRotation().getYAxis();
			rotationAxis[2] = nodeTrf.getRotation().getZAxis();

			if(res.m_translationAxis < 3)
			{
				m_objectPicking.m_translationAxisSelected = res.m_translationAxis;
				m_objectPicking.m_scaleAxisSelected = kMaxU32;
				m_objectPicking.m_rotationAxisSelected = kMaxU32;
			}
			else if(res.m_scaleAxis < 3)
			{
				m_objectPicking.m_translationAxisSelected = kMaxU32;
				m_objectPicking.m_scaleAxisSelected = res.m_scaleAxis;
				m_objectPicking.m_rotationAxisSelected = kMaxU32;
			}
			else if(res.m_rotationAxis < 3)
			{
				m_objectPicking.m_translationAxisSelected = kMaxU32;
				m_objectPicking.m_scaleAxisSelected = kMaxU32;
				m_objectPicking.m_rotationAxisSelected = res.m_rotationAxis;

				// Calc the pivot point
				const Transform camTrf =
					SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>().getFrustum().getWorldTransform();

				Plane axisPlane;
				axisPlane.setFromRay(nodeOrigin, rotationAxis[res.m_rotationAxis]);

				Bool collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, m_objectPicking.m_pivotPoint);
				if(!collides)
				{
					// Clicked the gizmo from the back side, use the negative plane
					axisPlane.setFromRay(nodeOrigin, -rotationAxis[res.m_rotationAxis]);
					collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, m_objectPicking.m_pivotPoint);
					if(!collides)
					{
						ANKI_LOGW("Can't determin the pivot point");
						m_objectPicking.m_pivotPoint = nodeOrigin;
					}
				}
			}

			if(res.m_translationAxis < 3 || res.m_scaleAxis < 3)
			{
				const U32 axis = (res.m_translationAxis < 3) ? res.m_translationAxis : res.m_scaleAxis;
				m_objectPicking.m_pivotPoint =
					nodeOrigin + rotationAxis[axis] * projectNdcToRay(Input::getSingleton().getMousePositionNdc(), nodeOrigin, rotationAxis[axis]);
			}
		}
		else
		{
			// Clicked on sky
			m_selectedNode = nullptr;
			m_selectedNodeUuid = 0;
			m_objectPicking.m_translationAxisSelected = kMaxU32;
			m_objectPicking.m_scaleAxisSelected = kMaxU32;
			m_objectPicking.m_rotationAxisSelected = kMaxU32;
		}
	}

	if(!m_mouseOverAnyWindow && Input::getSingleton().getMouseButton(MouseButton::kLeft) > 1 && m_selectedNode)
	{
		// Possibly dragging

		const Transform& nodeTrf = m_selectedNode->getLocalTransform();
		Array<Vec3, 3> rotationAxis;
		rotationAxis[0] = nodeTrf.getRotation().getXAxis();
		rotationAxis[1] = nodeTrf.getRotation().getYAxis();
		rotationAxis[2] = nodeTrf.getRotation().getZAxis();

		if(m_objectPicking.m_translationAxisSelected < 3)
		{
			const U32 axis = m_objectPicking.m_translationAxisSelected;
			const F32 moveDistance = projectNdcToRay(Input::getSingleton().getMousePositionNdc(), m_objectPicking.m_pivotPoint, rotationAxis[axis]);

			const Vec3 oldPosition = nodeTrf.getOrigin().xyz;
			Vec3 newPosition = oldPosition + rotationAxis[axis] * moveDistance;

			// Snap position
			for(U32 i = 0; i < 3 && m_toolbox.m_scaleTranslationSnapping > kEpsilonf; ++i)
			{
				newPosition[i] = round(newPosition[i] / m_toolbox.m_scaleTranslationSnapping) * m_toolbox.m_scaleTranslationSnapping;
			}
			m_selectedNode->setLocalOrigin(newPosition);

			// Update the pivot
			const Vec3 moveDiff = newPosition - oldPosition; // Move the pivot as you moved the node origin
			m_objectPicking.m_pivotPoint = m_objectPicking.m_pivotPoint + moveDiff;
		}
		else if(m_objectPicking.m_scaleAxisSelected < 3)
		{
			const U32 axis = m_objectPicking.m_scaleAxisSelected;
			const F32 moveDistance = projectNdcToRay(Input::getSingleton().getMousePositionNdc(), m_objectPicking.m_pivotPoint, rotationAxis[axis]);

			const F32 oldAxisScale = nodeTrf.getScale()[axis];
			F32 newAxisScale = oldAxisScale + moveDistance;

			// Snap scale
			if(m_toolbox.m_scaleTranslationSnapping > kEpsilonf)
			{
				newAxisScale = round(newAxisScale / m_toolbox.m_scaleTranslationSnapping) * m_toolbox.m_scaleTranslationSnapping;
			}

			Vec3 scale = nodeTrf.getScale().xyz;
			scale[axis] = max(newAxisScale, m_toolbox.m_scaleTranslationSnapping);
			m_selectedNode->setLocalScale(scale);

			// Update the pivot
			const F32 adjustedMoveDistance = newAxisScale - oldAxisScale;
			m_objectPicking.m_pivotPoint = m_objectPicking.m_pivotPoint + rotationAxis[axis] * adjustedMoveDistance;
		}
		else if(m_objectPicking.m_rotationAxisSelected < 3)
		{
			const U32 axis = m_objectPicking.m_rotationAxisSelected;
			const Vec3 nodeOrigin = nodeTrf.getOrigin().xyz;

			// Compute the new pivot point
			Plane axisPlane;
			axisPlane.setFromRay(nodeOrigin, rotationAxis[axis]);

			Vec3 newPivotPoint;
			Bool collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, newPivotPoint);
			if(!collides)
			{
				// Clicked the gizmo from the back side, use the negative plane
				axisPlane.setFromRay(nodeOrigin, -rotationAxis[axis]);
				collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, newPivotPoint);
				if(!collides)
				{
					ANKI_LOGW("Can't determin the pivot point");
					newPivotPoint = nodeOrigin;
				}
			}

			// Compute the angle between the points
			const Vec3 dir0 = (newPivotPoint - nodeOrigin).normalize();
			const Vec3 dir1 = (m_objectPicking.m_pivotPoint - nodeOrigin).normalize();
			F32 angle = acos(saturate(dir1.dot(dir0)));
			if(m_toolbox.m_rotationSnappingDeg > kEpsilonf)
			{
				const F32 rad = toRad(m_toolbox.m_rotationSnappingDeg);
				angle = round(angle / rad) * rad;
			}

			Vec3 cross = dir1.cross(dir0);

			if(cross != Vec3(0.0f))
			{
				cross = cross.normalize();
				const F32 angleSign = (cross.dot(rotationAxis[axis]) < 1.0f) ? -1.0f : 1.0f;
				angle *= angleSign;

				// Apply the angle
				if(m_objectPicking.m_rotationAxisSelected == 0)
				{
					m_selectedNode->rotateLocalX(angle);
				}
				else if(m_objectPicking.m_rotationAxisSelected == 1)
				{
					m_selectedNode->rotateLocalY(angle);
				}
				else
				{
					m_selectedNode->rotateLocalZ(angle);
				}

				// Use the snapped angle to adjust the pivot point
				Mat3 rot;
				rot.setZAxis(rotationAxis[axis]);
				rot.setXAxis(dir1);
				rot.setYAxis(rotationAxis[axis].cross(dir1).normalize());
				rot.rotateZAxis(angle);
				newPivotPoint = nodeOrigin + rot.getXAxis() * (newPivotPoint - nodeOrigin).length();

				m_objectPicking.m_pivotPoint = newPivotPoint;
			}
		}
	}

	if(m_selectedNode)
	{
		Renderer::getSingleton().getDbg().enableGizmos(m_selectedNode->getWorldTransform(), true);
	}
	else
	{
		Renderer::getSingleton().getDbg().enableGizmos(Transform::getIdentity(), false);
	}
}

void EditorUi::handleInput()
{
	Input& input = Input::getSingleton();

	if(!SceneGraph::getSingleton().isPaused())
	{
		if(input.getKey(KeyCode::kEscape) > 0)
		{
			// Esc pauses
			SceneGraph::getSingleton().pause(true);
		}
		else
		{
			// Skip input handling if playing
			return;
		}
	}

	input.hideMouseCursor(false);
	input.lockMouseWindowCenter(false);

	if(m_quit)
	{
		input.addEvent(InputEvent::kWindowClosed);
	}

	static Vec2 mousePosOn1stClick = input.getMousePositionNdc();
	if(input.getMouseButton(MouseButton::kRight) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = input.getMousePositionNdc();
	}

	if(input.getMouseButton(MouseButton::kRight) > 0 && !m_mouseOverAnyWindow)
	{
		// move the camera
		SceneNode& mover = SceneGraph::getSingleton().getActiveCameraNode();

		constexpr F32 kRotateAngle = toRad(2.5f);
		constexpr F32 kMouseSensitivity = 5.0f;

		if(input.getKey(KeyCode::kUp) > 0)
		{
			mover.rotateLocalX(kRotateAngle);
		}

		if(input.getKey(KeyCode::kDown) > 0)
		{
			mover.rotateLocalX(-kRotateAngle);
		}

		if(input.getKey(KeyCode::kLeft) > 0)
		{
			mover.rotateLocalY(kRotateAngle);
		}

		if(input.getKey(KeyCode::kRight) > 0)
		{
			mover.rotateLocalY(-kRotateAngle);
		}

		F32 moveDistance = 0.1f;
		if(input.getKey(KeyCode::kLeftShift) > 0)
		{
			moveDistance *= 4.0f;
		}

		if(input.getKey(KeyCode::kA) > 0)
		{
			mover.moveLocalX(-moveDistance);
		}

		if(input.getKey(KeyCode::kD) > 0)
		{
			mover.moveLocalX(moveDistance);
		}

		if(input.getKey(KeyCode::kQ) > 0)
		{
			mover.moveLocalY(-moveDistance);
		}

		if(input.getKey(KeyCode::kE) > 0)
		{
			mover.moveLocalY(moveDistance);
		}

		if(input.getKey(KeyCode::kW) > 0)
		{
			mover.moveLocalZ(-moveDistance);
		}

		if(input.getKey(KeyCode::kS) > 0)
		{
			mover.moveLocalZ(moveDistance);
		}

		const Vec2 velocity = input.getMousePositionNdc() - mousePosOn1stClick;
		input.moveMouseNdc(mousePosOn1stClick);
		if(velocity != Vec2(0.0))
		{
			Euler angles(mover.getLocalRotation().getRotationPart());
			angles.x += velocity.y * toRad(360.0f) * F32(m_dt) * kMouseSensitivity;
			angles.x = clamp(angles.x, toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y += -velocity.x * toRad(360.0f) * F32(m_dt) * kMouseSensitivity;
			angles.z = 0.0f;
			mover.setLocalRotation(Mat3(angles));
		}
	}

	// Delete key deletes the selected node
	m_showDeleteSceneNodeDialog = m_showDeleteSceneNodeDialog || (input.getKey(KeyCode::kDelete) == 1 && m_selectedNode);
}

} // end namespace anki
