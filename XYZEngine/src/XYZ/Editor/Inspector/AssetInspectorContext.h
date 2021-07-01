#pragma once
#include "InspectorContext.h"

#include "XYZ/Asset/Asset.h"

namespace XYZ {
	namespace Editor {

		class AssetInspectorContext : public InspectorContext
		{
		public:		
			virtual void OnImGuiRender() override;

			void SetContext(const Ref<Asset>& context);


		private:
			static void drawMaterial(Ref<Material>& material);
			static void drawShader(Ref<Shader>& shader);
		
		private:
			Ref<Asset> m_Context;
		};
	}
}