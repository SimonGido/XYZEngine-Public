#pragma once

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Scene.h"

extern "C" {

	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoDomain MonoDomain;
}



namespace XYZ {
	
	class ScriptEngine
	{
	public:
		static void Init(const std::string& assemblyPath);
		static void Shutdown();

		static void LoadXYZRuntimeAssembly(const std::string& path);
		static void ReloadAssembly(const std::string& path);

		static void SetSceneContext(const Ref<Scene>& scene);
		static const Ref<Scene>& GetCurrentSceneContext();

		static void OnCreateEntity(SceneEntity entity);
		static void OnDestroyEntity(SceneEntity entity);
		static void OnUpdateEntity(SceneEntity entity, Timestep ts);

		static bool ModuleExists(const std::string& moduleName);
		static void InitScriptEntity(SceneEntity entity);
		static void InstantiateEntityClass(SceneEntity entity);
		static MonoDomain* GetMonoDomain();
	};

}