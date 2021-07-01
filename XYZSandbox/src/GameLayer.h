#pragma once

#include <XYZ.h>


namespace XYZ {

	class GameLayer : public XYZ::Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		bool onWindowResize(WindowResizeEvent& event);

	private:
		Ref<Scene> m_Scene;

		Editor::EditorCamera m_EditorCamera;

		SceneEntity m_Entity;

		bool m_CheckboxVal = false;
		float m_Value = 0.1f;
		float m_Test = 2.8f;
		float m_Haha = 3.4f;

		bool m_BranchOpen = true;
		bool m_ChildrenOpen = true;
		bool m_AnotherChildrenOpen = false;
	};

}