#pragma once

#include <glm/glm.hpp>
#include <algorithm>

namespace XYZ {

	struct AABB
	{
		glm::vec3 Min, Max;

		AABB();
		AABB(const glm::vec3& min, const glm::vec3& max);
			
		float GetPerimeter() const;
		float CalculateArea() const;
		bool Contains(const AABB& aabb) const;
		bool Intersect(const AABB& aabb) const;

		static AABB Union(const AABB& a, const AABB& b);

		AABB operator +(const glm::vec2& val) const;
	};
}