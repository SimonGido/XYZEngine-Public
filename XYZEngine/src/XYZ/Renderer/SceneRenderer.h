#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/EditorComponents.h"

namespace XYZ {
	
	struct GridProperties
	{
		glm::mat4 Transform;
		glm::vec2 Scale;
		float LineWidth;
	};

	struct SceneRendererOptions
	{
		bool ShowGrid = true;
		bool ShowBoundingBoxes = false;
	};

	struct SceneRendererCamera
	{
		XYZ::Camera Camera;
		glm::mat4 ViewMatrix;
		glm::vec3 ViewPosition;
	};

	struct RenderQueue
	{
		struct SpriteDrawCommand
		{
			SpriteRenderer* Sprite;
			TransformComponent* Transform;
		};
		struct DrawCommand
		{
			const RendererCommand*	Command;
			TransformComponent*		Transform;
		};

		std::vector<SpriteDrawCommand>	 SpriteDrawList;		
		std::vector<DrawCommand>         DrawCommandList;
	};


	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetViewportSize(uint32_t width, uint32_t height);

		static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
		static void BeginScene(const Scene* scene, const glm::mat4 viewProjectionMatrix, const glm::vec3& viewPosition);
		static void EndScene();
		static void SubmitSprite(SpriteRenderer* sprite, TransformComponent* transform);
		

		static void SubmitRendererCommand(const RendererCommand* command, TransformComponent* transform);
		static void SubmitLight(PointLight2D* light, const glm::mat4& transform);
		static void SubmitLight(SpotLight2D* light, const glm::mat4& transform);

		static void SetGridProperties(const GridProperties& props);

		static void UpdateViewportSize();

		static Ref<RenderPass> GetFinalRenderPass();
		static uint32_t GetFinalColorBufferRendererID();
		static SceneRendererOptions& GetOptions();
	
	private:
		static void flush();
		static void flushLightQueue();
		static void flushDefaultQueue();
		static void sortQueue(RenderQueue& queue);

		static void geometryPass(RenderQueue& queue, const Ref<RenderPass>& pass, bool clear);
		static void lightPass();
		static void bloomPass();
		static void gaussianBlurPass();
	};
}