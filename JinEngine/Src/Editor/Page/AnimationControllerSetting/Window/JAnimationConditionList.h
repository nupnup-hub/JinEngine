#pragma once
#include"../../JEditorWindow.h"  
#include"../../../Interface/JEditorObjectInterface.h" 
#include"../../../../Core/FSM/JFSMparameterValueType.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Core
	{
		class JAnimationFSMdiagram;
		class JFSMparameter;
	}
	namespace Editor
	{ 
		class JEditorPopupMenu;
		class JEditorStringMap;
		class JAnimationConditionList final : public JEditorWindow,
			public JEditorObjectHandlerInterface
		{
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<Core::JAnimationFSMdiagram> selectedDiagram;
			Core::JUserPtr<Core::JFSMparameter> selectedCondition;
		private: 
			std::unique_ptr<JEditorPopupMenu> conditionListPopup;
			std::unique_ptr<JEditorStringMap> editorString;
		private:
			using AniContUserPtr = Core::JUserPtr<JAnimationController>;
			using DataHandleStructure = Core::JDataHandleStructure<Core::JTransition::GetMaxTaskCapacity(), Core::JIdentifier>;
			using RegisterEvF = Core::JMFunctorType<JAnimationConditionList, void>;
			using ConditionCreationFunctor = Core::JFunctor<void, DataHandleStructure&, Core::JDataHandle&, AniContUserPtr, size_t>;
			using ConditionCreationBind = Core::JBindHandle<ConditionCreationFunctor, const Core::EmptyType&, const Core::EmptyType&, AniContUserPtr, const size_t>;
			
			using SetConditionTypeFunctor = Core::JFunctor<void, const Core::J_FSM_PARAMETER_VALUE_TYPE, AniContUserPtr, size_t>;
			using SetConditionNameFunctor = Core::JFunctor<void, const std::string, AniContUserPtr, size_t>;
			using SetConditionBooleanValueFunctor = Core::JFunctor<void, const bool, AniContUserPtr, size_t>;
			using SetConditionIntValueFunctor = Core::JFunctor<void, const int, AniContUserPtr, size_t>;
			using SetConditionFloatValueFunctor = Core::JFunctor<void, const float, AniContUserPtr, size_t>;
		private:
			DataHandleStructure fsmdata;
			std::unique_ptr<RegisterEvF::Functor> regCreateConditionEvF;
			std::unique_ptr<RegisterEvF::Functor> regDestroyConditionEvF;
			std::unique_ptr<ConditionCreationFunctor> createConditionF;
			std::unique_ptr<ConditionCreationFunctor> destroyConditionF;

			std::unique_ptr<SetConditionTypeFunctor> setConditionTypeF;
			std::unique_ptr<SetConditionNameFunctor> setConditionNameF;
			std::unique_ptr<SetConditionBooleanValueFunctor> setConditionBoolF;
			std::unique_ptr<SetConditionIntValueFunctor> setConditionIntF;
			std::unique_ptr<SetConditionFloatValueFunctor> setConditionFloatF;
		public:
			JAnimationConditionList(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAnimationConditionList() = default;
			JAnimationConditionList(const JAnimationConditionList& rhs) = delete;
			JAnimationConditionList& operator=(const JAnimationConditionList& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept;
			void UpdateWindow()final;
		private:
			void BuildConditionList();
		private:
			void RegisterCreateConditionEv();
			void RegisterDestroyConditionEv();
		private:
			void CreateCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
			void DestroyCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid);
		protected:
			void DoSetClose()noexcept final; 
		};
	}
}
