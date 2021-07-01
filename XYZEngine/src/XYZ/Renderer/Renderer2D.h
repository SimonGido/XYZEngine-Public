#pragma once

#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "XYZ/Renderer/Font.h"

namespace XYZ {
	// TODO: Generic way to specify layout of renderer?

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();


		static void BeginScene(const glm::mat4& viewProjectionMatrix, const glm::vec3& viewPos);

		static uint32_t SetTexture(const Ref<Texture>& texture);
		static void SetMaterial(const Ref<Material>& material);
		
		static void SubmitCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color = glm::vec4(1.0f));
		static void SubmitPoint(const glm::vec3& pos, float radius, const glm::vec4& color = glm::vec4(1.0f));

		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor = 1.0f);
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		static void SubmitQuad(const glm::vec3& position, const glm::vec2 & size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		
		static void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));
		static void SubmitLineQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color = glm::vec4(1));

	
		static void SubmitCollisionQuad(const glm::mat4& transform, uint32_t id);
	
		static void SubmitQuadNotCentered(const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		static void SubmitGrid(const glm::mat4& transform, const glm::vec2& scale, float lineWidth);

		static void Flush();
		static void FlushLines();
		static void FlushCollisions();
		static void FlushPoints();

		/**
		* Clean up after rendering
		*/
		static void EndScene();
		
	};
}