// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/ImageViewerUi.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Window/Input.h>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

ImageViewerUi::ImageViewerUi()
{
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("ShaderBinaries/UiVisualizeImage.ankiprogbin", m_imageProgram));

	for(U32 i = 0; i < 2; i++)
	{
		ShaderProgramResourceVariantInitInfo variantInit(m_imageProgram);
		variantInit.addMutation("TEXTURE_TYPE", i);

		const ShaderProgramResourceVariant* variant;
		m_imageProgram->getOrCreateVariant(variantInit, variant);
		m_imageGrPrograms[i].reset(&variant->getProgram());
	}
}

void ImageViewerUi::drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	const Bool imageChanged = !!m_image && (m_imageUuid != m_image->getUuid());
	if(imageChanged)
	{
		m_crntMip = 0;
		m_zoom = 1.0f;
		m_depth = 0.0f;
		m_pointSampling = true;
		m_colorChannel = {true, true, true, true};
		m_maxColorValue = 1.0f;

		m_imageUuid = m_image->getUuid();
	}

	if(ImGui::GetFrameCount() > 1)
	{
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Image Viewer", &m_open, windowFlags))
	{
		if(ImGui::BeginChild("Toolbox", Vec2(0.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
							 ImGuiWindowFlags_None))
		{
			// Texture info
			{
				ImGui::TextUnformatted(ICON_MDI_INFORMATION_SLAB_BOX);
				if(m_image)
				{
					const Texture& tex = m_image->getTexture();
					ImGui::SetItemTooltip("%u x %u x %u\nMips %u\nFormat %s", tex.getWidth(), tex.getHeight(), tex.getDepth(), tex.getMipmapCount(),
										  getFormatInfo(tex.getFormat()).m_name);
				}
				ImGui::SameLine();
			}

			// Zoom
			{
				if(ImGui::Button(ICON_MDI_MINUS))
				{
					m_zoom -= 0.1f;
				}
				ImGui::SameLine();
				ImGui::DragFloat("##Zoom", &m_zoom, 0.01f, 0.1f, 20.0f, "Zoom %.3f");
				ImGui::SameLine();
				if(ImGui::Button(ICON_MDI_PLUS))
				{
					m_zoom += 0.1f;
				}
				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();
			}

			// Sampling
			{
				ImGui::Checkbox("Point Sampling", &m_pointSampling);
				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();
			}

			// Colors
			{
				ImGui::Checkbox("Red", &m_colorChannel[0]);
				ImGui::SameLine();
				ImGui::Checkbox("Green", &m_colorChannel[1]);
				ImGui::SameLine();
				ImGui::Checkbox("Blue", &m_colorChannel[2]);
				ImGui::SameLine();
				const U32 colorComponentCount = (!!m_image) ? getFormatInfo(m_image->getTexture().getFormat()).m_componentCount : 4;
				if(colorComponentCount == 4)
				{
					ImGui::Checkbox("Alpha", &m_colorChannel[3]);
					ImGui::SameLine();
				}
				ImGui::Spacing();
			}

			// Mips combo
			if(m_image)
			{
				const U32 mipCount = m_image->getTexture().getMipmapCount();
				UiStringList mipLabels;
				for(U32 mip = 0; mip < mipCount; ++mip)
				{
					mipLabels.pushBackSprintf("Mip %u (%u x %u)", mip, m_image->getTexture().getWidth() >> mip,
											  m_image->getTexture().getHeight() >> mip);
				}

				if(ImGui::BeginCombo("##Mipmap", (mipLabels.getBegin() + m_crntMip)->cstr(), ImGuiComboFlags_HeightLarge))
				{
					for(U32 mip = 0; mip < mipCount; ++mip)
					{
						const Bool isSelected = (m_crntMip == mip);
						if(ImGui::Selectable((mipLabels.getBegin() + mip)->cstr(), isSelected))
						{
							m_crntMip = mip;
						}

						if(isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::SameLine();
			}

			// Depth
			if(m_image && m_image->getTexture().getTextureType() == TextureType::k3D)
			{
				UiStringList labels;
				for(U32 d = 0; d < m_image->getTexture().getDepth(); ++d)
				{
					labels.pushBackSprintf("Depth %u", d);
				}

				if(ImGui::BeginCombo("##Depth", (labels.getBegin() + U32(m_depth))->cstr(), ImGuiComboFlags_HeightLarge))
				{
					for(U32 d = 0; d < m_image->getTexture().getDepth(); ++d)
					{
						const Bool isSelected = (m_depth == F32(d));
						if(ImGui::Selectable((labels.getBegin() + d)->cstr(), isSelected))
						{
							m_depth = F32(d);
						}

						if(isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::SameLine();
			}

			// Avg color
			{
				const Vec4 avgColor = (m_image) ? m_image->getAverageColor() : Vec4(0.0f);

				ImGui::Text("Average Color %.2f %.2f %.2f %.2f", avgColor.x, avgColor.y, avgColor.z, avgColor.w);
				ImGui::SameLine();

				ImGui::ColorButton("Average Color", avgColor);
				ImGui::SameLine();
			}

			// Max color slider
			ImGui::SliderFloat("##Max color", &m_maxColorValue, 0.0f, 5.0f, "Max Color = %.3f");
		}
		ImGui::EndChild();

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
		if(Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0)
		{
			windowFlags |= ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove;
		}

		if(ImGui::BeginChild("Image", Vec2(-1.0f, -1.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX, windowFlags))
		{
			if(m_image)
			{
				Texture& tex = m_image->getTexture();

				// Center image
				const Vec2 imageSize = Vec2(F32(tex.getWidth()), F32(tex.getHeight())) * m_zoom;

				class ExtraPushConstants
				{
				public:
					Vec4 m_colorScale;
					Vec4 m_depth;
				} pc;
				pc.m_colorScale.x = F32(m_colorChannel[0]) / m_maxColorValue;
				pc.m_colorScale.y = F32(m_colorChannel[1]) / m_maxColorValue;
				pc.m_colorScale.z = F32(m_colorChannel[2]) / m_maxColorValue;
				pc.m_colorScale.w = F32(m_colorChannel[3]);

				pc.m_depth = Vec4((m_depth + 0.5f) / F32(tex.getDepth()));

				ImTextureID texid;
				texid.m_texture = &tex;
				texid.m_textureSubresource = TextureSubresourceDesc::surface(m_crntMip, 0, 0, DepthStencilAspectBit::kNone);
				texid.m_customProgram = m_imageGrPrograms[tex.getTextureType() != TextureType::k2D].get();
				texid.m_extraFastConstantsSize = U8(sizeof(pc));
				texid.setExtraFastConstants(&pc, sizeof(pc));
				texid.m_pointSampling = m_pointSampling;
				ImGui::Image(texid, imageSize);

				if(ImGui::IsItemHovered())
				{
					if(Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0)
					{
						// Zoom
						const F32 zoomSpeed = 0.05f;
						if(Input::getSingleton().getMouseButton(MouseButton::kScrollDown) > 0)
						{
							m_zoom *= 1.0f - zoomSpeed;
						}
						else if(Input::getSingleton().getMouseButton(MouseButton::kScrollUp) > 0)
						{
							m_zoom *= 1.0f + zoomSpeed;
						}

						// Pan
						if(Input::getSingleton().getMouseButton(MouseButton::kLeft) > 0)
						{
							auto toWindow = [&](Vec2 in) {
								in = in * 0.5f + 0.5f;
								in.y = 1.0f - in.y;
								in *= ImGui::GetMainViewport()->WorkSize;
								return in;
							};

							const Vec2 delta =
								toWindow(Input::getSingleton().getMousePositionNdc()) - toWindow(Input::getSingleton().getMousePreviousPositionNdc());

							if(delta.x != 0.0f)
							{
								ImGui::SetScrollX(ImGui::GetScrollX() - delta.x);
							}

							if(delta.y != 0.0f)
							{
								ImGui::SetScrollY(ImGui::GetScrollY() - delta.y);
							}
						}
					}
				}
			}
			else
			{
				ImGui::TextUnformatted("No image");
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	if(!m_open)
	{
		// It was closed
		m_image.reset(nullptr);
	}
}

} // end namespace anki
