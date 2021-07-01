#include "stdafx.h"
#include "SpriteEditor.h"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/EditorRenderer.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Editor/EditorHelper.h"

#include <imgui.h>

namespace XYZ {
	namespace Editor {

		static glm::vec4 CalcBorders(const Ref<SubTexture>& subTexture)
		{
			glm::vec4 border = subTexture->GetTexCoords();
			auto& texture = subTexture->GetTexture();
			border.x *= texture->GetWidth();
			border.y *= texture->GetHeight();
			border.z *= texture->GetWidth();
			border.w *= texture->GetHeight();
			return border;
		}

		static void SetBorder(Ref<SubTexture>& subTexture, glm::vec4 border)
		{
			auto& texture = subTexture->GetTexture();
		
			border.x /= texture->GetWidth();
			border.y /= texture->GetHeight();
			border.z /= texture->GetWidth();
			border.w /= texture->GetHeight();

			subTexture->SetTexCoords({ border.x, border.y, border.z, border.w });
		}

		SpriteEditor::SpriteEditor()
			:
			m_Camera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_PixelBuffer(nullptr),
			m_ViewportFocused(false),
			m_ViewportHovered(false),
			m_ToolSectionWidth(300.0f),
			m_ViewSectionWidth(300.0f)
		{
			m_SpriteRenderer.Material = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
			FramebufferSpecs specs;
			specs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecs(FramebufferTextureFormat::RGBA16F),
				FramebufferTextureSpecs(FramebufferTextureFormat::DEPTH24STENCIL8)
			};
			m_RenderPass = RenderPass::Create({ Framebuffer::Create(specs) });
			m_Camera.LockOrtho(true);
		}
		void SpriteEditor::OnUpdate(Timestep ts)
		{
			if (m_Context.Raw())
			{
				if (m_ViewportHovered && m_ViewportFocused)
				{
					m_Camera.OnUpdate(ts);			
				}
				glm::vec4 border = m_Output->GetTexCoords();
				glm::vec2 textureSize = { (float)m_Context->GetWidth(), (float)m_Context->GetHeight() };
				glm::vec3 min = glm::vec3(glm::vec2(border.x, border.y) - 0.5f, 0.0f);
				glm::vec3 max = glm::vec3(glm::vec2(border.z, border.w) - 0.5f, 0.0f);
				EditorRenderer::SubmitEditorAABB(min, max, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				
				EditorRenderer::SubmitEditorSprite(&m_SpriteRenderer, &m_Transform);
				EditorRenderer::BeginPass(m_RenderPass, m_Camera.GetViewProjection(), m_Camera.GetPosition());
				EditorRenderer::EndPass(true);
			}
		}
		void SpriteEditor::OnImGuiRender(bool& open)
		{
			if (open)
			{			
				if (ImGui::Begin("Sprite Editor", &open))
				{
					ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					m_ViewSectionWidth = viewportPanelSize.x - m_ToolSectionWidth - 5.0f;

					EditorHelper::DrawSplitter(false, 5.0f, &m_ToolSectionWidth, &m_ViewSectionWidth, 50.0f, 50.0f);
					
					tools();
					ImGui::SameLine();
					viewer();
				}
				ImGui::End();
			}
		}
		void SpriteEditor::OnEvent(Event& event)
		{		
			if (m_ViewportHovered && m_ViewportFocused)
				m_Camera.OnEvent(event);
		}
		void SpriteEditor::SetContext(const Ref<Texture2D>& context)
		{
			if (m_PixelBuffer)
			{
				delete[]m_PixelBuffer;
				m_PixelBuffer = nullptr;
			}
			m_Context = context;
			m_Context->GetData(&m_PixelBuffer);
			m_SpriteRenderer.SubTexture = Ref<SubTexture>::Create(m_Context);
			m_Output = Ref<SubTexture>::Create(m_Context);
		}
		void SpriteEditor::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
				m_RenderPass->GetSpecification().TargetFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			}
		}
		void SpriteEditor::viewer()
		{
			if (ImGui::BeginChild("##Viewer", ImVec2(m_ViewSectionWidth, 0)))
			{
				ImVec2 viewerPanelSize = ImGui::GetContentRegionAvail();
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
				bool blocked = imguiLayer->GetBlockedEvents();
				if (blocked)
					imguiLayer->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);


				if (m_Context.Raw() && !ImGui::IsWindowCollapsed())
				{
					handlePanelResize({ viewerPanelSize.x, viewerPanelSize.y });
					uint32_t renderID = m_RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID(0);
					ImGui::Image(reinterpret_cast<void*>((void*)(uint64_t)renderID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
				}
			}
			ImGui::EndChild();
		}
		void SpriteEditor::tools()
		{
			if (ImGui::BeginChild("##Sprite", ImVec2(m_ToolSectionWidth, 0)))
			{
				EditorHelper::DrawNodeControl("Sprite", m_Output, [](auto& value) {

					glm::vec4 border = CalcBorders(value);
					const char* names[4] = {
						 "L", "B", "R", "T"
					};
					EditorHelper::DrawVec4Control("Border", names, border, 0.0f, 65.0f);
					SetBorder(value, border);
				});
			}
			ImGui::EndChild();
		}
	}
}


