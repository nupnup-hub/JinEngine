#pragma once
#include"JSceneObserverSettingType.h"
#include"../../JEditorWindow.h"
#include"../../../Menubar/JEditorMenuNodeUtilData.h"
#include"../../../EditTool/JEditorGameObjectSurpportToolType.h" 
#include"../../../../Core/Math/JVector.h"
#include"../../../../Object/JObjectModifyInterface.h"
#include"../../../../Object/Resource/Scene/Accelerator/JAcceleratorType.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../../../Object/Component/Light/JLightType.h"
#include"../../../../Graphic/JGraphicConstants.h"

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
		class JEditorGeometryTool;
		class JEditorMenuBar;
		class JEditorMenuNode;
		class JEditorMouseDragSceneBox;
		class JEditorIdentifierList;

		class JSceneObserver final : public JEditorWindow, public JObjectModifyInterface
		{
		private:
			struct EditorOption
			{
			public:
				bool allowDisplayDebugging = true;
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
				enum class OBJ_TYPE
				{
					SHAPE,
					LIGHT,
					COUNT
				};
			public:
				static constexpr int minObjCount = 0;
				static constexpr int maxObjCount = 100;
				static constexpr int matCount = 7;
			public:
				int xCount = 1;
				int yCount = 1;
				int zCount = 1;
			public:
				OBJ_TYPE objType = OBJ_TYPE::SHAPE;
				J_DEFAULT_SHAPE meshType = J_DEFAULT_SHAPE::CUBE;
				J_LIGHT_TYPE litType = J_LIGHT_TYPE::POINT;
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
			struct TextureDebug
			{
			public:
				std::vector<Graphic::ResourceHandle> gpuHandle;
				std::vector<Core::JDataHandle> dataHandle;
			public:
				float sizeFactor = 0.25f;
			public:
				bool HasValidHandle()const noexcept;
			};
		public:
			using BeginScenePlayF = Core::JSFunctorType<void>;
			using EndScenePlayF = Core::JSFunctorType<void>;
		private:
			using SelectMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using ActivateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using DeActivateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
			using UpdateMenuNodeT = typename Core::JMFunctorType<JSceneObserver, void, const J_OBSERVER_SETTING_TYPE>;
		private:
			using MenuSwitchIconOnF = typename Core::JSFunctorType<void, JSceneObserver*>;
			using MenuSwitchIconOffF = typename Core::JSFunctorType<void, JSceneObserver*>;
		private:
			static constexpr uint menuSwitchIconCount = 8;
		private:
			std::unique_ptr<BeginScenePlayF::Functor> beginScenePlayF;
			std::unique_ptr<EndScenePlayF::Functor> endScenePlayF;
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
		private:
			std::unique_ptr<JEditorSceneCoordGrid> coordGrid;
			std::unique_ptr<JEditorBinaryTreeView> editBTreeView;
			std::unique_ptr<JEditorCameraControl> editCamCtrl;
			std::unique_ptr<JEditorMouseDragSceneBox> mouseBBox;
			std::unique_ptr<JEditorIdentifierList> idenList;
			std::unique_ptr<JEditorTransformTool> positionTool;
			std::unique_ptr<JEditorTransformTool> rotationTool;
			std::unique_ptr<JEditorTransformTool> scaleTool;
			std::unique_ptr<JEditorGeometryTool> geoTool;
		private:
			std::vector<JUserPtr<JTexture>> menuIconTexture;	//cashing
			std::vector<JUserPtr<JTexture>> sceneIconTexture;	//cashing
			std::vector<JEditorTransformTool*> toolVec; 
		private:
			std::unique_ptr<TextureDebug> textureDebug;
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
				const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType,
				const std::vector<size_t>& listenWindowGuidVec = {});
			~JSceneObserver();
			JSceneObserver(const JSceneObserver& rhs) = delete;
			JSceneObserver& operator=(const JSceneObserver& rhs) = delete;
		private:
			void BuildMenuBar(const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType);
			void BuildMenuIcon(const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType);
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void SetScenePlayProccess(std::unique_ptr<BeginScenePlayF::Functor> newBeginScenePlayF, std::unique_ptr<EndScenePlayF::Functor> newEndScenePlayF);
		public:
			void Initialize(JUserPtr<JScene> newScene, const std::wstring& editorCameraName)noexcept;
			void UpdateWindow()final;
		private:
			void UpdateMouseWheel()final;
		private: 
			//camera, light icon
			void DisplaySceneIcon(const JVector2F sceneImagePos, const bool canSelectIcon, _Out_ bool& hasSelected);
		private: 
			//bind node event func per J_OBSERVER_SETTING_TYPE
			void CreateMenuLeafNode(JEditorMenuNode* parent, J_OBSERVER_SETTING_TYPE type)noexcept;
			void SelectObserverSettingNode(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void ActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void DeActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
			void UpdateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept;
		private:
			//J_OBSERVER_SETTING_TYPE update func
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
			void TextureDetailOnScreen();		 
		private:
			void UpdateMainCamFrustum()noexcept; 
		private:
			void ActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type);
			void DeActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type);
			J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE ConvertSettingToToolType(const J_OBSERVER_SETTING_TYPE type)const noexcept;
			J_OBSERVER_SETTING_TYPE ConvertToolToSettingType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)const noexcept;
		private:
			void CreateHelperGameObject();
			void DestroyHelperGameObject();
		private:
			void DoSetOpen()noexcept final;
			void DoSetClose()noexcept final;
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool)final;
			void StoreEditorWindow(JFileIOTool& tool)final;
		private:
			//Debug
			void CreateShapeGroup();
			void CreateLightGroup();
			//void CreateDebugMaterial()noexcept;
			//void DestroyDebugMaterial()noexcept;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) final;
		};
	}
}