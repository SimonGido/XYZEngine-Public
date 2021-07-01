#pragma once

#include <glm/glm.hpp>

namespace XYZ {

	struct ParticleVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	struct ParticleData
	{
		glm::vec4 Color			 = glm::vec4(0.0f);
		glm::vec4 TexCoord		 = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		glm::vec2 Position		 = glm::vec2(0.0f);
		glm::vec2 Size			 = glm::vec2(0.0f);
		float	  Rotation		 = 0.0f;

	private:
		float	  Alignment[3];
	};

	struct ParticleSpecification
	{
		glm::vec4 StartColor;	
		glm::vec2 StartSize;
		glm::vec2 StartVelocity;
		glm::vec2 StartPosition;

	private:
		float	  TimeAlive = 0.0f;
		int32_t   IsAlive = 1;
	};


	enum class ParticleEmissionShape
	{
		Sphere,
		Rectangle,
		Cone
	};
}