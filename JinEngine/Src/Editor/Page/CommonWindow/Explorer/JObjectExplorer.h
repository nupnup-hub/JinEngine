#pragma once
#include"../../JEditorWindow.h"  
#include"../../../Utility/JEditorRenameHelper.h"
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
		class JEditorString;
		class JEditorPopupMenu;
		class JEditorSearchBarHelper;
		class JObjectExplorer final : public JEditorWindow, public JEditorObjectHandlerInterface
		{ 
		private:
			Core::JUserPtr<JGameObject> root;
			Core::JUserPtr<JGameObject> selectedObject;  

			std::unique_ptr<JEditorString>editorString;
			std::unique_ptr<JEditorRenameHelper>renameHelper;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
			std::unique_ptr<JEditorPopupMenu>explorerPopup; 
		private:
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), JGameObject>;
			using CreateGameObjectFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JGameObject>, const size_t, const J_DEFAULT_SHAPE>;
			using CreateModelFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
			using DestroyGameObjectFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, const size_t>;
			using UndoDestroyGameObjectFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&>;

			using CreateGameObjectBind = Core::JBindHandle<CreateGameObjectFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JGameObject>, const size_t, const J_DEFAULT_SHAPE>;
			using CreateModelBind = Core::JBindHandle<CreateModelFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
			using DestroyGameObjectBind = Core::JBindHandle<DestroyGameObjectFunctor, const Core::EmptyType&, const Core::EmptyType&, const size_t>;
			using UndoDestroyGameObjectBind = Core::JBindHandle<UndoDestroyGameObjectFunctor, const Core::EmptyType&, const Core::EmptyType&>;

			using CreateGameObjectEvStruct = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateGameObjectBind, DestroyGameObjectBind>;
			using DestroyGameObjectEvStruct = JEditorTCreateBindFuncEvStruct<DataHandleStructure, DestroyGameObjectBind, UndoDestroyGameObjectBind>;
			using CreateModelEvStruct = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateModelBind, DestroyGameObjectBind>;;
			using ChangeParentF = Core::JSFunctorType<void, JObjectExplorer*, Core::JUserPtr<JGameObject>, Core::JUserPtr<JGameObject>>;
		
			using RegisterCreateGEvF = Core::JMFunctorType<JObjectExplorer, void, J_DEFAULT_SHAPE>;
			using RegisterDestroyGEvF = Core::JMFunctorType<JObjectExplorer, void>;
			using RenameF = Core::JSFunctorType<void, JObjectExplorer*>;
		private:
			DataHandleStructure dataStructure;
			std::unique_ptr<CreateGameObjectFunctor> createF;
			std::unique_ptr<CreateModelFunctor> createModelF;
			std::unique_ptr<DestroyGameObjectFunctor> destroyF;
			std::unique_ptr<UndoDestroyGameObjectFunctor> undoDestroyF; 
			std::unique_ptr<ChangeParentF::Functor> changeParentF;
			std::unique_ptr<RegisterCreateGEvF::Functor> regCreateGobjF;
			std::unique_ptr<RegisterDestroyGEvF::Functor> regDestroyGobjF;
			std::unique_ptr<RenameF::Functor> renameF;
		public:
			JObjectExplorer(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType);
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