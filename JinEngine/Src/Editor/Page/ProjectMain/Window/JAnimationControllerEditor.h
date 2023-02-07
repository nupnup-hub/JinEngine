#pragma once
#include"../../JEditorWindow.h"   
#include"../../../Interface/JEditorObjectInterface.h"
#include"../../../../Object/Resource/JResourceUserInterface.h"
#include"../../../../Core/Event/JEventListener.h" 
#include"../../../../Core/FSM/AnimationFSM/JAnimationStateType.h"
#include"../../../../Core/FSM/JFSMconditionValueType.h"
#include"../../../../Core/FSM/JFSMInterface.h"
#include"../../../../Core/Undo/JTransition.h"
#include<unordered_map>
#include<vector>
#include<tuple>
#include<bitset>

namespace JinEngine
{
	class JAnimationController;
	class JObject;
	namespace Core
	{
		class JFSMcondition;
		class JAnimationFSMstate;
	}
	namespace Editor
	{
		class JEditorStringMap;
		class JEditorInputBuffHelper;
		class JEditorPopupMenu;
		class JEditorGraphView;
		class JAnimationControllerEditor final : public JEditorWindow,
			public JResourceUserInterface,
			public JEditorObjectHandlerInterface
		{
		private:
			static constexpr int invalidIndex = -1;
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			int selectedDiagramIndex = invalidIndex;
			int selectedConditionIndex = invalidIndex;
			int selectedStateIndex = invalidIndex;
		private:
			float preMousePosX = 0;
			float preMousePosY = 0;
		private:
			bool reqInitDockNode = false;
		private:
			std::unique_ptr<JEditorStringMap> editorString;
			std::unique_ptr<JEditorInputBuffHelper> inputBuff;
			std::unique_ptr<JEditorGraphView> stateGraph;
		private:
			std::unique_ptr<JEditorPopupMenu>diagramListPopup;
			std::unique_ptr<JEditorPopupMenu>conditionListPopup;
			std::unique_ptr<JEditorPopupMenu>diagramViewPopup;
		private:
			using AniContUserPtr = Core::JUserPtr<JAnimationController>;
		private:
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
			using RegisterEvF = Core::JMFunctorType<JAnimationControllerEditor, void>;
			using DiagramHandleFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, AniContUserPtr, const size_t>;
			using DiagramHandleBind = Core::JBindHandle<DiagramHandleFunctor, const Core::EmptyType&, const Core::EmptyType&, AniContUserPtr, const size_t>;

			using ConditionHandleFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, AniContUserPtr, size_t>;
			using ConditionHandleBind = Core::JBindHandle<ConditionHandleFunctor, const Core::EmptyType&, const Core::EmptyType&, AniContUserPtr, const size_t>;

			using StateHandleFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, AniContUserPtr, const size_t, const size_t>;
			using StateHandleBind = Core::JBindHandle<StateHandleFunctor, const Core::EmptyType&, const Core::EmptyType&, AniContUserPtr, const size_t, const size_t>;
		private:
			//Condition
			using SetConditionTypeFunctor = Core::JFunctor<void, const Core::J_FSMCONDITION_VALUE_TYPE, AniContUserPtr, size_t>;
			using SetConditionNameFunctor = Core::JFunctor<void, const std::string, AniContUserPtr, size_t>;
			using SetConditionBooleanValueFunctor = Core::JFunctor<void, const bool, AniContUserPtr, size_t>;
			using SetConditionIntValueFunctor = Core::JFunctor<void, const int, AniContUserPtr, size_t>;
			using SetConditionFloatValueFunctor = Core::JFunctor<void, const float, AniContUserPtr, size_t>;
			//State
			using TryConnectStateTransitionF = Core::JSFunctorType<void, JAnimationControllerEditor*>;
			using ConnectStateTrasitionF = Core::JSFunctorType<void, JAnimationControllerEditor*>;
		private:
			DataHandleStructure fsmdata;
			std::unique_ptr<RegisterEvF::Functor> regCreateDiagramEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyDiagramEvF;
			std::unique_ptr<RegisterEvF::Functor> regCreateConditionEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyConditionEvF;
			std::unique_ptr<RegisterEvF::Functor> regCreateStateEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyStateEvF;

			std::unique_ptr<DiagramHandleFunctor> createDiagramF;
			std::unique_ptr<DiagramHandleFunctor> destroyDiagramF;
			std::unique_ptr<ConditionHandleFunctor> createConditionF;
			std::unique_ptr<ConditionHandleFunctor> destroyConditionF;
			std::unique_ptr<StateHandleFunctor> createStateF;
			std::unique_ptr<StateHandleFunctor> destroyStateF;

			std::unique_ptr<SetConditionTypeFunctor> setConditionTypeF;
			std::unique_ptr<SetConditionNameFunctor> setConditionNameF;
			std::unique_ptr<SetConditionBooleanValueFunctor> setConditionBoolF;
			std::unique_ptr<SetConditionIntValueFunctor> setConditionIntF;
			std::unique_ptr<SetConditionFloatValueFunctor> setConditionFloatF;

			std::unique_ptr<TryConnectStateTransitionF::Functor> tryConnectStateTransF;
			std::unique_ptr<ConnectStateTrasitionF::Functor> connectStateTransF;
		public:
			JAnimationControllerEditor(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE ownerPageType, 
				const J_EDITOR_WINDOW_FLAG windowFlag,
				const bool hasMetadata);
			~JAnimationControllerEditor();
			JAnimationControllerEditor(const JAnimationControllerEditor& rhs) = delete;
			JAnimationControllerEditor& operator=(const JAnimationControllerEditor& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		private:
			void SetAnimationController(Core::JUserPtr<JObject> newAniCont);
		private:
			bool HasAnimationController()const noexcept;
		private:
			void RegisterDiagramFunc();
			void RegisterConditionFunc();
			void RegisterStateFunc();
			void RegisterViewGraphGroup(JAnimationController* aniCont);
		public:
			void UpdateWindow()final;
		private:
			void BuildDiagramList();
			void BuildConditionList();
			void BuildDiagramView();
			void BuildDockNode();
		private:
			void CloseAllPopup()noexcept; 
			void ClearSelectedIndex()noexcept;
		private:
			void RegisterCreateDiagramEv();
			void RegisterDestroyDiagramEv();
			void RegisterCreateConditionEv();
			void RegisterDestroyConditionEv();
			void RegisterCreateStateEv();
			void RegisterDestroyStateEv();
		private:
			void CreateDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void DestroyDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void CreateCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void DestroyCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void CreateState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniContconst, const size_t diagramGuid, const size_t stateGuid);
			void DestroyState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid);
		protected:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) final;
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}