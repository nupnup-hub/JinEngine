#pragma once
#include"../Core/JDataType.h"
#include<vector>
#include<memory>

namespace JinEngine
{
	class JScene;
	class ISceneFrameDirty;
	namespace Graphic
	{
		enum class J_GRAPHIC_DRAW_FREQUENCY
		{
			ALWAYS,
			UPDATED
		};
		struct JGraphicDrawTargetListener
		{
		public:
			const J_GRAPHIC_DRAW_FREQUENCY drawFrequency;
			bool drawRequest = false;
		public:
			JGraphicDrawTargetListener(const J_GRAPHIC_DRAW_FREQUENCY drawFrequency);
			~JGraphicDrawTargetListener();
		};
		struct JGraphicDrawTarget
		{
		public:
			JScene* scene;
			ISceneFrameDirty* sceneFrameDirty;
			uint transformCount;
			uint renderItemCount;
			uint animatorCount;
			uint cameraCount;
			uint lightCount;
			uint shadowCount;
			const bool isMainScene;
			std::vector<std::unique_ptr<JGraphicDrawTargetListener>>listenerInfo;
		public:
			JGraphicDrawTarget(JScene* scene, const J_GRAPHIC_DRAW_FREQUENCY drawFrequency, const bool isMainScene);
			~JGraphicDrawTarget();
		};
		class JGraphicDrawList
		{
			friend class JGraphicImpl;
		private:
			static std::vector<std::unique_ptr<JGraphicDrawTarget>> drawList;
		public:
			static bool AddDrawList(JScene* scene, const J_GRAPHIC_DRAW_FREQUENCY drawFrequency, const bool isMainScene)noexcept;
			static bool PopDrawList(JScene* scene)noexcept;
			static bool HasDrawList(JScene* scene)noexcept;
			static void UpdateScene(JScene* scene)noexcept;
		private:
			static bool AddDrawListListener(JScene* scene, const J_GRAPHIC_DRAW_FREQUENCY drawFrequency)noexcept;
			static int GetIndex(JScene* scene)noexcept;
			static void UpdateDrawList(int index)noexcept;
		};
	}
}