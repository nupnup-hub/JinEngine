#pragma once
#include"../../JEditorWindow.h"  
#include"../../../../Object/JObjectType.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../../../Utility/JDelegate.h" 
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
		class JEditorPopup;
		class JObjectExplorer final : public JEditorWindow
		{ 
		private:
			Core::JUserPtr<JGameObject> root;
			Core::JUserPtr<JGameObject> renameTar;
			Core::JUserPtr<JGameObject> selectedObject;
			std::unique_ptr<JEditorString>editorString;
			std::unique_ptr<JEditorPopup>explorerPopup;
			std::string nameBuf;
		private:
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), JGameObject>;
			using CreateGameObjectFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JGameObject>, const size_t>; 
			using CreateGameObjectBind = Core::JBindHandle<CreateGameObjectFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JGameObject>, const size_t>;		 
		
			using DestroyGameObjectFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, const size_t>;
			using DestroyGameObjectBind = Core::JBindHandle<DestroyGameObjectFunctor, const Core::EmptyType&, const Core::EmptyType&, const size_t>;
			using UndoDestroyGameObjectFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&>;
			using UndoDestroyGameObjectBind = Core::JBindHandle<UndoDestroyGameObjectFunctor, const Core::EmptyType&, const Core::EmptyType&>;

			using CreateGameObjectEvStruct = JEditorCreateBindFuncEvStruct<DataHandleStructure, CreateGameObjectBind, DestroyGameObjectBind>;
			using DestroyGameObjectEvStruct = JEditorCreateBindFuncEvStruct<DataHandleStructure, DestroyGameObjectBind, UndoDestroyGameObjectBind>;

			using CreateModelFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
			using CreateModelBind = Core::JBindHandle<CreateModelFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JGameObject>, Core::JUserPtr<JMeshGeometry>, const size_t>;
 
			using CreateModelEvStruct = JEditorCreateBindFuncEvStruct<DataHandleStructure, CreateModelBind, DestroyGameObjectBind>;

			using RenameFuncF = Core::JSFunctorType<void, const std::string, Core::JUserPtr<JGameObject>>;
			using ChangeParentF = Core::JSFunctorType<void, Core::JUserPtr<JGameObject>, Core::JUserPtr<JGameObject>>;
		private:
			DataHandleStructure dataStructure;
			std::unordered_map<size_t, std::unique_ptr<CreateGameObjectFunctor>> createFuncMap;
			std::tuple<size_t, std::unique_ptr<DestroyGameObjectFunctor>> destroyT;
			std::tuple<size_t, std::unique_ptr<RenameFuncF::Functor>> renameT;
			std::unique_ptr<UndoDestroyGameObjectFunctor> undoDestroyF;
			std::unique_ptr<CreateModelFunctor> createModelF;
			std::unique_ptr<ChangeParentF::Functor> changeParentF;
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
			void ObjectExplorerOnScreen(JGameObject* gObj);
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
			void DoSetUnFocus()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}