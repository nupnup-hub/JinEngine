#pragma once
#include"../../JEditorWindow.h"
#include"../../../Menubar/JEditorMenuNodeUtilData.h"
#include"../../../Utility/JEditorGameObjectSurpportToolType.h" 
#include"../../../../Utility/JVector.h"
#include"../../../../Core/SpaceSpatial/JSpaceSpatialType.h"
 
//Debug
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
namespace JinEngine
{
	class JCamera;
	class JScene;
	//class JMaterial;
	class JGameObject;
	namespace Core
	{
		struct JSpaceSpatialOption;
	}
	namespace Editor
	{
		class JEditorBinaryTreeView;
		class JEditorSceneCoordGrid;
		class JEditorCameraControl;
		class JEditorTransformTool;
		class JEditorMenuBar;
		class JEditorMenuNode; 
		class JSceneObserver final : public JEditorWindow
		{    
		public:
			enum class J_OBSERVER_SETTING_TYPE
			{
				//OPTION is control option panel
				//VIEW is rendering render result
				//TOOL is help editor

				OPTION_SPACE_SPATIAL,	 
				VIEW_SETTING_SPACE_SPATIAL_TREE,
				VIEW_SHADOW_VIEWER,
				VIEW_OCCLUSION_VIEWER,
				TOOL_CAM_FRUSTUM,	//TOOL
				TOOL_EDIT_GOBJ_POS,	 
				TOOL_EDIT_GOBJ_ROT,
				TOOL_EDIT_GOBJ_SCALE,
				TOOL_MAKE_DEBUG_OBJECT,
				COUNT
			}; 
		private:
			using OpenMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using ActivateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using DeActivateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using UpdateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
		private:
			using MenuIconT = typename Core::JSFunctorType<void, JSceneObserver*>;
		private: 
			std::unique_ptr<JEditorMenuBar> menubar;
			JEditorMenuNodeUtilData nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::COUNT];
			std::unique_ptr<OpenMenuNodeT::Functor>openNodeFunctor;
			std::unique_ptr<ActivateMenuNodeT::Functor>activateNodeFunctor;
			std::unique_ptr<DeActivateMenuNodeT::Functor>deActivateNodeFunctor;
			std::unique_ptr<UpdateMenuNodeT::Functor>updateNodeFunctor;
			std::unique_ptr<MenuIconT::Functor> menuIconFunctor;
		private:
			Core::JUserPtr<JScene> scene;
			Core::JUserPtr<JGameObject> cameraObj;
			Core::JUserPtr<JCamera> cameraComp;
			Core::JUserPtr<JGameObject> mainCamFrustum; 
		private:
			std::unique_ptr<JEditorSceneCoordGrid> coordGrid;
		private:
			std::unique_ptr<JEditorBinaryTreeView> editorBTreeView;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
			std::unique_ptr<JEditorTransformTool> positionTool;
			std::unique_ptr<JEditorTransformTool> rotationTool;
			std::unique_ptr<JEditorTransformTool> scaleTool;
		private:
			static constexpr uint toolCount = 3;
		private:
			std::vector<JEditorTransformTool*> toolVec;
			J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE lastActivatedToolType= J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::NONE;
		private:
			std::wstring editorCameraName;
			JVector3<float> lastCamPos{ 0,0,0 };
			JVector3<float> lastCamRot{ 0,0,0 };
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
			void CreateMenuLeafNode(JEditorMenuNode* parent, J_OBSERVER_SETTING_TYPE type)noexcept;
			void OpenObserverSettingNode(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void ActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void DeActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void UpdateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
		private: 			
			void SceneSpaceSpatialOptionOnScreen();
			void OctreeOptionOnScreen();
			void BvhOptionOnScreen();
			void KdTreeOptionOnScreen();
			bool CommonOptionOnScreen(const std::string& uniqueName, Core::JSpaceSpatialOption& commonOption);
			void DebugTreeOnScreen();
		private:
			void ShadowMapViewerOnScreen();
			void OcclusionResultOnScreen();
		private:
			void UpdateMainCamFrustum()noexcept;
			void MakeMainCamFrustum()noexcept; 
		private:
			void ActivateToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type);
			void DeActivateToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type);
			J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE ConvertSettingToToolType(const J_OBSERVER_SETTING_TYPE type)const noexcept;
			J_OBSERVER_SETTING_TYPE ConvertToolToSettingType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)const noexcept;
		public:
			void DoSetOpen()noexcept final;
			void DoSetClose()noexcept final;
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
			void StoreEditorWindow(std::wofstream& stream)final;
			void LoadEditorWindow(std::wifstream& stream)final;
		private:
			//Debug
			void CreateShapeGroup(const J_DEFAULT_SHAPE& shape, const uint xDim, const uint yDim, const uint zDim);
			//void CreateDebugMaterial()noexcept;
			//void DestroyDebugMaterial()noexcept;
		};
	}
}