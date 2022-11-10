#pragma once
#include"../../JEditorWindow.h"
#include"../../../../Utility/JVector.h"
#include"../../../../Core/SpaceSpatial/JSpaceSpatialType.h"

namespace JinEngine
{
	class JCamera;
	class JScene;
	class JMaterial;
	class JGameObject;
	namespace Core
	{
		struct JSpaceSpatialOption;
	}
	namespace Editor
	{
		class JEditorBinaryTreeView;
		class JEditorCameraControl;
		class JSceneObserver final : public JEditorWindow
		{  
		private:
			Core::JUserPtr<JScene> scene;
			Core::JUserPtr<JGameObject> cameraObj;
			Core::JUserPtr<JGameObject> mainCamFrustum;
		private:
			std::unique_ptr< JEditorBinaryTreeView> editorBTreeView;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
		private:
			int spaceSpatialIndex = 0; 
		private:
			bool isOpenSpatialOption = false;
			bool isMainCameraFrustumActivated = false;
			//Debug Shadow
			bool isShadowViewer = false;
			uint shadowIndex = 0;
			//Debug Spatial Space Tree
			bool isSpatialSpaceTreeViewer = false;
		private:
			std::wstring editorCameraName;
			JVector3<float> lastCamPos{ 0,0,0 };
			JVector3<float> lastCamRot{ 0,0,0 };
		private:
			//Debug
			static constexpr int debugMaterialCount = 8;
			JMaterial* debugMaterial[debugMaterialCount];
			JVector4<float> color[debugMaterialCount] = { {0.85f, 0.15f, 0.15f, 0.75f},
			{0.85f, 0.4f, 0.15f, 0.75f} ,
			{0.85f, 0.85f, 0.15f, 0.75f} ,
			{0.15f, 0.85f, 0.15f, 0.75f} ,
			{0.15f, 0.15f, 0.85f, 0.75f} ,
			{0.3f, 0.85f, 0.45f, 0.75f} ,
			{0.6f, 0.15f, 0.9f, 0.75f} ,
			{0.25f, 0.25f, 0.25f, 0.75f} };
			//uint streamCount = 0;
		public:
			JSceneObserver(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType);
			~JSceneObserver();
			JSceneObserver(const JSceneObserver& rhs) = delete;
			JSceneObserver& operator=(const JSceneObserver& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JScene> newScene, const std::wstring& editorCameraName)noexcept;
			void UpdateWindow()final;
		private: 			
			void SceneStructureOptionOnScreen();
			void OctreeOptionOnScreen();
			void BvhOptionOnScreen();
			void KdTreeOptionOnScreen();
			bool CommonOptionOnScreen(const std::string& uniqueName, Core::JSpaceSpatialOption& commonOption);
			void DebugTreeOnScreen(const Core::J_SPACE_SPATIAL_TYPE type, const std::string& uniqueLabel);
		private:
			void UpdateMainCamFrustum()noexcept;
			void MakeMainCamFrustum();
		private:
			void ShadowMapViewerOnScreen();
		public:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
			void StoreEditorWindow(std::wofstream& stream)final;
			void LoadEditorWindow(std::wifstream& stream)final;
		private:
			//Debug
			//void CreateShapeGroup(const J_DEFAULT_SHAPE& shape);
			//void CreateDebugMaterial()noexcept;
			//void DestroyDebugMaterial()noexcept;
		};
	}
}