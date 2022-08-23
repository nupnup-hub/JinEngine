#pragma once
#include"../../JEditorWindow.h"   
#include"../../../../Core/Event/JEventListener.h" 
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
		class JEditorPopup;
		class JAnimationControllerEditor : public JEditorWindow, public Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEventStruct*>
		{
			struct DiagramViewFuncData;
		private:
			//Diagram
			using DiagramCreateFunctor = Core::JFunctor<void, const size_t, const std::string, const size_t>;
			using DiagramCreateBinder = Core::JBindHandle<DiagramCreateFunctor, const size_t, const std::string, const size_t>;
			using DiagramCreateTuple = std::tuple<size_t, DiagramCreateFunctor>;

			using DiagramDestroyFunctor = Core::JFunctor<void, const size_t, const size_t>;
			using DiagramDestroyBinder = Core::JBindHandle<DiagramDestroyFunctor, const size_t, const size_t>;
			using DiagramDestroyTuple = std::tuple<size_t, DiagramDestroyFunctor>;

			using ConditionCreateFunctor = Core::JFunctor<void, const size_t, const std::string, const size_t>;
			using ConditionCreateBinder = Core::JBindHandle<ConditionCreateFunctor, const size_t, const std::string, const size_t>;
			using ConditionCreateTuple = std::tuple<size_t, ConditionCreateFunctor>;

			using ConditionDestroyFunctor = Core::JFunctor<void, const size_t, const size_t>;
			using ConditionDestroyBinder = Core::JBindHandle<ConditionDestroyFunctor, const size_t, const size_t>;
			using ConditionDestroyTuple = std::tuple<size_t, ConditionDestroyFunctor>;

			using DiagramViewCreateFunctor = Core::JFunctor<void, JAnimationController&, DiagramViewFuncData&>;
			using DiagramViewCretaeBinder = Core::JBindHandle<DiagramViewCreateFunctor, JAnimationController&, DiagramViewFuncData&>;
			using DiagramViewCreateTuple = std::tuple<size_t, DiagramViewCreateFunctor>;
			using DiagramViewDestroyFunctor = Core::JFunctor<void, JAnimationController&, Core::JAnimationFSMstate&, DiagramViewFuncData&>;
			using DiagramViewDestroyBinder = Core::JBindHandle<DiagramViewDestroyFunctor, JAnimationController&, Core::JAnimationFSMstate&, DiagramViewFuncData&>;
			using DiagramViewDestroyTuple = std::tuple<size_t, DiagramViewDestroyFunctor>;
		private:
			//Condition
			using SetBooleanConditionFunctor = Core::JFunctor<void, Core::JFSMcondition&,  bool>;
			using SetBooleanConditionBinder = Core::JBindHandle<SetBooleanConditionFunctor, Core::JFSMcondition&, bool>;

			using SetIntConditionFunctor = Core::JFunctor<void, Core::JFSMcondition&, int>;
			using SetIntConditionBinder = Core::JBindHandle<SetBooleanConditionFunctor, Core::JFSMcondition&, int>;

			using SetFloatConditionFunctor = Core::JFunctor<void, Core::JFSMcondition&, float>;
			using SetFloatConditionBinder = Core::JBindHandle<SetBooleanConditionFunctor, Core::JFSMcondition&, float>;
		private:
			size_t controllerGuid = 0;
			size_t selectedDiagramGuid = 0;
			size_t selectedStateGuid = 0;
			size_t selectedConditionGuid = 0;
			bool isSelectedDiagram = false;
			bool isSelectedState = false;
			bool isSelectedCondition = false;
		private:
			static constexpr uint stateShapeWidth = 100;
			static constexpr uint stateShapeHeight = 150;
			static constexpr uint nameMaxLength = 50;
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
			const std::string defaultDiagramName = "NewAnimationDiagram";
			const std::string defaultStateName = "NewAnimatioNState";
			const std::string defaultConditionName = "NewCondition";
			std::string nameBuf;
		private:
			std::unique_ptr<JEditorPopup>diagramListPopup;
			std::unique_ptr<JEditorPopup>conditionListPopup;
			std::unique_ptr<JEditorPopup>diagramViewPopup;

			std::unique_ptr<DiagramCreateTuple> createDiagramTuple;
			std::unique_ptr<DiagramDestroyTuple> destroyDiagramTuple;

			std::unique_ptr<ConditionCreateTuple> createConditionTuple;
			std::unique_ptr<ConditionDestroyTuple> destroyConditionTuple;

			std::unique_ptr<DiagramViewCreateTuple> createDiagramViewTuple;
			std::unique_ptr<DiagramViewDestroyTuple> destroyDiagramViewTuple;

			std::unique_ptr<SetBooleanConditionFunctor> setBooleanFunctor;
			std::unique_ptr<SetIntConditionFunctor> setIntFunctor;
			std::unique_ptr<SetFloatConditionFunctor> setFloatFunctor;
		public:
			JAnimationControllerEditor(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JAnimationControllerEditor();
			JAnimationControllerEditor(const JAnimationControllerEditor& rhs) = delete;
			JAnimationControllerEditor& operator=(const JAnimationControllerEditor& rhs) = delete;

			void Initialize();
			void UpdateWindow()override;
		private:
			void BuildDiagramList();
			void BuildDiagramListPopup();
			void BuildConditionList();
			void BuildConditionListPopup();
			void BuildAnimationDiagramView(float cursorPosY);
		private:
			void CloseAllPopup()noexcept;
			void ClearSelectableBuff()noexcept;

			void CreateNewDiagram(const size_t controllerGuid, const std::string name, const size_t guid)noexcept;
			void CreateNewCondition(const size_t controllerGuid, const std::string name, const size_t guid)noexcept;
			void CreateNewState(const size_t controllerGuid, const std::string name, const size_t guid, const DiagramViewFuncData funcData)noexcept;

			void DestroyDiagram(const size_t controllerGuid, const size_t guid)noexcept;
			void DestroyCondition(const size_t controllerGuid, const size_t guid)noexcept;
			void DestroyState(const size_t controllerGuid, const size_t guid)noexcept;
		private:
			void ClearCash()noexcept;
		protected:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct) final;
		};
	}
}