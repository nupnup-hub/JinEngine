#pragma once
#include"JSceneObserverSettingType.h"
#include"../../JEditorWindow.h"
#include"../../../Menubar/JEditorMenuNodeUtilData.h"
#include"../../../EditTool/JEditorGameObjectSurpportToolType.h" 
#include"../../../Interface/JEditorObjectHandleInterface.h"
#include"../../../../Core/Math/JVector.h"
#include"../../../../Object/Resource/Scene/Accelerator/JAcceleratorType.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"

namespace JinEngine
{
	class JCamera;
	class JScene;
	class JTexture;
	class JMaterial;
	class JGameObject;
	class JAcceleratorOption;
	namespace Editor
	{
		class JEditorBinaryTreeView;
		class JEditorSceneCoordGrid;
		class JEditorCameraControl;
		class JEditorTransformTool;
		class JEditorMenuBar;
		class JEditorMenuNode;
		class JEditorMouseIdenDragBox;
		class JSceneObserver final : public JEditorWindow, public JEditorObjectHandlerInterface
		{
		private:
			struct FrustumInfo
			{
			public:
				JUserPtr<JGameObject> root;
				JUserPtr<JGameObject> nearFrustum;
				JUserPtr<JGameObject> farFrustum;
			public:
				JUserPtr<JCamera> cam;
			public:
				FrustumInfo(const JUserPtr<JCamera>& cam);
			public:
				void Create(const JUserPtr<JGameObject>& parent);
				void Clear();
			public:
				void Update();
			public:
				bool IsValid()const noexcept;
			};
			struct EditorOption
			{
			public:
				bool allowDisplayDebug = true;
				bool allowFrustumCulling = false;
				bool allowOccCulling = false;
				bool allowReflectCullingResult = false;
			};
			struct EditorCamData
			{
			public:
				JUserPtr<JCamera> cam = nullptr;
			public:
				std::wstring name;
				JVector3<float> lastPos{ 0,0,0 };
				JVector3<float> lastRot{ 0,0,0 };
			};
			struct TestData
			{
			public:
				static constexpr int minObjCount = 0;
				static constexpr int maxObjCount = 100;
				static constexpr int matCount = 7;
			public:
				int xCount = 1;
				int yCount = 1;
				int zCount = 1;
			public:
				J_DEFAULT_SHAPE meshType = J_DEFAULT_SHAPE::CUBE;
			public:
				JVector3<float> offsetPos = JVector3<float>(0, 0, 0);
				JVector3<float> offsetRot = JVector3<float>(0, 0, 0);
				JVector3<float> offsetScale = JVector3<float>(1, 1, 1);
			public:
				JVector3<float> distance = JVector3<float>(1, 1, 1);
			public:
				std::vector<JUserPtr<JMaterial>> matVec; 
			public:
				std::vector<JUserPtr<JGameObject>> objParentVec;
			public:
				void Initialize();
				void Clear();
			};
		private:
			using SelectMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using ActivateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using DeActivateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using UpdateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
		private:
			using MenuSwitchIconOnF = typename Core::JSFunctorType<void, JSceneObserver*>;
			using MenuSwitchIconOffF = typename Core::JSFunctorType<void, JSceneObserver*>;
		private:
			static constexpr uint menuSwitchIconCount = 7;
		private:
			std::unique_ptr<JEditorMenuBar> menubar;
			JEditorMenuNodeUtilData nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::COUNT];
			std::unique_ptr<SelectMenuNodeT::Functor>selectNodeFunctor;
			std::unique_ptr<ActivateMenuNodeT::Functor>activateNodeFunctor;
			std::unique_ptr<DeActivateMenuNodeT::Functor>deActivateNodeFunctor;
			std::unique_ptr<UpdateMenuNodeT::Functor>updateNodeFunctor;
			std::unique_ptr<MenuSwitchIconOnF::Functor> switchIconOnFunctorVec[menuSwitchIconCount];
			std::unique_ptr<MenuSwitchIconOffF::Functor> switchIconOffFunctorVec[menuSwitchIconCount];
		private:
			JUserPtr<JScene> scene;
			//JUserPtr<JGameObject> selectedGobj;
			std::unordered_map<size_t, FrustumInfo> camFrustumMap;
		private:
			std::unique_ptr<JEditorSceneCoordGrid> coordGrid;
			std::unique_ptr<JEditorBinaryTreeView> editorBTreeView;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
			std::unique_ptr<JEditorMouseIdenDragBox> mouseBBox;
			std::unique_ptr<JEditorTransformTool> positionTool;
			std::unique_ptr<JEditorTransformTool> rotationTool;
			std::unique_ptr<JEditorTransformTool> scaleTool;
		private:
			std::vector<JUserPtr<JTexture>> menuIconTexture;	//cashing
			std::vector<JUserPtr<JTexture>> sceneIconTexture;	//cashing
			std::vector<JEditorTransformTool*> toolVec;
		private:
			EditorOption editOption;
			EditorCamData editCamData;
		private:
			TestData testData;
		private:
			bool isCreateHelperGameObj = false;
		public:
			JSceneObserver(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag,
				const std::vector< J_OBSERVER_SETTING_TYPE> useSettingType);
			~JSceneObserver();
			JSceneObserver(const JSceneObserver& rhs) = delete;
			JSceneObserver& operator=(const JSceneObserver& rhs) = delete;
		private:
			void BuildMenuBar(const std::vector< J_OBSERVER_SETTING_TYPE> useSettingType);
			void BuildMenuIcon(const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType);
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(JUserPtr<JScene> newScene, const std::wstring& editorCameraName)noexcept;
			void UpdateWindow()final;
		private:
			void UpdateMouseWheel()final;
		private:
			void DisplaySceneIcon(const JVector2F sceneImagePos, const bool canSelectIcon, _Out_ bool& hasSelected);
		private:
			void CreateMenuLeafNode(JEditorMenuNode* parent, J_OBSERVER_SETTING_TYPE type)noexcept;
			void SelectObserverSettingNode(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void ActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void DeActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void UpdateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
		private:
			void SceneAcceleratorOptionOnScreen();
			void EditorCameraOptionOnScreen();
			void EngineTestOptionOnScreen();	//For testing engine performance
			void OctreeOptionOnScreen();
			void BvhOptionOnScreen();
			void KdTreeOptionOnScreen();
			bool CommonOptionOnScreen(const std::string& uniqueName, JAcceleratorOption& commonOption);
			void DebugTreeOnScreen();
		private:
			void ShadowMapViewerOnScreen();
			void RenderResultOnScreen();
		private:
			void UpdateMainCamFrustum()noexcept;
			void CreateCamFrustum(JUserPtr<JCamera> cam)noexcept;
		private:
			void ActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type);
			void DeActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type);
			J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE ConvertSettingToToolType(const J_OBSERVER_SETTING_TYPE type)const noexcept;
			J_OBSERVER_SETTING_TYPE ConvertToolToSettingType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)const noexcept;
		private:
			void CreateHelperGameObject();
			void DestroyHelperGameObject();
		public:
			void DoSetOpen()noexcept final;
			void DoSetClose()noexcept final;
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
			void StoreEditorWindow(std::wofstream& stream)final;
			void LoadEditorWindow(std::wifstream& stream)final;
		private:
			//Debug
			void CreateShapeGroup();
			//void CreateDebugMaterial()noexcept;
			//void DestroyDebugMaterial()noexcept;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) final;
		};
	}
}