#include "stdafx.h"
#include "Renderer.h"

#include "CustomRenderer2D.h"
#include "Renderer2D.h"
#include "SceneRenderer.h"

#include "XYZ/Core/Application.h"

#include <GL/glew.h>

namespace XYZ {
	
	
	struct RendererData
	{
		RendererData()
			: Pool(1)
		{}
		std::shared_ptr<ThreadPass<RenderCommandQueue>> CommandQueue;
		ThreadPool										Pool;
		Ref<RenderPass>									ActiveRenderPass;
		Ref<VertexArray>								FullscreenQuadVertexArray;
		Ref<VertexBuffer>								FullscreenQuadVertexBuffer;
		Ref<IndexBuffer>								FullscreenQuadIndexBuffer;

		std::future<bool>							    RenderThreadFinished;
	};

	static RendererData s_Data;

	static void SetupFullscreenQuad()
	{
		s_Data.FullscreenQuadVertexArray = VertexArray::Create();
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].TexCoord = glm::vec2(0, 1);

		BufferLayout layout = {
			{ 0, ShaderDataComponent::Float3, "a_Position" },
			{ 1, ShaderDataComponent::Float2, "a_TexCoord" }
		};
		s_Data.FullscreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		s_Data.FullscreenQuadVertexBuffer->SetLayout(layout);
		s_Data.FullscreenQuadVertexArray->AddVertexBuffer(s_Data.FullscreenQuadVertexBuffer);

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_Data.FullscreenQuadIndexBuffer = IndexBuffer::Create(indices, 6);
		s_Data.FullscreenQuadVertexArray->SetIndexBuffer(s_Data.FullscreenQuadIndexBuffer);
	}

	void Renderer::Init()
	{
		s_Data.CommandQueue = std::make_shared<ThreadPass<RenderCommandQueue>>();
		Renderer::Submit([=]() {
			RendererAPI::Init();
		});
		CustomRenderer2D::Init();
		Renderer2D::Init();
		SceneRenderer::Init();
		
		SetupFullscreenQuad();
	}

	void Renderer::Shutdown()
	{
		CustomRenderer2D::Shutdown();
		Renderer2D::Shutdown();
		SceneRenderer::Shutdown();

		s_Data.FullscreenQuadVertexArray.Reset();
		s_Data.FullscreenQuadVertexBuffer.Reset();
		s_Data.FullscreenQuadIndexBuffer.Reset();
	}

	void Renderer::Clear()
	{
		Renderer::Submit([=]() {
			RendererAPI::Clear();
		});
	}

	void Renderer::SetClearColor(const glm::vec4& color)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetClearColor(color);
		});
	}

	void Renderer::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetViewport(x, y, width, height);
		});
	}

	void Renderer::SetLineThickness(float thickness)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetLineThickness(thickness);
			});
	}

	void Renderer::SetPointSize(float size)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetPointSize(size);
			});
	}

	void Renderer::SetDepthTest(bool val)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetDepth(val);
			});
	}

	void Renderer::DrawArrays(PrimitiveType type, uint32_t count)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawArrays(type, count);
			});
	}

	void Renderer::DrawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t queueType)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawIndexed(type, indexCount);
		}, queueType);
	}

	void Renderer::DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count, uint32_t offset)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawInstanced(vertexArray, count, offset);
		});
	}

	void Renderer::DrawElementsIndirect(void* indirect)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawInstancedIndirect(indirect);
		});
	}

	void Renderer::SubmitFullsceenQuad()
	{
		s_Data.FullscreenQuadVertexArray->Bind();
		Renderer::DrawIndexed(PrimitiveType::Triangles, 6);
	}


	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear)
	{
		XYZ_ASSERT(renderPass.Raw(), "Render pass can not be null");
		s_Data.ActiveRenderPass = renderPass;
		if (!s_Data.ActiveRenderPass->GetSpecification().TargetFramebuffer->GetSpecification().SwapChainTarget)
			s_Data.ActiveRenderPass->GetSpecification().TargetFramebuffer->Bind();

		if (clear)
		{
			renderPass->GetSpecification().TargetFramebuffer->Clear();
		}
	}

	void Renderer::EndRenderPass()
	{
		XYZ_ASSERT(s_Data.ActiveRenderPass.Raw(), "No active render pass! Have you called Renderer::EndRenderPass twice?");
		s_Data.ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		s_Data.ActiveRenderPass = nullptr;
	}

	ThreadPool& Renderer::GetPool()
	{
		return s_Data.Pool;
	}

	void Renderer::WaitAndRender()
	{
		auto queue = s_Data.CommandQueue;
		queue->Swap();
		//s_Data.RenderThreadFinished = s_Data.Pool.PushJob<bool>([queue]() -> bool{
			{
				auto val = queue->Read();
				val.Get().Execute();
			}
			
		//	return true;
		//});
	}


	void Renderer::BlockRenderThread()
	{
		//s_Data.RenderThreadFinished.wait();
	}

	ScopedLockReference<RenderCommandQueue> Renderer::GetRenderCommandQueue(uint8_t type)
	{
		return s_Data.CommandQueue->Write();
	}
}