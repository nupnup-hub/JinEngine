#pragma once
#include"../../JEditorWindow.h"   
#include"../../../Utility/JEditorInputBuffHelper.h"
#include"../../../../Core/Event/JEventListener.h" 
#include"../../../../Core/FSM/AnimationFSM/JAnimationStateType.h"
#include"../../../../Core/FSM/JFSMconditionValueType.h"
#include "../../../../Core/FSM/JFSMInterface.h"
#include"../../../../Core/Undo/JTransition.h"
#include<unordered_map>
#include<vector>
#include<tuple>
#include<bitset>

namespace JinEngine
{
	class JAnimationController;
	namespace Core
	{
		class JFSMcondition;
		class JAnimationFSMstate;
	}
	namespace Editor
	{
		class JEditorString;
		class JEditorPopupMenu;
		class JAnimationControllerEditor final : public JEditorWindow
		{ 
		private:
			static constexpr int invalidIndex = -1;
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			int diagramIndex = invalidIndex;
			int conditionIndex = invalidIndex;
			int stateIndex = invalidIndex;
		private:
			static constexpr uint stateShapeWidth = 100;
			static constexpr uint stateShapeHeight = 150; 
			static constexpr uint selectableBufLength = 100;
			static constexpr uint frameThickness = 3;

			bool diagramListSelectable[selectableBufLength];
			bool conditionListSelectable[selectableBufLength];
			bool diagramViewSelectable[selectableBufLength];

			float preMousePosX = 0;
			float preMousePosY = 0;
		private:
			std::unique_ptr<JEditorString> editorString;
			const std::string diagramListName;
			const std::string conditionListName;
			const std::string diagramViewName;
			std::unique_ptr<JEditorInputBuffHelper> inputBuff;
		private:
			std::unique_ptr<JEditorPopupMenu>diagramListPopup;
			std::unique_ptr<JEditorPopupMenu>conditionListPopup;
			std::unique_ptr<JEditorPopupMenu>diagramViewPopup;
		private:
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
			using CreateDiagramFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JAnimationController>, const size_t>;
			using CreateDiagramBind = Core::JBindHandle<CreateDiagramFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JAnimationController>, const size_t>;

			using CreateConditionFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JAnimationController>, size_t>;
			using CreateConditionBind = Core::JBindHandle<CreateConditionFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JAnimationController>, const size_t>;
			
			using CreateStateFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, Core::JUserPtr<JAnimationController>, const size_t, const size_t>;
			using CreateStateBind = Core::JBindHandle<CreateStateFunctor, const Core::EmptyType&, const Core::EmptyType&, Core::JUserPtr<JAnimationController>, const size_t, const size_t>;
		private:
			//Condition
			using SetConditionTypeFunctor = Core::JFunctor<void, const Core::J_FSMCONDITION_VALUE_TYPE, Core::JUserPtr<JAnimationController>, size_t>;
			using SetConditionNameFunctor = Core::JFunctor<void, const std::string, Core::JUserPtr<JAnimationController>, size_t>;
			using SetConditionBooleanValueFunctor = Core::JFunctor<void, const bool, Core::JUserPtr<JAnimationController>, size_t>;
			using SetConditionIntValueFunctor = Core::JFunctor<void, const int, Core::JUserPtr<JAnimationController>, size_t>;
			using SetConditionFloatValueFunctor = Core::JFunctor<void, const float, Core::JUserPtr<JAnimationController>, size_t>;
		private:
			DataHandleStructure fsmdata;
			std::tuple<size_t, std::unique_ptr<CreateDiagramFunctor>> createDiagramT;
			std::tuple<size_t, std::unique_ptr<CreateDiagramFunctor>> destroyDiagramT;
			std::tuple<size_t, std::unique_ptr<CreateConditionFunctor>> createConditionT;
			std::tuple<size_t, std::unique_ptr<CreateConditionFunctor>> destroyConditionT;
			std::tuple<size_t, std::unique_ptr<CreateStateFunctor>> createStateT;
			std::tuple<size_t, std::unique_ptr<CreateStateFunctor>> destroyStateT;

			std::unique_ptr<SetConditionTypeFunctor> setConditionTypeF;
			std::unique_ptr<SetConditionNameFunctor> setConditionNameF;
			std::unique_ptr<SetConditionBooleanValueFunctor> setConditionBoolF;
			std::unique_ptr<SetConditionIntValueFunctor> setConditionIntF;
			std::unique_ptr<SetConditionFloatValueFunctor> setConditionFloatF;
		public:
			JAnimationControllerEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JAnimationControllerEditor();
			JAnimationControllerEditor(const JAnimationControllerEditor& rhs) = delete;
			JAnimationControllerEditor& operator=(const JAnimationControllerEditor& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		private:
			void RegisterDiagramFunc();
			void RegisterConditionFunc();
			void RegisterStateFunc();
		public: 
			void UpdateWindow()final;
		private:
			void BuildDiagramList();
			void BuildDiagramListPopup();
			void BuildConditionList();
			void BuildConditionListPopup();
			void BuildDiagramView(float cursorPosY);
			void BuildDiagramViewPopup();
		private:
			void CloseAllPopup()noexcept;
			void ClearSelectableBuff(bool* selectableBuf)noexcept;
		private:
			void ClearCash()noexcept;
		protected:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct) final;
		};
	}
}