#pragma once
#include"../../JEditorWindow.h"   
#include"../../../Interface/JEditorObjectInterface.h"
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Core/Undo/JTransition.h"

namespace JinEngine
{
	class JGameObject;
	class JObject;
	class JMeshGeometry;
	namespace Editor
	{
		class JEditorStringMap;
		class JEditorRenameHelper;
		class JEditorPopupMenu;
		class JEditorSearchBarHelper;
		class JObjectExplorer final : public JEditorWindow, public JEditorObjectHandlerInterface
		{ 
		private:
			Core::JUserPtr<JGameObject> root;
			Core::JUserPtr<JGameObject> selectedObject;  

			std::unique_ptr<JEditorStringMap>editorString;
			std::unique_ptr<JEditorRenameHelper>renameHelper;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
			std::unique_ptr<JEditorPopupMenu>explorerPopup; 
		private:
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), JGameObject>;
			using CreateGameObjectF = Core::JTransitionCreationHandleType<DataHandleStructure&, Core::JUserPtr<JGameObject>, const size_t, const J_DEFAULT_SHAPE>;
			using CreateModelF = Core::JTransitionCreationHandleType<DataHandleStructure&, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
			using DestroyGameObjectF = Core::JTransitionCreationHandleType<DataHandleStructure&, const size_t>;
			using UndoDestroyGameObjectF = Core::JTransitionCreationHandleType<DataHandleStructure&>;

			using CreateGameObjectEvStruct = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateGameObjectF::Bind, DestroyGameObjectF::Bind>;
			using DestroyGameObjectEvStruct = JEditorTCreateBindFuncEvStruct<DataHandleStructure, DestroyGameObjectF::Bind, UndoDestroyGameObjectF::Bind>;
			using CreateModelEvStruct = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateModelF::Bind, DestroyGameObjectF::Bind>;;
			using ChangeParentF = Core::JSFunctorType<void, JObjectExplorer*, Core::JUserPtr<JGameObject>, Core::JUserPtr<JGameObject>>;
		
			using RegisterCreateGEvF = Core::JMFunctorType<JObjectExplorer, void, J_DEFAULT_SHAPE>;
			using RegisterDestroyGEvF = Core::JMFunctorType<JObjectExplorer, void>;
			using RenameF = Core::JSFunctorType<void, JObjectExplorer*>;
		private:
			DataHandleStructure dataStructure;
			std::unique_ptr<CreateGameObjectF::Functor> createF;
			std::unique_ptr<CreateModelF::Functor> createModelF;
			std::unique_ptr<DestroyGameObjectF::Functor> destroyF;
			std::unique_ptr<UndoDestroyGameObjectF::Functor> undoDestroyF;
			std::unique_ptr<ChangeParentF::Functor> changeParentF;
			std::unique_ptr<RegisterCreateGEvF::Functor> regCreateGobjF;
			std::unique_ptr<RegisterDestroyGEvF::Functor> regDestroyGobjF;
			std::unique_ptr<RenameF::Functor> renameF;
		public:
			JObjectExplorer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JObjectExplorer();
			JObjectExplorer(const JObjectExplorer& rhs) = delete;
			JObjectExplorer& operator=(const JObjectExplorer& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JGameObject> newRoot)noexcept;
			void UpdateWindow()final;
		private:
			void BuildObjectExplorer();
			void ObjectExplorerOnScreen(JGameObject* gObj, const bool isAcivatedSearch);  
		private:
			void RegisterCreateGameObjectEv(J_DEFAULT_SHAPE shapeType);
			void RegisterDestroyGameObjectEv();
			void CreateGameObject(DataHandleStructure& dS, 
				Core::JDataHandle& dH,
				Core::JUserPtr<JGameObject> p, 
				const size_t guid,
				const J_DEFAULT_SHAPE shapeType);
			void CreateModel(DataHandleStructure& dS,
				Core::JDataHandle& dH,
				Core::JUserPtr<JGameObject> p,
				Core::JUserPtr<JMeshGeometry> m,
				const size_t guid);
			void DestroyGameObject(DataHandleStructure& dS, Core::JDataHandle& dH, const size_t guid);
			void UndoDestroyGameObject(DataHandleStructure& dS, Core::JDataHandle& dH);
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
			void DoSetUnFocus()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}