#pragma once
#include"../../JEditorWindow.h"  
#include"../../../Interface/JEditorObjectInterface.h" 
#include"../../../../Core/Event/JEventListener.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../EditTool/JEditorViewStructure.h"
#include"../../../Popup/JEditorPopupMenu.h" 
namespace JinEngine
{
	class JAnimationController;
	namespace Core
	{ 
		class JAnimationFSMdiagram; 
		class JAnimationFSMstate;
		class JAnimationFSMtransition;
	}

	namespace Editor
	{
		class JEditorGraphView;
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JAnimationStateView final : public JEditorWindow,
			public JEditorObjectHandlerInterface
		{ 
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<Core::JAnimationFSMdiagram> selectedDiagram;
			Core::JUserPtr<Core::JAnimationFSMstate> selectedState;
			Core::JUserPtr<Core::JAnimationFSMtransition> selectedTransition;
		private:
			std::unique_ptr<JEditorGraphView> stateGraph;
			std::unique_ptr<JEditorPopupMenu> statePopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			using AniContUserPtr = Core::JUserPtr<JAnimationController>;
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
			using RegisterEvF = Core::JMFunctorType<JAnimationStateView, void>;
			using CreateStateF = Core::JTransitionCreationHandleType<DataHandleStructure&, AniContUserPtr, const size_t, const size_t>;
			using CreateTransitionF = Core::JTransitionCreationHandleType<DataHandleStructure&, AniContUserPtr, const size_t, const size_t, const size_t>;
			using DestroyTransitionF = Core::JTransitionCreationHandleType<DataHandleStructure&, AniContUserPtr, const size_t>;
			using UndoDestroyF = Core::JTransitionCreationHandleType<DataHandleStructure&, AniContUserPtr>;

			using TryConnectStateTransitionF = Core::JSFunctorType<void, JAnimationStateView*>;
			using ConnectStateTrasitionF = Core::JSFunctorType<void, JAnimationStateView*>;
		private:
			DataHandleStructure fsmdata;
			std::unique_ptr<RegisterEvF::Functor> regCreateStateEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyStateEvF; 
			std::unique_ptr<RegisterEvF::Functor> regDestroyTransitionEvF;
			std::unique_ptr<TryConnectStateTransitionF::Functor> tryConnectStateTransF;
			std::unique_ptr<ConnectStateTrasitionF::Functor> connectStateTransF;

			std::unique_ptr<CreateStateF::Functor> createStateF;
			std::unique_ptr<CreateStateF::Functor> destroyStateF;
			std::unique_ptr<CreateTransitionF::Functor> createTransitionF;
			std::unique_ptr<DestroyTransitionF::Functor> destroyTransitionF;
			std::unique_ptr<UndoDestroyF::Functor> undoDestroyF;
		public:
			JAnimationStateView(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationStateView();
			JAnimationStateView(const JAnimationStateView& rhs) = delete;
			JAnimationStateView& operator=(const JAnimationStateView& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JAnimationController> aniCont)noexcept;
			void UpdateWindow()final; 
		private:
			void BuildDiagramView();
		private:
			void SetSelecteObject(Core::JUserPtr<Core::JIdentifier> preSelected, Core::JUserPtr<Core::JIdentifier> newSelected);
			void ClearSelectedObject();
		private:
			void RegisterViewGraphGroup(JAnimationController* newAnicont);
			void RegisterCreateStateEv();
			void RegisterDestroyStateEv();
			void RegisterCreateTransitionEv(const size_t inGuid, const size_t outGuid);
			void RegisterDestroyTransitionEv();
		private:
			void CreateState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniContconst, const size_t diagramGuid, const size_t stateGuid);
			void DestroyState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid);
			void CreateTranstion(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniContconst, const size_t diagramGuid, const size_t stateGuid, const size_t outGuid);
			void DestroyTransition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid);
			void UndoDestroy(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont);
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) final;
		};
	}
}
